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

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/thread/mutex.hpp>

#include <rsc/runtime/Printable.h>

#include <rsc/threading/TaskExecutor.h>

#include <rsb/Event.h>

#include <rsb/eventprocessing/ScopeDispatcher.h>

#include "Notifications.h"
#include "SpreadConnection.h"
#include "MembershipManager.h"
#include "ReceiverTask.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

class Bus;
typedef boost::shared_ptr<Bus> BusPtr;

/**
 * Manages and arbitrates one connection to a Spread daemon.
 *
 * One or more connectors for sending (@ref OutConnector) and/or
 * receiving notifications (@ref InConnector) are associated to the bus.
 * Notifications received via the Spread connection as well as notifications
 * send over the Spread connection are distributed to all receiving connectors
 * whose scope is a super-scope of the scope of the notification in question.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT Bus : public ReceiverTask::Handler,
                             public rsc::runtime::Printable,
                             public boost::enable_shared_from_this<Bus> {
public:
    class Sink {
    public:
        virtual void handleNotification(NotificationPtr notification) = 0;

        virtual void handleError(const std::exception& error) = 0;
    };

    typedef boost::shared_ptr<Sink> SinkPtr;

    // Since this class uses shared_from_this, there better be no way
    // of obtaining an instance that is not owned by a shared_ptr.
    static BusPtr create(SpreadConnectionPtr connection);
    virtual ~Bus();

    void printContents(std::ostream& stream) const ;

    const std::string getTransportURL() const;

    void activate();
    void deactivate();

    void addSink(const Scope& scope, SinkPtr sink);
    void removeSink(const Scope& scope, const Sink* sink);

    void handleOutgoingNotification(OutgoingNotificationPtr notification);
    void handleIncomingNotification(IncomingNotificationPtr notification);
    void handleError(const std::exception& error);
private:
    typedef eventprocessing::WeakScopeDispatcher<Sink> ScopeDispatcher;

    rsc::logging::LoggerPtr         logger;

    bool                            active;

    // Connection and Spread group membership
    SpreadConnectionPtr             connection;

    MembershipManager               memberships;

    // Receiving and dispatching
    rsc::threading::TaskExecutorPtr executor;
    boost::shared_ptr<ReceiverTask> receiver;

    ScopeDispatcher                 scopeDispatcher;

    boost::mutex                    sinkMutex;

    Bus(SpreadConnectionPtr connection);

    void sendNotification(OutgoingNotificationPtr notification);
};

}
}
}
