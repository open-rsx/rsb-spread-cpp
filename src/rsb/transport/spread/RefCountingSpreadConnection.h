/* ============================================================
 *
 * This file is part of the rsb-spread project
 *
 * Copyright (C) 2015 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include <boost/thread/recursive_mutex.hpp>

#include "SpreadConnection.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * A @ref SpreadConnection which uses reference counting for #activate and
 * #deactivate calls to control the effective activation and deactivation cycle.
 *
 * @author jwienke
 */
class RSBSPREAD_EXPORT RefCountingSpreadConnection: public SpreadConnection {
public:
    RefCountingSpreadConnection(const std::string& host   = defaultHost(),
                     unsigned int port         = defaultPort());
    virtual ~RefCountingSpreadConnection();

    /**
     * Activates the connection and thereby connects to the spread daemon as
     * configured in the constructor.
     *
     * @throw CommException error connecting to the daemon
     * @throw rsc::misc::IllegalStateException already activated
     */
    virtual void activate();

    /**
     * Disconnects from the daemon.
     *
     * @pre there must be no more reader blocking in #receive
     * @throw rsc::misc::IllegalStateException already deactivated
     */
    virtual void deactivate();

private:
    rsc::logging::LoggerPtr logger;

    unsigned int counter;

    boost::recursive_mutex mutex;

};

}
}
}
