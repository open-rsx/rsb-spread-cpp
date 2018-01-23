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

#include "ConnectorBase.h"

#include <rsc/misc/IllegalStateException.h>

namespace rsb {
namespace transport {
namespace spread {

ConnectorBase::ConnectorBase(BusPtr bus) :
    active(false), bus(bus) {
}

ConnectorBase::~ConnectorBase() {}

void ConnectorBase::printContents(std::ostream& stream) const {
    stream << "active = " << this->active
           << ", bus = " << this->bus;
}

const std::string ConnectorBase::getTransportURL() const {
    return this->bus->getTransportURL();
}

bool ConnectorBase::isActive() const {
    return this->active;
}

void ConnectorBase::activate() {
    if (this->active) {
        throw rsc::misc::IllegalStateException("Connector is already active");
    }
}

void ConnectorBase::deactivate() {
    if (!this->active) {
        throw rsc::misc::IllegalStateException("Connector is not active");
    }
}

}
}
}
