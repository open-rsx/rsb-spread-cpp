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

#include <string>

#include <rsc/runtime/Properties.h>

#include <rsb/transport/InPullConnector.h>
#include <rsb/transport/ConverterSelectingConnector.h>

#include "ConnectorBase.h"

#include "SpreadConnection.h"
#include "MembershipManager.h"
#include "DeserializingHandler.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * This class implements pull-style event receiving for the Spread
 * transport.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT InPullConnector: public virtual transport::InPullConnector,
                                        public virtual transport::ConverterSelectingConnector<std::string>,
                                        public virtual ConnectorBase {
public:
    typedef converter::ConverterSelectionStrategy<std::string>::Ptr ConverterSelectionStrategyPtr;

    InPullConnector(ConverterSelectionStrategyPtr converters,
                    SpreadConnectionPtr           connection);
    virtual ~InPullConnector();

    void setScope(const Scope& scope);

    void activate();
    void deactivate();

    void setQualityOfServiceSpecs(const QualityOfServiceSpec& specs);

    EventPtr raiseEvent(bool block);

private:
    rsc::logging::LoggerPtr  logger;

    MembershipManager        memberships;
    boost::shared_ptr<Scope> activationScope;
    DeserializingHandler     messageHandler;

    EventPtr handleIncomingNotification(rsb::protocol::NotificationPtr notification);
};

}
}
}
