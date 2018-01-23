/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2011-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <string>

#include <rsc/threading/TaskExecutor.h>

#include <rsb/protocol/Notification.h>

#include <rsb/transport/InPushConnector.h>
#include <rsb/transport/ConverterSelectingConnector.h>

#include "InConnector.h"

#include "ReceiverTask.h"
#include "SpreadConnection.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

class ReceiverTask;

/**
 * This class implements push-style event receiving for Spread-based
 * transport.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT InPushConnector: public virtual transport::InPushConnector,
                                        public virtual transport::ConverterSelectingConnector<std::string>,
                                        public virtual InConnector {
public:
    InPushConnector(ConverterSelectionStrategyPtr converters,
                    SpreadConnectionPtr           connection);
    virtual ~InPushConnector();

    void activate();
    void deactivate();

    void setQualityOfServiceSpecs(const QualityOfServiceSpec& specs);

    void setErrorStrategy(ParticipantConfig::ErrorStrategy strategy);

    void handleIncomingNotification(rsb::protocol::NotificationPtr notification);
private:
    class Handler;
    typedef boost::shared_ptr<Handler> HandlerPtr;

    rsc::logging::LoggerPtr         logger;

    rsc::threading::TaskExecutorPtr exec;
    boost::shared_ptr<ReceiverTask> rec;
    HandlerPtr                      handler;
};

}
}
}
