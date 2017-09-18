/* ============================================================
 *
 * This file is a part of the rsb-spread project.
 *
 * Copyright (C) 2013, 2015, 2017 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "registration.h"

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include <rsb/transport/Factory.h>

#include "Factory.h"

namespace rsb {
namespace transport {
namespace spread {

static FactoryPtr factory;
static boost::mutex registrationMutex;

void registerTransport() {
    boost::mutex::scoped_lock lock(registrationMutex);

    if (!factory) {
        factory.reset(new Factory());

        std::set<std::string> options;
        options.insert("host");
        options.insert("port");

        {
            InPushFactory& connectorFactory = getInPushFactory();

            connectorFactory.registerConnector
                ("spread",
                 boost::bind(&Factory::createInPushConnector, factory, _1),
                 "spread", true, options);
        }

        {
            InPullFactory& connectorFactory = getInPullFactory();

            connectorFactory.registerConnector
                ("spread",
                 boost::bind(&Factory::createInPullConnector, factory, _1),
                 "spread", true, options);
        }

        {
            OutFactory& connectorFactory = getOutFactory();

            connectorFactory.registerConnector
                ("spread",
                 boost::bind(&Factory::createOutConnector, factory, _1),
                 "spread", true, options);
        }
    }

}

void unregisterTransport() {
    boost::mutex::scoped_lock lock(registrationMutex);

    /* TODO
    {
        InPushFactory& connectorFactory = getInPushFactory();

        connectorFactory.unregisterConnector("spread");
    }

    {
        InPullFactory& connectorFactory = getInPullFactory();

        connectorFactory.unregisterConnector("spread");
    }

    {
        OutFactory& connectorFactory = getOutFactory();

        connectorFactory.unregisterConnector("spread");
    }*/

    factory.reset();
}

}
}
}
