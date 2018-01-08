/* ============================================================
 *
 * This file is part of the rsb-spread project.
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

#include "MembershipManager.h"

#include "SpreadGroup.h"

namespace rsb {
namespace transport {
namespace spread {

MembershipManager::MembershipManager(SpreadConnectionPtr connection) :
    connection(connection) {
}

MembershipManager::~MembershipManager() {
}

void MembershipManager::join(const std::string& group) {
    GroupMap::iterator it = this->groups.find(group);
    if (it == this->groups.end()) {
        SpreadGroup(group).join(this->connection);
        this->groups[group] = 1;
    } else {
        it->second++;
    }
}

void MembershipManager::leave(const std::string& group) {
    GroupMap::iterator it = this->groups.find(group);
    assert(it != this->groups.end());
    if (--it->second == 0) {
        this->groups.erase(it);
        SpreadGroup(group).leave(this->connection);
    }
}

}
}
}
