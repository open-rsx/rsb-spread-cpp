/* ============================================================
 *
 * This file is part of the rsb-spread project
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

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#if defined WIN32 // see comment in SpreadConnection::send
#include <boost/thread/mutex.hpp>
#endif

#include <rsc/logging/Logger.h>

#include "SpreadMessage.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

RSBSPREAD_EXPORT std::string defaultHost();

RSBSPREAD_EXPORT unsigned int defaultPort();

/**
 * A wrapper class providing an object-oriented interface to the Spread API.
 *
 * @note This class is generally not thread-safe. The only exception
 *       to this rule is #interruptReceive. It can be used to
 *       terminate the receiver thread of the connection.
 *
 * @author swrede
 * @author jwienke
 * @author jmoringe
 */
class RSBSPREAD_EXPORT SpreadConnection {
public:
    SpreadConnection(const std::string& host = defaultHost(),
                     unsigned int port       = defaultPort());
    virtual ~SpreadConnection();

    const std::string getTransportURL() const;

    /**
     * @name connection state management
     * @todo is this really necessary?
     */
    //@{

    /**
     * Tells if this instance is connected to spread daemon.
     *
     * @return @c true if connected
     */
    bool isActive() const;

    /**
     * Activates the connection by connecting to the Spread daemon as
     * configured in the constructor.
     *
     * @throw CommException error connecting to the daemon
     * @throw rsc::misc::IllegalStateException already activated
     */
    virtual void activate();

    /**
     * Disconnects from the daemon.
     *
     * @pre there must be no more reader blocking in #receive
     * @throw rsc::misc::IllegalStateException already deactivated
     */
    virtual void deactivate();

    //@}

    /**
     * @name group management
     */
    //@{

    /**
     * Joins the Spread group @a group.
     *
     * @param group Name of the group
     * @throw rsc::misc::IllegalStateException connection was not active
     * @throw CommException Spread error joining
     */
    void join(const std::string& group);

    /**
     * Leaves the Spread group @a group.
     *
     * @param group Name of the Spread group.
     * @throw rsc::misc::IllegalStateException connection was not active
     * @throw CommException Spread error leaving
     */
    void leave(const std::string& group);

    //@}

    /**
     * @name fundamental message exchange
     */
    //@{

    /**
     * Receives the next message from this connection into @a message.
     *
     * Blocks until a message is available.
     *
     * @param message out parameter with the message to fill with the read contents
     * @throw rsc::misc::IllegalStateException connection was not active
     * @throw CommException communication error receiving a message
     * @throw boost::thread_interrupted if receiving was interrupted using
     *                                  #interruptReceive
     *
     * @note not all readers in different threads receive all messages, one
     *       message is only received by one thread
     */
    void receive(SpreadMessage& message);

    /**
     * Sends @a message over the Spread ring.
     *
     * @param message message to send
     * @throw rsc::misc::IllegalStateException connection was not active
     * @throw CommException communication error sending the message
     */
    void send(const SpreadMessage& message);

    //@}

    /**
     * Interrupts a potential receiver blocking in the read call some time after
     * this call. The receiver may receive all queued messages before being
     * interrupted.
     *
     * @note this method may explicitly be called from a different thread than
     *       the one blocking in #receive. Nevertheless only one other thread at
     *       a time is allowed call this method.
     * @throw rsc::misc::IllegalStateException connection was not active
     */
    void interruptReceive();

private:
    rsc::logging::LoggerPtr logger;

    /**
     * A flag to indicate whether we are connected to Spread.
     */
    volatile bool connected;

    /**
     * Host for the Spread daemon.
     */
    std::string host;

    /**
     * Port for the Spread daemon.
     */
    unsigned int port;

    /**
     * The "name" of the Spread daemon.
     *
     * Can consists of port and host, e.g. 4803\@localhost or only a
     * port. See SP_connect(3) man-page for details.
     */
    std::string daemonName;

    /**
     * Handle to the underlying Spread mailbox.
     */
    // Do not use Spread's mailbox type to avoid exposing sp.h with
    // strange defines that prevent other code from compiling.
    int mailbox;

    /**
     * Private name of this connection.
     */
    std::string privateGroup;

#if defined WIN32 // see comment in SpreadConnection::send
    boost::mutex mutex;
#endif

};

typedef boost::shared_ptr<SpreadConnection> SpreadConnectionPtr;

}
}
}
