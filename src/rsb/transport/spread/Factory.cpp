/* ============================================================
 *
 * This file is a part of the rsb-spread project.
 *
 * Copyright (C) 2015, 2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "Factory.h"

#include <rsb/converter/ConverterSelectionStrategy.h>

#include "InPushConnector.h"
#include "InPullConnector.h"
#include "OutConnector.h"

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

typedef rsb::converter::ConverterSelectionStrategy<std::string>::Ptr ConverterSelectionStrategyPtr;

Factory::Factory()
    : logger(rsc::logging::Logger::getLogger("rsb.transport.spread.Factory")) {
}

BusPtr Factory::obtainBus(const HostAndPort& options) {
    RSCDEBUG(this->logger, (boost::format("Obtaining bus for host = %1%, port = %2%")
                            % options.first % options.second));

    {
        boost::mutex::scoped_lock lock(this->busesLock);

        // Try to find an existing Bus instance for options. If there
        // is an instance, try to lock the pointer to see whether it
        // is still alive. If so, return it.
        BusMap::iterator it = this->buses.find(options);
        if (it != this->buses.end()) {
            BusPtr bus = it->second.lock();
            if (bus) {
                RSCDEBUG(this->logger, (boost::format("Found existing %1%") % bus));
                return bus;
            } else {
                this->buses.erase(it);
            }
        }

        // If there was no suitable Bus instance or the existing
        // instance was dead, create a new one and store a weak
        // pointer in the map.
        SpreadConnectionPtr connection(new SpreadConnection(options.first, options.second));
        BusPtr bus = Bus::create(connection);
        RSCDEBUG(this->logger, (boost::format("Created new %1%") % bus));
        bus->activate();
        this->buses[options] = bus;
        return bus;
    }
}

Factory::HostAndPort Factory::parseOptions(const rsc::runtime::Properties& args) {
    return make_pair(args.get  <string>      ("host", defaultHost()),
                     args.getAs<unsigned int>("port", defaultPort()));
}

rsb::transport::InPushConnector*
Factory::createInPushConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating InPushConnector with properties " << args);

    return new InPushConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            obtainBus(parseOptions(args)));
}

rsb::transport::InPullConnector*
Factory::createInPullConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating InPullConnector with properties " << args);

    return new InPullConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            obtainBus(parseOptions(args)));
}

rsb::transport::OutConnector*
Factory::createOutConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating OutConnector with properties " << args);

    return new OutConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            obtainBus(parseOptions(args)),
            args.getAs<unsigned int>("maxfragmentsize", 100000));
}

}
}
}
