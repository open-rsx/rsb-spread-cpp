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

#include <rsb/converter/Converter.h>

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

ReceiverTask::ReceiverTask(SpreadConnectionPtr connection,
                           HandlerPtr          handler) :
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.ReceiverTask")),
    connection(connection), assemblyPool(new AssemblyPool()), handler(handler),
    errorStrategy(ParticipantConfig::ERROR_STRATEGY_PRINT) {
    RSCTRACE(logger, "ReceiverTask::ReceiverTask, SpreadConnection: " << this->connection);
}

ReceiverTask::~ReceiverTask() {
}

void ReceiverTask::execute() {
    // TODO Do performance optimization for data joining
    try {

        SpreadMessagePtr message(new SpreadMessage(SpreadMessage::REGULAR));
        this->connection->receive(message);
        if (!message) {
            throw CommException(
                    "Receiving a SpreadMessage returned a zero pointer, why?");
        }

        RSCDEBUG(logger,
                "ReceiverTask::execute new SpreadMessage received " << message);

        if (message->getType() != SpreadMessage::REGULAR) {
            return;
        }

        rsb::protocol::FragmentedNotificationPtr notification
            (new rsb::protocol::FragmentedNotification());
        if (!notification->ParseFromString(message->getData())) {
            throw CommException("Failed to parse notification in pbuf format");
        }

        RSCTRACE(this->logger,
                 "Parsed event seqnum: " << notification->notification().event_id().sequence_number());
        RSCTRACE(this->logger,
                 "Binary length: " << notification->notification().data().length());
        RSCTRACE(this->logger,
                 "Number of split message parts: " << notification->num_data_parts());
        RSCTRACE(this->logger,
                 "... received message part    : " << notification->data_part());

        // Build data from parts
        handleAndJoinFragmentedNotification(notification);
    } catch (rsb::CommException& e) {
        // TODO QoS would not like swallowing the exception
        rsc::debug::DebugToolsPtr tools = rsc::debug::DebugTools::newInstance();
        switch (this->errorStrategy) {
        case ParticipantConfig::ERROR_STRATEGY_LOG:
            RSCERROR(this->logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating receiving new spread messages!");
            break;
        case ParticipantConfig::ERROR_STRATEGY_PRINT:
            cerr << "Error receiving spread message: " << e.what() << endl
                 << tools->exceptionInfo(e) << endl
                 << "Terminating receiving new spread messages!" << endl;
            break;
        case ParticipantConfig::ERROR_STRATEGY_EXIT:
            RSCFATAL(this->logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating the whole process as requested via configuration.");
            exit(1);
            break;
        default:
            assert(false);
            RSCERROR(this->logger,
                     "Error receiving spread message: " << e.what() << endl << tools->exceptionInfo(e) << "\nTerminating receiving new spread messages!");
            break;
        }
        this->cancel();
    } catch (boost::thread_interrupted& e) {
        return;
    }

}

void ReceiverTask::handleAndJoinFragmentedNotification(
    rsb::protocol::FragmentedNotificationPtr notification) {

    rsb::protocol::NotificationPtr completeNotification;

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

    if (completeNotification) {
        this->handler->handleIncomingNotification(completeNotification);
    }
}

void ReceiverTask::setPruning(const bool& pruning) {
    this->assemblyPool->setPruning(pruning);
}

void ReceiverTask::setErrorStrategy(ParticipantConfig::ErrorStrategy strategy) {
    this->errorStrategy = strategy;
}

}
}
}
