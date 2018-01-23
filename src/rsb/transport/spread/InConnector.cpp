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

#include "InConnector.h"

#include <boost/format.hpp>

#include <rsc/misc/langutils.h>
#include <rsc/debug/DebugTools.h>

#include <rsb/MetaData.h>

#include "GroupNameCache.h"

namespace rsb {
namespace transport {
namespace spread {

InConnector::InConnector(ConverterSelectionStrategyPtr converters,
                         SpreadConnectionPtr           connection) :
    ConverterSelectingConnector<std::string>(converters),
    ConnectorBase(connection),
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.InConnector")),
    memberships(connection),
    errorStrategy(ParticipantConfig::ERROR_STRATEGY_LOG) {
}

InConnector::~InConnector() {}

void InConnector::activate() {
    ConnectorBase::activate();

    this->connection->activate();

    this->memberships.join(GroupNameCache::scopeToGroup(this->scope));

    this->active = true;
}

void InConnector::deactivate() {
    ConnectorBase::deactivate();

    this->memberships.leave(GroupNameCache::scopeToGroup(this->scope));

    this->connection->deactivate();

    this->active = false;
}

void InConnector::setScope(const Scope& scope) {
    assert(!this->active);

    this->scope = scope;
}

void InConnector::setErrorStrategy(ParticipantConfig::ErrorStrategy strategy) {
    this->errorStrategy = strategy;
}

EventPtr InConnector::notificationToEvent(rsb::protocol::NotificationPtr notification) {
    EventPtr event(new Event());

    try {
        ConverterPtr converter = getConverter(notification->wire_schema());
        AnnotatedData deserialized
            = converter->deserialize(notification->wire_schema(),
                                     notification->data());

        fillEvent(event, *notification, deserialized.second, deserialized.first);

        event->mutableMetaData().setReceiveTime();
    } catch (const std::exception& exception) {
        handleError("deserializing notification", exception,
                    "Continue with next notification", "Terminating");
    }

    return event;
}

void InConnector::handleError(const std::string&    context,
                              const std::exception& exception,
                              const std::string&    continueDescription,
                              const std::string&    abortDescription) {
    rsc::debug::DebugToolsPtr tools = rsc::debug::DebugTools::newInstance();
    boost::format message
        = (boost::format("Error %1%: %2%\n\n%3%\n")
           % context % exception.what() % tools->exceptionInfo(exception));
    switch (this->errorStrategy) {
    case ParticipantConfig::ERROR_STRATEGY_LOG:
        RSCERROR(this->logger, message << continueDescription);
        break;
    case ParticipantConfig::ERROR_STRATEGY_PRINT:
        std::cerr << message << continueDescription << std::endl;
        break;
    case ParticipantConfig::ERROR_STRATEGY_EXIT:
        RSCFATAL(this->logger, message << abortDescription);
        exit(1);
        break;
    }
}

}
}
}
