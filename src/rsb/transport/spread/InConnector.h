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

#include <stdexcept>

#include <rsb/Scope.h>
#include <rsb/ParticipantConfig.h>

#include <rsb/transport/InPushConnector.h>

#include "ConnectorBase.h"
#include "SpreadConnection.h"
#include "MembershipManager.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * Base class for in-direction connectors.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT InConnector : public virtual transport::InPushConnector,
                                     public virtual ConnectorBase {
public:
    InConnector(SpreadConnectionPtr connection);
    virtual ~InConnector();

    virtual void activate();
    virtual void deactivate();

    virtual void setScope(const Scope& scope);

    virtual void setErrorStrategy(ParticipantConfig::ErrorStrategy strategy);
protected:
    MembershipManager                memberships;
    Scope                            scope;

    ParticipantConfig::ErrorStrategy errorStrategy;
};

}
}
}
