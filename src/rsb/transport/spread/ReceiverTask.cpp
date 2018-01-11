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

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

ReceiverTask::ReceiverTask(SpreadConnectionPtr connection,
                           HandlerPtr          handler) :
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.ReceiverTask")),
    connection(connection), handler(handler),
    errorStrategy(ParticipantConfig::ERROR_STRATEGY_PRINT) {
    RSCTRACE(logger, "ReceiverTask::ReceiverTask, SpreadConnection: " << this->connection);
}

ReceiverTask::~ReceiverTask() {
}

void ReceiverTask::execute() {
    // TODO Do performance optimization for data joining
    try {
        SpreadMessage message;
        this->connection->receive(message);

        rsb::protocol::NotificationPtr notification
            = this->messageHandler.handleMessage(message);
        if (notification) {
            this->handler->handleIncomingNotification(notification);
        }
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

void ReceiverTask::setPruning(const bool& pruning) {
    this->messageHandler.setPruning(pruning);
}

void ReceiverTask::setErrorStrategy(ParticipantConfig::ErrorStrategy strategy) {
    this->errorStrategy = strategy;
}

}
}
}
