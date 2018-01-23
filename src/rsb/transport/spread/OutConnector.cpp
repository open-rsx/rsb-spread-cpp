/* ============================================================
 *
 * This file is a part of the rsb-spread project.
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

#include "OutConnector.h"

#include <rsc/misc/langutils.h>

#include <rsb/MetaData.h>
#include <rsb/EventId.h>
#include <rsb/Scope.h>

#include <rsb/protocol/ProtocolException.h>
#include <rsb/protocol/FragmentedNotification.h>

using namespace std;

using namespace rsc::runtime;
using namespace rsc::logging;

using namespace rsb::protocol;
using namespace rsb::converter;

namespace rsb {
namespace transport {
namespace spread {

OutConnector::OutConnector(ConverterSelectionStrategyPtr converters,
                           BusPtr                        bus,
                           unsigned int                  maxFragmentSize) :
    transport::ConverterSelectingConnector<string>(converters),
    ConnectorBase(bus),
    logger(Logger::getLogger("rsb.transport.spread.OutConnector")),
    qosSpecs(QualityOfServiceSpec(QualityOfServiceSpec::ORDERED,
                                  QualityOfServiceSpec::RELIABLE)),
    messageQOS(SpreadMessage::FIFO),
    maxFragmentSize(maxFragmentSize), minDataSpace(5) {
}

OutConnector::~OutConnector() {
    if (this->active) {
        deactivate();
    }
}

void OutConnector::setScope(const Scope& /*scope*/) {
}

void OutConnector::activate() {
    ConnectorBase::activate();

    this->active = true;
}

void OutConnector::deactivate() {
    ConnectorBase::deactivate();

    this->active = false;
}

void OutConnector::setQualityOfServiceSpecs(const QualityOfServiceSpec& specs) {
    this->qosSpecs = specs;
    switch (specs.getOrdering()) {
    case QualityOfServiceSpec::UNORDERED:
        switch (specs.getReliability()) {
        case QualityOfServiceSpec::UNRELIABLE:
            this->messageQOS = SpreadMessage::UNRELIABLE;
            break;
        case QualityOfServiceSpec::RELIABLE:
            this->messageQOS = SpreadMessage::RELIABLE;
            break;
        };
        break;
    case QualityOfServiceSpec::ORDERED:
        switch (specs.getReliability()) {
        case QualityOfServiceSpec::UNRELIABLE:
            this->messageQOS = SpreadMessage::FIFO;
            break;
        case QualityOfServiceSpec::RELIABLE:
            this->messageQOS = SpreadMessage::FIFO;
            break;

        };
        break;
    }
}

void OutConnector::handle(EventPtr event) {
    // Store send time in the event. The sending informer could in
    // principle inspect this.
    event->mutableMetaData().setSendTime(rsc::misc::currentTimeMicros());

    // Create a list of all fragments required to send this event in
    // one or more Spread messages.
    OutgoingNotificationPtr notification(new OutgoingNotification());
    notification->scope  = event->getScope();
    notification->qos    = this->messageQOS;
    notification->groups = this->groupNameCache.scopeToGroups(notification->scope);

    // TODO exception handling if converter is not available
    std::string& wire = notification->serializedPayload;
    ConverterPtr converter = getConverter(event->getType());
    std::string wireSchema
        = converter->serialize(std::make_pair(event->getType(),
                                              event->getData()),
                               wire);
    notification->wireSchema = wireSchema;

    for (unsigned int fragment = 0, offset = 0;
         (fragment == 0) || (offset < wire.size());
         ++fragment, offset += this->maxFragmentSize) {
        // Allocate and populate a new fragment. When processing the
        // first fragment, transmit all meta data.
        notification->fragments.resize(fragment + 1);
        FragmentedNotification& fragmentNotification
            = notification->fragments.back();
        fillNotificationId(*(fragmentNotification.mutable_notification()), event);
        if (fragment == 0) {
            notification->notification = fragmentNotification.mutable_notification();
            fillNotificationHeader(*(fragmentNotification.mutable_notification()),
                                   event, wireSchema);
        }

        // Use remaining space in fragment for payload data.
        unsigned int headerByteSize = fragmentNotification.ByteSize();
        assert(headerByteSize <= maxFragmentSize - minDataSpace);
        if (headerByteSize >= maxFragmentSize - minDataSpace) {
            throw ProtocolException(
                    "The meta data of this event are too big for Spread!");
        }
        unsigned int maxDataPartSize = maxFragmentSize - headerByteSize;

        string dataPart = wire.substr(offset, maxDataPartSize);
        fragmentNotification.mutable_notification()->set_data(dataPart);
        fragmentNotification.set_data_part(fragment);

        // Optimistic guess for the number of required fragments.
        fragmentNotification.set_num_data_parts(1);
    }

    this->bus->handleOutgoingNotification(notification);
}

}
}
}
