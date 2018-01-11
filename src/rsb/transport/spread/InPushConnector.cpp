/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2012-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "InPushConnector.h"

#include <rsc/threading/ThreadedTaskExecutor.h>

#include <rsb/MetaData.h>

#include "GroupNameCache.h"
#include "ReceiverTask.h"

using namespace std;

using namespace rsc::logging;
using namespace rsc::runtime;
using namespace rsc::threading;

using namespace rsb::converter;

namespace rsb {
namespace transport {
namespace spread {

class InPushConnector::Handler : public ReceiverTask::Handler {
public:
    Handler(InPushConnector* connector) :
        connector(connector) {
    }

    void handleIncomingNotification(rsb::protocol::NotificationPtr notification) {
        this->connector->handleIncomingNotification(notification);
    }

    InPushConnector* connector;
};

InPushConnector::InPushConnector(const ConverterSelectionStrategyPtr converters,
                                 SpreadConnectionPtr                 connection) :
    transport::ConverterSelectingConnector<string>(converters),
    ConnectorBase(connection),
    logger(Logger::getLogger("rsb.transport.spread.InPushConnector")),
    memberships(connection),
    exec(new ThreadedTaskExecutor),
    handler(new Handler(this)) {
    this->rec.reset(new ReceiverTask(connection, this->handler));
}

InPushConnector::~InPushConnector() {
    if (this->active) {
        deactivate();
    }
}

void InPushConnector::activate() {
    ConnectorBase::activate();

    this->connection->activate();

    // (re-)start threads
    this->exec->schedule(rec);
    this->active = true;

    // check that scope is applied
    if (activationScope) {
        setScope(*activationScope);
        activationScope.reset();
    }

}

void InPushConnector::deactivate() {
    ConnectorBase::deactivate();

    this->rec->cancel();
    if (this->connection->isActive()) {
        this->connection->interruptReceive();
        this->rec->waitDone();
    }
    this->connection->deactivate();
    this->active = false;
}

void InPushConnector::setQualityOfServiceSpecs(const QualityOfServiceSpec& specs) {
    this->rec->setPruning(specs.getReliability() < QualityOfServiceSpec::RELIABLE);
}

void InPushConnector::setScope(const Scope& scope) {
    if (!active) {
        activationScope.reset(new Scope(scope));
    } else {
        this->memberships.join(GroupNameCache::scopeToGroup(scope));
    }
}

void InPushConnector::setErrorStrategy(ParticipantConfig::ErrorStrategy strategy) {
    this->rec->setErrorStrategy(strategy);
}

void InPushConnector::handleIncomingNotification(rsb::protocol::NotificationPtr notification) {

    EventPtr event(new Event());

    // TODO fix error handling, see #796
    try {
        ConverterPtr converter = getConverter(notification->wire_schema());
        AnnotatedData deserialized
            = converter->deserialize(notification->wire_schema(),
                                     notification->data());

        fillEvent(event, *notification, deserialized.second, deserialized.first);

        event->mutableMetaData().setReceiveTime();

        for (std::list<eventprocessing::HandlerPtr>::iterator it = this->handlers.begin();
             it != this->handlers.end(); ++it) {
            (*it)->handle(event);
        }
    } catch (const std::exception& ex) {
        RSCWARN(this->logger, "InPushConnector::handleIncomingNotification caught std exception: " << ex.what() );
    } catch (...) {
        RSCWARN(this->logger, "InPushConnector::handleIncomingNotification caught unknown exception" );
    }

}

}
}
}
