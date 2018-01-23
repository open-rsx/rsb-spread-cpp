/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
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

#pragma once

#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>

#include <rsc/logging/Logger.h>
#include <rsc/threading/RepetitiveTask.h>

#include <rsb/eventprocessing/ScopeDispatcher.h>

#include <rsb/protocol/Notification.h>

#include "SpreadConnection.h"
#include "DeserializingHandler.h"
#include "Notifications.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * A task that receives @c FragmentedNotifications from a @c
 * SpreadConnection, deserializes them to events and notifies a
 * handler with deserialized Events.
 *
 * Messages may be split into multiple @c FragmentedNotifications to
 * respect the spread limitations. Hence, there is an assembly
 * strategy for multiple notifications forming one Event. An optional
 * pruning for Event fragments may be enable to avoid a growing pool
 * if @c FragmentedNotifications are lost. As a default this pruning
 * is disabled.
 *
 * @author swrede
 * @author jwienke
 * @author jmoringe
 */
class RSBSPREAD_EXPORT ReceiverTask: public rsc::threading::RepetitiveTask {
public:

    class Handler {
    public:
        virtual void handleIncomingNotification(IncomingNotificationPtr notification) = 0;

        virtual void handleError(const std::exception& error) = 0;
    };
    typedef boost::shared_ptr<Handler> HandlerPtr;

    ReceiverTask(SpreadConnectionPtr connection,
                 HandlerPtr          handler);
    virtual ~ReceiverTask();

    void execute();

    /**
     * Enables or disables pruning of messages and waits until the
     * changes are performed. Thread-safe method.
     *
     * @param pruning if @c true and not pruning, start pruning, else
     *        if @c false and pruning, stop pruning
     */
    void setPruning(const bool& pruning);

private:

    /**
     * Notifies the handler of this task about a received event which
     * is generated from an internal notification and the joined data
     * that may originate from several fragments of the notification.
     *
     * @param notification notification with full data to notify about
     */
    void notifyHandler(protocol::NotificationPtr notification);

    rsc::logging::LoggerPtr logger;

    SpreadConnectionPtr     connection;
    DeserializingHandler    messageHandler;

    HandlerPtr              handler;
    boost::recursive_mutex  handlerMutex;
};

}
}
}
