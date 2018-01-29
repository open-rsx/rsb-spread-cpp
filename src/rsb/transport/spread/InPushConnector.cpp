/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2012-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "InPushConnector.h"

#include <rsc/threading/ThreadedTaskExecutor.h>

using namespace std;

using namespace rsc::logging;
using namespace rsc::runtime;
using namespace rsc::threading;

using namespace rsb::converter;

namespace rsb {
namespace transport {
namespace spread {

InPushConnector::InPushConnector(const ConverterSelectionStrategyPtr converters,
                                 BusPtr                              bus) :
    transport::ConverterSelectingConnector<string>(converters),
    ConnectorBase(bus), InConnector(converters, bus),
    logger(Logger::getLogger("rsb.transport.spread.InPushConnector")) {
}

InPushConnector::~InPushConnector() {
    if (this->active) {
        deactivate();
    }
}

void InPushConnector::setQualityOfServiceSpecs(const QualityOfServiceSpec& /*specs*/) {
    // TODO ?->setPruning(specs.getReliability() < QualityOfServiceSpec::RELIABLE);
}

void InPushConnector::handleNotification(NotificationPtr notification) {
    EventPtr event = notificationToEvent(notification);

    if (event) {
        try {
            for (std::list<eventprocessing::HandlerPtr>::iterator it = this->handlers.begin();
                 it != this->handlers.end(); ++it) {
                (*it)->handle(event);
            }
        } catch (const std::exception& exception) {
            handleError("dispatching event to handlers", exception,
                        "Continuing with next event", "Terminating");
        }
    }
}

}
}
}
