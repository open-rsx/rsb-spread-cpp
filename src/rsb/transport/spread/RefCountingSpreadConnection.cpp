/* ============================================================
 *
 * This file is a part of the rsb-spread project.
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

#include "RefCountingSpreadConnection.h"

#include <rsc/misc/IllegalStateException.h>

using namespace std;

namespace rsb {
namespace transport {
namespace spread {

RefCountingSpreadConnection::RefCountingSpreadConnection(const string& host,
        unsigned int port) :
        SpreadConnection(host, port), logger(
                rsc::logging::Logger::getLogger(
                        "rsb.transport.spread.RefCountingSpreadConnection")), counter(
                0) {
}

RefCountingSpreadConnection::~RefCountingSpreadConnection() {
    // fallback deactivation, no need for locking since destructor
    if (this->counter > 0) {
        RSCWARN(logger, "Destructing despite not having received a sufficient "
                << "number of deactivate calls. "
                << "Will manually disconnect the spread connection.");
        SpreadConnection::deactivate();
    }
}

void RefCountingSpreadConnection::activate() {
    boost::recursive_mutex::scoped_lock lock(this->mutex);
    RSCTRACE(logger, "activate called, counter=" << this->counter);
    if (this->counter == 0) {
        RSCDEBUG(logger, "First activation, passing down to "
                << "actual SpreadConnection for real activation.");
        SpreadConnection::activate();
    }
    ++this->counter;
}

void RefCountingSpreadConnection::deactivate() {
    boost::recursive_mutex::scoped_lock lock(this->mutex);
    RSCTRACE(logger, "deactivate called, counter=" << this->counter);
    if (this->counter == 0) {
        RSCWARN(logger, "Deactivate called despite being inactive already");
        throw rsc::misc::IllegalStateException("Deactivate called too often. "
                "This instance is already inactive");
    }
    --this->counter;
    if (this->counter == 0) {
        RSCDEBUG(logger, "Received last deactivation call. "
                << "Passing down to SpreadConnection for actual deactivation.");
        SpreadConnection::deactivate();
    }
}

}
}
}
