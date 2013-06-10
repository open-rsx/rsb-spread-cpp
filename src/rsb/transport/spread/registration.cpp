/* ============================================================
 *
 * This file is a part of the rsb-spread project.
 *
 * Copyright (C) 2013 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <boost/thread/mutex.hpp>

#include <rsb/transport/Factory.h>

#include "InPushConnector.h"
#include "InPullConnector.h"
#include "OutConnector.h"

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

static bool registered = false;
static boost::mutex registrationMutex;

void registerTransport() {
    boost::mutex::scoped_lock lock(registrationMutex);

    if (!registered) {

        {
            InPushFactory& factory = getInPushFactory();

            set<string> options;
            options.insert("host");
            options.insert("port");

            factory.registerConnector("spread", &InPushConnector::create,
                                      "spread", true, options);
        }

        {
            InPullFactory& factory = getInPullFactory();

            set<string> options;
            options.insert("host");
            options.insert("port");

            factory.registerConnector("spread", &InPullConnector::create,
                                      "spread", true, options);
        }

        {
            OutFactory& factory = getOutFactory();

            set<string> options;
            options.insert("host");
            options.insert("port");

            factory.registerConnector("spread", &OutConnector::create,
                                      "spread", true, options);
        }

        registered = true;
    }

}

void unregisterTransport() {
    boost::mutex::scoped_lock lock(registrationMutex);

    {
        InPushFactory& factory = getInPushFactory();

        // TODO factory.unregisterConnector("spread");
    }

    {
        InPullFactory& factory = getInPullFactory();

        // TODO factory.unregisterConnector("spread");
    }

    {
        OutFactory& factory = getOutFactory();

        // TODO factory.unregisterConnector("spread");
    }
}

}
}
}
