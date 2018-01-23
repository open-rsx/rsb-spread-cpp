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

#pragma once

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <rsb/Scope.h>

#include <rsb/protocol/Notification.h>
#include <rsb/protocol/FragmentedNotification.h>

#include "SpreadMessage.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

class RSBSPREAD_EXPORT Notification {
public:
    Scope                        scope;
    std::string                  wireSchema;
    std::string                  serializedPayload;
    rsb::protocol::Notification* notification;
};

typedef boost::shared_ptr<Notification> NotificationPtr;

class RSBSPREAD_EXPORT IncomingNotification : public Notification {
public:
    rsb::protocol::NotificationPtr notificationOwnership;
};

typedef boost::shared_ptr<IncomingNotification> IncomingNotificationPtr;


class RSBSPREAD_EXPORT OutgoingNotification : public Notification {
public:
    SpreadMessage::QOS                                 qos;
    std::vector<std::string>                           groups;
    std::vector<rsb::protocol::FragmentedNotification> fragments;
};

typedef boost::shared_ptr<OutgoingNotification> OutgoingNotificationPtr;

}
}
}
