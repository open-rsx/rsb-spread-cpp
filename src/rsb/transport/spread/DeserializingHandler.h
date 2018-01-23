/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
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
 * ============================================================  */

#pragma once

#include <rsc/logging/Logger.h>

#include <rsb/protocol/FragmentedNotification.h>

#include "SpreadMessage.h"
#include "Assembly.h"
#include "Notifications.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * Deserializes @ref SpreadMessage objects into @ref
 * rsb::protocol::Notification objects.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT DeserializingHandler {
public:
    DeserializingHandler();
    virtual ~DeserializingHandler();

    /**
     * Enables or disables pruning of messages.
     *
     * @param pruning if @c true and not pruning, start pruning, else
     *        if @c false and pruning, stop pruning
     */
    void setPruning(const bool& pruning);

    /**
     * Handles received Spread messages.
     *
     * Extracts notifications and joins fragmented payloads in case of
     * multiple fragment messages.
     *
     * @param message Spread message to handle
     * @return pointer to the joined notification
     */
    IncomingNotificationPtr handleMessage(const SpreadMessage& message);
private:
    rsc::logging::LoggerPtr logger;

    AssemblyPoolPtr assemblyPool;

    rsb::protocol::NotificationPtr
    maybeJoinFragments(rsb::protocol::FragmentedNotificationPtr fragment);
};

}
}
}
