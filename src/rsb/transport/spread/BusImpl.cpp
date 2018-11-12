/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
 *
 * This file may be licensed under the terms of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by:
 *   CoR-Lab, Research Institute for Cognition and Robotics
 *     Bielefeld University
 *
 * ============================================================ */


#include "BusImpl.h"

#include <boost/format.hpp>

#include <rsc/misc/IllegalStateException.h>

#include <rsb/protocol/ProtocolException.h>

#include "GroupNameCache.h"

namespace rsb {
namespace transport {
namespace spread {

// WeakHandlerAdapter
//
// This prevents the ReceiverTask from keeping the Bus alive, breaking
// the shared_ptr cycle.

namespace {

class WeakHandlerAdapter : public ReceiverTask::Handler {
public:
    boost::weak_ptr<Bus> bus;

    WeakHandlerAdapter(BusPtr bus) :
        bus(bus) {
    }

    void handleIncomingNotification(IncomingNotificationPtr notification) {
        BusPtr bus = this->bus.lock();
        if (bus) {
            bus->handleIncomingNotification(notification);
        }
    }

    void handleError(const std::exception& error) {
        BusPtr bus = this->bus.lock();
        if (bus) {
            bus->handleError(error);
        }
    }
};

typedef boost::shared_ptr<WeakHandlerAdapter> WeakHandlerAdapterPtr;

}

/// BusImpl

BusPtr BusImpl::create(SpreadConnectionPtr connection) {
    return BusPtr(new BusImpl(connection));
}

BusImpl::BusImpl(SpreadConnectionPtr connection) :
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.Bus")),
    active(false),
    connection(connection), memberships(connection),
    executor(new rsc::threading::ThreadedTaskExecutor()) {
}

BusImpl::~BusImpl() {
    RSCINFO(this->logger, (boost::format("%1% is destructing") % *this));
    if (this->active) {
        deactivate();
    }
}

void BusImpl::printContents(std::ostream& stream) const {
    stream << "connection = ";
    this->connection->printContents(stream);
    stream << ", state = " << (this->active ? "" : "not ") << "active"
           << ", sinks = " << this->scopeDispatcher.size();
}

const std::string BusImpl::getTransportURL() const {
    return this->connection->getTransportURL();
}

void BusImpl::activate() {
    if (this->active) {
        throw rsc::misc::IllegalStateException("Bus is already active");
    }

    this->connection->activate();

    WeakHandlerAdapterPtr handler(new WeakHandlerAdapter(shared_from_this()));
    this->receiver.reset(new ReceiverTask(this->connection, handler));
    this->executor->schedule(this->receiver);

    this->active = true;
}

void BusImpl::deactivate() {
    if (!this->active) {
        throw rsc::misc::IllegalStateException("Bus is not active");
    }

    this->receiver->cancel();
    this->connection->interruptReceive();
    this->receiver->waitDone();

    this->connection->deactivate();

    this->active = false;
}

void BusImpl::addSink(const Scope& scope, SinkPtr sink) {
    RSCDEBUG(this->logger,
             (boost::format("Bus %1% is adding scope = %2%, sink = %3%")
              % *this % scope % sink))

    {
        boost::mutex::scoped_lock lock(this->sinkMutex);

        this->memberships.join(GroupNameCache::scopeToGroup(scope));

        this->scopeDispatcher.addSink(scope, sink);
    }
}

void BusImpl::removeSink(const Scope& scope, const Sink* sink) {
    RSCDEBUG(this->logger,
             (boost::format("Bus %1% is removing scope = %2%, sink = %3%")
              % *this % scope % sink))

    {
        boost::mutex::scoped_lock lock(this->sinkMutex);

        this->scopeDispatcher.removeSink(scope, sink);

        this->memberships.leave(GroupNameCache::scopeToGroup(scope));
    }
}

namespace {

struct PoorPersonsLambda1 {
    OutgoingNotificationPtr notification;

    PoorPersonsLambda1(OutgoingNotificationPtr notification) :
        notification(notification) {}

    void operator()(BusImpl::Sink& sink) {
        sink.handleNotification(this->notification);
    }
};

}

void BusImpl::handleOutgoingNotification(OutgoingNotificationPtr notification) {
    RSCTRACE(this->logger, (boost::format("Bus %1% is handling outgoing "
                                          "notification %2%, scope = %3%")
                            % *this % notification % notification->scope));

    sendNotification(notification);

    {
        boost::mutex::scoped_lock lock(this->sinkMutex);

        this->scopeDispatcher.mapSinks(notification->scope,
                                       PoorPersonsLambda1(notification));
    }
}

namespace {

struct PoorPersonsLambda2 {
    IncomingNotificationPtr notification;

    PoorPersonsLambda2(IncomingNotificationPtr notification) :
        notification(notification) {}

    void operator()(BusImpl::Sink& sink) {
        sink.handleNotification(this->notification);
    }
};

}

void BusImpl::handleIncomingNotification(IncomingNotificationPtr notification) {
    RSCTRACE(this->logger, (boost::format("Bus %1% is handling incoming "
                                          "notification %2%, scope = %3%")
                            % *this % notification % notification->scope));

    {
        boost::mutex::scoped_lock lock(this->sinkMutex);

        this->scopeDispatcher.mapSinks(notification->scope,
                                       PoorPersonsLambda2(notification));
    }
}

namespace {

struct PoorPersonsLambda3 {
    const std::exception& exception;

    PoorPersonsLambda3(const std::exception& exception) :
        exception(exception) {}

    void operator()(BusImpl::Sink& sink) {
        sink.handleError(this->exception);
    }
};

}

void BusImpl::handleError(const std::exception& error) {
    boost::mutex::scoped_lock lock(this->sinkMutex);

    this->scopeDispatcher.mapAllSinks(PoorPersonsLambda3(error));
}

///

void BusImpl::sendNotification(OutgoingNotificationPtr notification) {
    SpreadMessage message;

    // Quality of service.
    message.setQOS(notification->qos);

    // Add groups.
    for (std::vector<std::string>::const_iterator it
             = notification->groups.begin();
         it != notification->groups.end(); ++it) {
        message.addGroup(*it);
    }

    // Send fragments
    for (std::vector<rsb::protocol::FragmentedNotification>::iterator it
             = notification->fragments.begin();
         it != notification->fragments.end(); ++it) {
        it->set_num_data_parts(notification->fragments.size());

        if (!it->SerializeToString(&message.mutableData())) {
            throw rsb::protocol::ProtocolException("Failed to write notification to stream");
        }

        this->connection->send(message);
        // TODO implement queuing or throw messages away?
        // TODO maybe return exception with msg that was not sent
        // TODO especially important to fulfill QoS specs
    }
}

}
}
}
