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

#include <rsb/eventprocessing/ScopeDispatcher.h>

#include "Bus.h"
#include "Notifications.h"
#include "SpreadConnection.h"
#include "MembershipManager.h"
#include "ReceiverTask.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * Implementation of the @ref Bus interface.
 *
 * Use a rsc::threading::TaskExecutor do asynchronously drive the
 * Spread connection, a @ref MembershipManager for managing Spread
 * group memberships of the connection and a @ref
 * rsb::eventprocessing::ScopeDispatcher to route events to local
 * sinks.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT BusImpl : public Bus,
                                 public rsc::runtime::Printable,
                                 public boost::enable_shared_from_this<BusImpl> {
public:

    // Since this class uses shared_from_this, there better be no way
    // of obtaining an instance that is not owned by a shared_ptr.
    static BusPtr create(SpreadConnectionPtr connection);
    virtual ~BusImpl();

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

    BusImpl(SpreadConnectionPtr connection);

    void sendNotification(OutgoingNotificationPtr notification);
};

}
}
}
