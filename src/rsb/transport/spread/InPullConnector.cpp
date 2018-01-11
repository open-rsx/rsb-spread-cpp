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

#include "InPullConnector.h"

#include <rsb/MetaData.h>
#include <rsb/EventId.h>

using namespace std;

using namespace rsc::logging;
using namespace rsc::runtime;

using namespace rsb::converter;

namespace rsb {
namespace transport {
namespace spread {

InPullConnector::InPullConnector(ConverterSelectionStrategyPtr converters,
                                 SpreadConnectionPtr           connection) :
    ConverterSelectingConnector<std::string>(converters),
    logger(Logger::getLogger("rsb.transport.spread.InPullConnector")),
    active(false),
    connector(new SpreadWrapper(connection)) {
}

InPullConnector::~InPullConnector() {
    if (this->active) {
        deactivate();
    }
}

void InPullConnector::printContents(ostream& stream) const {
    stream << "active = " << this->active
           << ", connector = " << this->connector;
}

const string InPullConnector::getTransportURL() const {
    return this->connector->getTransportURL();
}

void InPullConnector::activate() {
    this->connector->activate();
    this->active = true;

    // check that scope is applied
    if (activationScope) {
        setScope(*activationScope);
        activationScope.reset();
    }

}

void InPullConnector::deactivate() {
    this->connector->deactivate();
}

void InPullConnector::setQualityOfServiceSpecs(const QualityOfServiceSpec& specs) {
    this->connector->setQualityOfServiceSpecs(specs);
}

void InPullConnector::setScope(const Scope& scope) {
    if (!active) {
        this->activationScope.reset(new Scope(scope));
    } else {
        this->connector->join(this->connector->makeGroupName(scope));
    }
}

EventPtr InPullConnector::raiseEvent(bool block) {
    assert(block);

    SpreadMessage message;
    EventPtr event;
    while (true) {
        this->connector->receive(message);

        rsb::protocol::NotificationPtr notification
            = this->messageHandler.handleMessage(message);
        if (!notification) {
            continue;
        }

        EventPtr event = handleIncomingNotification(notification);
        if (!event) {
            continue;
        }
        return event;
    };
    // This should never happen so far unless non-blocking (not implemented so far)
    return EventPtr();
}

EventPtr InPullConnector::handleIncomingNotification(rsb::protocol::NotificationPtr notification) {
    EventPtr event(new Event());

    // TODO fix error handling, see #796
    try {
        ConverterPtr converter = getConverter(notification->wire_schema());
        AnnotatedData deserialized
            = converter->deserialize(notification->wire_schema(),
                                     notification->data());

        fillEvent(event, *notification, deserialized.second, deserialized.first);

        event->mutableMetaData().setReceiveTime();
    } catch (const std::exception& ex) {
        RSCWARN(this->logger, "InPushConnector::handleIncomingNotification caught std exception: " << ex.what() );
    } catch (...) {
        RSCWARN(this->logger, "InPushConnector::handleIncomingNotification caught unknown exception" );
    }

    return event;
}

}
}
}
