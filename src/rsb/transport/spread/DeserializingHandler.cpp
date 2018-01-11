/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2012-2018 Jan Moringen <jmoringe@techfak.uni-bielfeld.de>
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

#include "DeserializingHandler.h"

#include <rsb/CommException.h>

using namespace rsc::logging;

namespace rsb {
namespace transport {
namespace spread {

DeserializingHandler::DeserializingHandler() :
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.DeserializingHandler")),
    assemblyPool(new AssemblyPool()) {
}

DeserializingHandler::~DeserializingHandler() {
}

void DeserializingHandler::setPruning(const bool& pruning) {
    this->assemblyPool->setPruning(pruning);
}

rsb::protocol::NotificationPtr
DeserializingHandler::handleMessage(const SpreadMessage& message) {
    // Ignore all non-regular messages.
    if (message.getType() != SpreadMessage::REGULAR) {
        return rsb::protocol::NotificationPtr();
    }

    // Deserialize notification fragment from Spread message.
    rsb::protocol::FragmentedNotificationPtr
        fragment(new rsb::protocol::FragmentedNotification());
    if (!fragment->ParseFromString(message.getData())) {
        throw CommException("Failed to parse notification in pbuf format");
    }

    RSCDEBUG(this->logger, "Notification with"
             << " sequence number = "<< fragment->notification().event_id().sequence_number()
             << " length = " << fragment->notification().data().length()
             << " fragment = " << fragment->data_part()
             << "/" << fragment->num_data_parts());

    // Assemble complete notification from parts, if necessary.
    if (fragment->num_data_parts() > 1) {
        return this->assemblyPool->add(fragment);
    } else {
        return rsb::protocol::NotificationPtr
            (fragment->mutable_notification(),
             rsc::misc::ParentSharedPtrDeleter
             <rsb::protocol::FragmentedNotification>(fragment));
    }
}

}
}
}
