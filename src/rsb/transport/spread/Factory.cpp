/* ============================================================
 *
 * This file is a part of the rsb-spread project.
 *
 * Copyright (C) 2015 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "InPushConnector.h"
#include "InPullConnector.h"
#include "OutConnector.h"

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

Factory::Factory()
    : logger(rsc::logging::Logger::getLogger("rsb.transport.spread.Factory")) {
}

rsb::transport::InPushConnector*
Factory::createInPushConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating InPushConnector with properties " << args);

    return new InPushConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            args.get<string>("host", defaultHost()),
            args.getAs<unsigned int>("port", defaultPort()));
}

rsb::transport::InPullConnector*
Factory::createInPullConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating InPullConnector with properties " << args);

    return new InPullConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            args.get<string>("host", defaultHost()),
            args.getAs<unsigned int>("port", defaultPort()));
}

rsb::transport::OutConnector*
Factory::createOutConnector(const rsc::runtime::Properties& args) {
    RSCDEBUG(this->logger, "creating OutConnector with properties " << args);
    return new OutConnector(
            args.get<ConverterSelectionStrategyPtr>("converters"),
            args.get<string>("host", defaultHost()),
            args.getAs<unsigned int>("port", defaultPort()),
            args.getAs<unsigned int>("maxfragmentsize", 100000));
}

}
}
}
