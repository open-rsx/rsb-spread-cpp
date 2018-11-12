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

#pragma once

#include <string>
#include <utility>

#include <boost/shared_ptr.hpp>

#include <boost/thread/mutex.hpp>

#include <rsc/runtime/Properties.h>

#include <rsb/transport/InConnector.h>
#include <rsb/transport/OutConnector.h>

#include "Bus.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

class RSBSPREAD_EXPORT Factory {
public:
    Factory();

    rsb::transport::InConnector*
    createInConnector(const rsc::runtime::Properties& args);

    rsb::transport::OutConnector*
    createOutConnector(const rsc::runtime::Properties& args);
private:

    typedef std::pair<std::string, unsigned int> HostAndPort;

    typedef std::map< HostAndPort, boost::weak_ptr<Bus> > BusMap;

    rsc::logging::LoggerPtr logger;

    BusMap                  buses;

    boost::mutex            busesLock;

    BusPtr obtainBus(const HostAndPort& options);

    static HostAndPort parseOptions(const rsc::runtime::Properties& args);

};

typedef boost::shared_ptr<Factory> FactoryPtr;

}
}
}
