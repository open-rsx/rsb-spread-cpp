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

#include "ErrorMessages.h"

#include <boost/format.hpp>

#include <sp.h>

namespace rsb {
namespace transport {
namespace spread {

std::string spreadErrorString(int code) {
    switch (code) {
    case ILLEGAL_SPREAD:
        return "Connection to Spread daemon failed, check port and hostname";
    case COULD_NOT_CONNECT:
        return "Connection to Spread daemon failed due to socket errors "
               ", check port and hostname. Is the daemon running?";

    case REJECT_QUOTA:
        return "The Spread daemon rejected the connection. Too many clients"
               " are connected to the Spread daemon.";
    case REJECT_NO_NAME:
        return "The Spread daemon rejected the connection. No name was "
               "supplied for the new client.";
    case REJECT_ILLEGAL_NAME:
        return "The Spread daemon rejected the connection. The name "
               "provided for the client is illegal.";
    case REJECT_NOT_UNIQUE:
        return "The Spread daemon rejected the connection. The name "
               "provided for the new client is not unique within the "
               "Spread daemon.";
    case REJECT_VERSION:
        return "The Spread daemon rejected the connection. Spread daemon "
               "version and Spread library versions do not match.";
    case REJECT_AUTH:
        return "The Spread daemon rejected the connection. The client failed"
               " to authenticate to the Spread daemon.";

    case CONNECTION_CLOSED:
        return "The Spread daemon has closed the client's connection.";
    case NET_ERROR_ON_SESSION:
        return "The client lost its connection to the Spread daemon. ";

    case ILLEGAL_SESSION:
        return "An illegal mailbox was supplied to the Spread library "
               "(client-side programming error).";
    case ILLEGAL_SERVICE:
        return "An illegal service was requested from the Spread library "
               "(client-side programming error).";
    case ILLEGAL_MESSAGE:
        return "An illegal message was passed to the Spread library "
               "(client-side programming error).";
    case ILLEGAL_GROUP:
        return "An illegal group name was passed to the Spread library "
               "(client-side programming error).";

    case GROUPS_TOO_SHORT:
        return "Group buffer too short when receiving a message.";
    case BUFFER_TOO_SHORT:
        return "Message body buffer too short when receiving a message.";
    case MESSAGE_TOO_LONG:
        return "Message body too long when sending a message";

#if defined SP_BUG
    case SP_BUG:
        return "Internal error in the Spread library.";
#endif

    default:
        return boost::str(boost::format("Unknown Spread error, code: %1%")
                          % code);
    }
}

}
}
}
