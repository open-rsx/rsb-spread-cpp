/* ============================================================
 *
 * This file is a part of the rsb-spread project.
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

#include "SpreadConnection.h"

#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <rsc/misc/IllegalStateException.h>

#include <rsc/runtime/ContainerIO.h>

#include <sp.h>

#include <rsb/CommException.h>

#include "ErrorMessages.h"

using namespace rsc::logging;

namespace rsb {
namespace transport {
namespace spread {

#define SPREAD_MAX_GROUPS  100
#define SPREAD_MAX_MESSLEN 180000

std::string defaultHost() {
    return "localhost";
}

unsigned int defaultPort() {
    return DEFAULT_SPREAD_PORT;
}

SpreadConnection::SpreadConnection(const std::string& host,
                                   unsigned int       port) :
    logger(Logger::getLogger("rsb.transport.spread.SpreadConnection")),
    connected(false),
    host(host), port(port),
#ifdef WIN32
    daemonName(boost::str(boost::format("%1%@%2%") % port % host))
#else
    daemonName((host == defaultHost())
               ? boost::lexical_cast<std::string>(port)
               : boost::str(boost::format("%1%@%2%") % port % host))
#endif
    {
}

SpreadConnection::~SpreadConnection() {
    if (this->connected) {
        deactivate();
    }
}

const std::string SpreadConnection::getTransportURL() const {
    return boost::str(boost::format("spread://%1%:%2%")
                      % this->host % this->port);
}

bool SpreadConnection::isActive() const {
    return this->connected;
}

void SpreadConnection::activate() {
    if (this->connected) {
        throw rsc::misc::IllegalStateException
            (boost::str(boost::format("Connection with id %1% is already active.")
                        % this->privateGroup));
    }

    RSCINFO(this->logger, (boost::format("Connecting to Spread daemon at '%1%'")
                           % this->daemonName));
    char privateGroup[MAX_GROUP_NAME];
    int ret = SP_connect(this->daemonName.c_str(), 0, 0, 0,
                         &this->mailbox, privateGroup);
    if (ret != ACCEPT_SESSION) {
        std::string message
            = boost::str(boost::format("Error connecting to Spread daemon "
                                       "at '%1%': %2%")
                         % this->daemonName % spreadErrorString(ret));
        RSCFATAL(this->logger, message);
        throw CommException(message);
    }
    this->privateGroup = std::string(privateGroup);
    RSCINFO(this->logger, (boost::format("Connected to Spread daemon at '%1%',"
                                         " private group is '%2%'")
                           % this->daemonName % this->privateGroup));

    this->connected = true;
}

void SpreadConnection::deactivate() {
    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }
    // We can safely ignore errors here since there is no way to
    // recover anyway.
    SP_disconnect(this->mailbox);

    this->connected = false;
}

void SpreadConnection::join(const std::string& group) {

    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    int ret = SP_join(this->mailbox, group.c_str());
    if (ret != 0) {
        std::string description
            = boost::str(boost::format("Error joining Spread group '%1%': %2%")
                         % group % spreadErrorString(ret));
        RSCERROR(this->logger, description);
        throw CommException(description);
    }

}

void SpreadConnection::leave(const std::string& group) {

    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    int ret = SP_leave(this->mailbox, group.c_str());
    if (ret != 0) {
        std::string description
            = boost::str(boost::format("Error leaving Spread group '%1%': %2%")
                         % group % spreadErrorString(ret));
        RSCERROR(this->logger, description);
        throw CommException(description);
    }

}

void SpreadConnection::receive(SpreadMessage& message) {

    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    // read from Spread multicast group
    int serviceType;
    char sender[MAX_GROUP_NAME];
    int numGroups;
    char groups[SPREAD_MAX_GROUPS][MAX_GROUP_NAME];
    int16 messType;
    int dummyEndianMismatch;
    char buf[SPREAD_MAX_MESSLEN];
    int ret = SP_receive(this->mailbox, &serviceType, sender, SPREAD_MAX_GROUPS,
                         &numGroups, groups, &messType, &dummyEndianMismatch,
                         SPREAD_MAX_MESSLEN, buf);
    if (ret < 0) {
        throw CommException(boost::str(boost::format("Spread receive error: %1%")
                                       % spreadErrorString(ret)));
    }

    // handle normal messages
    if (Is_regular_mess(serviceType)) {
        // cancel if requested
        if (numGroups == 1 && std::string(groups[0]) == this->privateGroup) {
            throw boost::thread_interrupted();
        }

        message.setType(SpreadMessage::REGULAR);
        message.setData(std::string(buf, ret));
        if (numGroups < 0) {
            // TODO check whether we shall implement a best effort strategy here
            RSCWARN(this->logger,
                    "error during message reception, group array too large, requested size "
                    << " configured size " << SPREAD_MAX_GROUPS);
        }
        for (int i = 0; i < numGroups; i++) {
            message.addGroup(std::string(groups[i]));
        }
        RSCDEBUG(this->logger, "Received regular message with groups "
                 << message.getGroups());
    } else if (Is_membership_mess(serviceType)) {
        // This will currently never happen as we do not want to have
        // membership messages and this message does not contain any
        // contents.
        RSCINFO(this->logger, "received spread membership message type");
        message.setType(SpreadMessage::MEMBERSHIP);
    } else {
        RSCFATAL(this->logger, "Received unknown Spread message type with type "
                 << serviceType);
        assert(false);
        throw CommException(
                "Received a message that is neither membership nor data message. "
                "This should never happen according to the spread documentation.");
    }

}

void SpreadConnection::send(const SpreadMessage& message) {

    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    // TODO check message size, if larger than ~100KB throw exception

    const std::string& data = message.getData();
    const std::set<std::string>& groups = message.getGroups();
    if (groups.empty()) {
        assert(false);
        throw CommException("Group information missing in message");
    }

    // The Spread client library does not seem to be thread-safe on
    // win32 (despite what the documentation says).
#if defined WIN32
    boost::mutex::scoped_lock lock(this->mutex);
#endif

    int ret;
    if (groups.size() == 1) { // use SP_multicast
        const std::string& group = *groups.begin();
        assert(group.size() < MAX_GROUP_NAME);
        ret = SP_multicast(this->mailbox, message.getQOS(), group.c_str(), 0,
                           data.size(), data.c_str());
    } else { // use SP_multigroup_multicast
        char groupNames[SPREAD_MAX_GROUPS][MAX_GROUP_NAME];
        int i = 0;
        for (std::set<std::string>::const_iterator it
                 = groups.begin(); it != groups.end(); ++it, ++i) {
            assert(it->size() < MAX_GROUP_NAME);
            char* end = copy(it->begin(), it->end(), groupNames[i]);
            *end = '\0';
        }
        ret = SP_multigroup_multicast
            (this->mailbox, message.getQOS(),
             groups.size(), (const char(*)[MAX_GROUP_NAME]) groupNames, 0,
             data.size(), data.c_str());
    }
    if (ret < 0) {
        throw CommException(boost::str(boost::format("Spread send error: %1%")
                                       % spreadErrorString(ret)));
    }
}

void SpreadConnection::interruptReceive() {
    if (!this->connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    // See comment in SpreadConnection::send.
#if defined WIN32
    boost::mutex::scoped_lock lock(this->mutex);
#endif

    SP_multicast(this->mailbox, RELIABLE_MESS, this->privateGroup.c_str(), 0, 0, 0);
}

}
}
}
