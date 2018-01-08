/* ============================================================
 *
 * This file is a part of the rsb-spread project.
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2013, 2015, 2017 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <sp.h>

#include <rsb/CommException.h>

using namespace std;

using namespace boost;

using namespace rsc::logging;

namespace rsb {
namespace transport {
namespace spread {

#define SPREAD_MAX_GROUPS   100
#define SPREAD_MAX_MESSLEN  180000

SpreadConnection::SpreadConnection(const string& host, unsigned int port) :
    logger(Logger::getLogger("rsb.transport.spread.SpreadConnection")), connected(false),
    host(host), port(port),
#ifdef WIN32
    spreadname(str(format("%1%@%2%") % port % host)),
#else
    spreadname((host == defaultHost())
               ? lexical_cast<string>(port)
               : str(format("%1%@%2%") % port % host)),
#endif
    msgCount(0) {
    RSCDEBUG(logger, "instantiated spread connection"
             << " to spread daemon at " << spreadname);
}

SpreadConnection::~SpreadConnection() {
    RSCDEBUG(logger, "destroying SpreadConnection object");
    if (connected) {
        deactivate();
    }
}

const string SpreadConnection::getTransportURL() const {
    return boost::str(boost::format("spread://%1%:%2%")
                      % this->host % this->port);
}

void SpreadConnection::activate() {
    // XXX spread init and group join - not threadsafe, what to do about this?
    if (connected) {
        throw rsc::misc::IllegalStateException("Connection with id " + spreadpg
                + " is already active.");
    }

    RSCDEBUG(logger, "connecting to spread daemon at " << spreadname);
    char spreadPrivateGroup[MAX_GROUP_NAME];
    int ret = SP_connect(spreadname.c_str(), 0, 0, 0, &con, spreadPrivateGroup);
    spreadpg = string(spreadPrivateGroup);
    stringstream errorString;
    if (ret != ACCEPT_SESSION) {
        errorString << "Error connecting to '" << spreadname << "': ";
        switch (ret) {
        case ILLEGAL_SPREAD:
            errorString
                    << "connection to spread daemon at "
                    << spreadname << " failed, check port and hostname";
            break;
        case COULD_NOT_CONNECT:
            errorString
                    << "connection to spread daemon failed due to socket errors, check port and hostname";
            break;
        case CONNECTION_CLOSED:
            errorString
                    << "communication errors occurred during setup of connection";
            break;
        case REJECT_VERSION:
            errorString
                    << "daemon or library version mismatch";
            break;
        case REJECT_NO_NAME:
            errorString << "protocol error during setup";
            break;
        case REJECT_ILLEGAL_NAME:
            errorString
                    << "name provided violated requirement, length or illegal character";
            break;
        case REJECT_NOT_UNIQUE:
            errorString
                    << "name provided is not unique on this daemon";
            break;
        default:
            errorString << "unknown spread connect error, value: " << ret;
        }
        SP_error(ret);
        RSCFATAL(logger, errorString.str());
        throw CommException(errorString.str());
    } else {
        RSCDEBUG(logger, "success, private group id is " << spreadpg);
    }
    RSCINFO(logger, "connected to spread daemon");

    connected = true;

}

void SpreadConnection::deactivate() {

    if (!connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }
    // we can safely ignore errors here since there is no way to recover any
    // of them
    SP_disconnect(con);

    connected = false;

}

bool SpreadConnection::isActive() {
    return connected;
}

void SpreadConnection::receive(SpreadMessagePtr sm) {

    if (!connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    // read from Spread multicast group
    int serviceType;
    int numGroups;
    char sender[MAX_GROUP_NAME];
    char retGroups[SPREAD_MAX_GROUPS][MAX_GROUP_NAME];
    int16 messType;
    int dummyEndianMismatch;
    char buf[SPREAD_MAX_MESSLEN];
    int ret = SP_receive(con, &serviceType, sender, SPREAD_MAX_GROUPS,
            &numGroups, retGroups, &messType, &dummyEndianMismatch,
            SPREAD_MAX_MESSLEN, buf);

    // check for errors
    if (ret < 0) {

        string err;
        switch (ret) {
        case ILLEGAL_SESSION:
            err = "spread receive error: mbox given to receive on was illegal";
            break;
        case ILLEGAL_MESSAGE:
            err = "spread receive error: message had an illegal structure";
            break;
        case CONNECTION_CLOSED:
            err = "spread receive error: message communication errors occurred";
            break;
        case GROUPS_TOO_SHORT:
            err
                    = "spread receive error: groups array too short to hold list of groups";
            break;
        case BUFFER_TOO_SHORT:
            err
                    = "spread receive error: message body buffer too short to hold the message received";
            break;
        default:
            err = "unknown spread receive error";
        }
        throw CommException("Spread communication error. Reason: " + err);

    }

    // handle normal messages
    if (Is_regular_mess(serviceType)) {

        RSCDEBUG(logger, "regular spread message received");

        // cancel if requested
        if (numGroups == 1 && string(retGroups[0]) == string(spreadpg)) {
            throw boost::thread_interrupted();
        }

        sm->setType(SpreadMessage::REGULAR);
        sm->setData(string(buf, ret));
        if (numGroups < 0) {
            // TODO check whether we shall implement a best effort strategy here
            RSCWARN(logger,
                    "error during message reception, group array too large, requested size "
                    << " configured size " << SPREAD_MAX_GROUPS);
        }
        for (int i = 0; i < numGroups; i++) {
            if (retGroups[i] != NULL) {
                string group = string(retGroups[i]);
                RSCDEBUG(logger,
                        "received message, addressed at group with name "
                        << group);
                sm->addGroup(group);
            }
        }

    } else if (Is_membership_mess(serviceType)) {
        // this will currently never happen as we do not want to have membership messages
        // and this message does not contain any contents

        RSCINFO(logger, "received spread membership message type");
        sm->setType(SpreadMessage::MEMBERSHIP);

    } else {

        RSCFATAL(logger, "received unknown spread message type with code " << serviceType);
        assert(false);
        throw CommException(
                "Received a message that is neither membership nor data message. "
                    "This should never happen according to the spread documentation.");

    }

}

void SpreadConnection::send(const SpreadMessage& msg) {

    // TODO check message size, if larger than ~100KB throw exception
    if (!connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    const std::string& data = msg.getData();
    const std::set<std::string>& groups = msg.getGroups();
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
        ret = SP_multicast(con, msg.getQOS(), group.c_str(), 0,
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
        ret = SP_multigroup_multicast(
            con, msg.getQOS(),
            groups.size(), (const char(*)[MAX_GROUP_NAME]) groupNames, 0,
            data.size(), data.c_str());
    }

    // TODO shouldn't msgCount be incremented only in case of success?
    ++msgCount;

    if (ret < 0) {

        stringstream err;
        switch (ret) {
        case ILLEGAL_SESSION:
            err << "Illegal Session";
            break;
        case ILLEGAL_MESSAGE:
            err << "Illegal Message";
            break;
        case CONNECTION_CLOSED:
            err << "Connection Closed";
            break;
        default:
            err << "Unknown spread error with code " << ret;
            break;
        }

        throw CommException(err.str());

    }

}

void SpreadConnection::interruptReceive() {

    if (!connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }

    // See comment in SpreadConnection::send.
#if defined WIN32
    boost::mutex::scoped_lock lock(this->mutex);
#endif

    SP_multicast(con, RELIABLE_MESS, spreadpg.c_str(), 0, 0, 0);

}

unsigned long SpreadConnection::getMsgCount() {
    return msgCount;
}

mailbox* SpreadConnection::getMailbox() {
    if (!connected) {
        throw rsc::misc::IllegalStateException("Connection is not active.");
    }
    return& con;
}

string defaultHost() {
    return "localhost";
}

unsigned int defaultPort() {
    return DEFAULT_SPREAD_PORT;
}

}
}
}
