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

#include <rsc/logging/Logger.h>

#include <rsb/Scope.h>
#include <rsb/ParticipantConfig.h>

#include <rsb/protocol/Notification.h>

#include <rsb/transport/InConnector.h>
#include <rsb/transport/ConverterSelectingConnector.h>

#include "ConnectorBase.h"
#include "Notifications.h"
#include "Bus.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * Event receiving connector for the Spread-based transport.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT InConnector : public virtual transport::InConnector,
                                     public virtual ConverterSelectingConnector<std::string>,
                                     public virtual ConnectorBase,
                                     public virtual Bus::Sink {
public:
    InConnector(ConverterSelectionStrategyPtr converters,
                BusPtr                        bus);
    virtual ~InConnector();

    void activate();
    void deactivate();

    void setScope(const Scope& scope);

    void setQualityOfServiceSpecs(const QualityOfServiceSpec& specs);

    void setErrorStrategy(ParticipantConfig::ErrorStrategy strategy);

    void handleNotification(NotificationPtr notification);

    void handleError(const std::exception& error);
 private:
    rsc::logging::LoggerPtr logger;

    Scope scope;

    ParticipantConfig::ErrorStrategy errorStrategy;

    EventPtr notificationToEvent(NotificationPtr& notification);

    void handleError(const std::string&    context,
                     const std::exception& exception,
                     const std::string&    continueDescription,
                     const std::string&    abortDescription);
};

}
}
}
