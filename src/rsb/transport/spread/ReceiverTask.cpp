/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2013-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "ReceiverTask.h"

#include <rsc/misc/langutils.h>
#include <rsc/debug/DebugTools.h>

#include <rsb/CommException.h>
#include <rsb/MetaData.h>
#include <rsb/EventId.h>

#include <rsb/converter/Converter.h>

#include "SpreadConnection.h"
#include "InPushConnector.h"

using namespace std;

using namespace rsc::logging;

using namespace rsb;
using namespace rsb::eventprocessing;
using namespace rsb::transport;
using namespace rsb::protocol;

namespace rsb {
namespace transport {
namespace spread {

ReceiverTask::ReceiverTask(SpreadConnectionPtr s, HandlerPtr handler,
        InPushConnector* connector) :
        logger(rsc::logging::Logger::getLogger("rsb.transport.spread.ReceiverTask")), con(
                s), connector(connector), assemblyPool(new AssemblyPool()), handler(
                handler), errorStrategy(ParticipantConfig::ERROR_STRATEGY_PRINT) {

    RSCTRACE(logger, "ReceiverTask::ReceiverTask, SpreadConnection: " << con);

}

ReceiverTask::~ReceiverTask() {
}

void ReceiverTask::execute() {
    // TODO Do performance optimization for data joining
    try {

        SpreadMessagePtr message(new SpreadMessage(SpreadMessage::REGULAR));
        con->receive(message);
        if (!message) {
            throw CommException(
                    "Receiving a SpreadMessage returned a zero pointer, why?");
        }

        RSCDEBUG(logger,
                "ReceiverTask::execute new SpreadMessage received " << message);

        if (message->getType() != SpreadMessage::REGULAR) {
            return;
        }

        FragmentedNotificationPtr notification(new FragmentedNotification());
        if (!notification->ParseFromString(message->getData())) {
            throw CommException("Failed to parse notification in pbuf format");
        }

        RSCTRACE(logger,
                 "Parsed event seqnum: " << notification->notification().event_id().sequence_number());
        RSCTRACE(logger,
                 "Binary length: " << notification->notification().data().length());
        RSCTRACE(logger,
                 "Number of split message parts: " << notification->num_data_parts());
        RSCTRACE(logger,
                 "... received message part    : " << notification->data_part());

        // Build data from parts
        NotificationPtr completeNotification =
                handleAndJoinFragmentedNotification(notification);
        if (completeNotification) {
            RSCTRACE(logger,
                     "ReceiverTask::execute fragmented notification joined, last message " << message);
            notifyHandler(completeNotification);
        }

    } catch (rsb::CommException& e) {
        // TODO QoS would not like swallowing the exception
        rsc::debug::DebugToolsPtr tools = rsc::debug::DebugTools::newInstance();
        switch (this->errorStrategy) {
        case ParticipantConfig::ERROR_STRATEGY_LOG:
            RSCERROR(logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating receiving new spread messages!");
            break;
        case ParticipantConfig::ERROR_STRATEGY_PRINT:
            cerr << "Error receiving spread message: " << e.what() << endl
                 << tools->exceptionInfo(e) << endl
                 << "Terminating receiving new spread messages!" << endl;
            break;
        case ParticipantConfig::ERROR_STRATEGY_EXIT:
            RSCFATAL(logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating the whole process as requested via configuration.");
            exit(1);
            break;
        default:
            assert(false);
            RSCERROR(logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating receiving new spread messages!");
            break;
        }
        this->cancel();
    } catch (boost::thread_interrupted& e) {
        return;
    }

}

NotificationPtr ReceiverTask::handleAndJoinFragmentedNotification(
        FragmentedNotificationPtr notification) {

    NotificationPtr completeNotification;

    bool multiPartNotification = notification->num_data_parts() > 1;
    if (multiPartNotification) {
        completeNotification = this->assemblyPool->add(notification);
    } else {
        completeNotification.reset(
                notification->mutable_notification(),
                rsc::misc::ParentSharedPtrDeleter
                        < rsb::protocol::FragmentedNotification
                        > (notification));
    }
    return completeNotification;

}

void ReceiverTask::notifyHandler(NotificationPtr notification) {

    EventPtr e(new Event());

    // TODO fix error handling, see #796
    try {
        InPushConnector::ConverterPtr c = this->connector->getConverter(
                notification->wire_schema());
        AnnotatedData deserialized = c->deserialize(
                notification->wire_schema(), notification->data());

        fillEvent(e, *notification, deserialized.second, deserialized.first);

        e->mutableMetaData().setReceiveTime();

        boost::recursive_mutex::scoped_lock lock(handlerMutex);
        if (this->handler) {
            this->handler->handle(e);
        } else {
            RSCINFO(logger, "No handler");
        }
    } catch (const std::exception& ex) {
        RSCWARN(logger, "ReceiverTask::notifyHandler caught std exception: " << ex.what() );
    } catch (...) {
        RSCWARN(logger, "ReceiverTask::notifyHandler caught unknown exception" );
    }

}

void ReceiverTask::setHandler(HandlerPtr handler) {
    boost::recursive_mutex::scoped_lock lock(handlerMutex);
    this->handler = handler;
}

void ReceiverTask::setPruning(const bool& pruning) {
    assemblyPool->setPruning(pruning);
}

void ReceiverTask::setErrorStrategy(ParticipantConfig::ErrorStrategy strategy) {
    this->errorStrategy = strategy;
}

}
}
}
