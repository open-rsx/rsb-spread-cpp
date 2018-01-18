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

#include "GroupNameCache.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>

#include <rsb/util/MD5.h>

#include <sp.h>

namespace rsb {
namespace transport {
namespace spread {

const std::vector<std::string>& GroupNameCache::scopeToGroups(const Scope& scope) {
    boost::upgrade_lock<boost::shared_mutex> lock(this->mutex);

    {
        Cache::const_iterator it = this->cache.find(scope);
        if (it != this->cache.end()) {
            return it->second;
        }
    }

    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        // avoid flooding the cache
        // rationale: normally there is only a limited amount of group
        // names used in a system. In other cases we assume that the
        // group names are created dynamically and in this case the
        // cache won't help at all
        if (this->cache.size() > 300) {
            this->cache.clear();
        }

        std::vector<std::string>& cacheItem = this->cache[scope];
        std::vector<Scope> scopes = scope.superScopes(true);
        for (std::vector<Scope>::const_iterator it = scopes.begin();
             it != scopes.end(); ++it) {
            cacheItem.push_back(this->scopeToGroup(*it));
        }

        return cacheItem;
    }
}

std::string GroupNameCache::scopeToGroup(const Scope& scope) {
    return rsb::util::MD5(scope.toString()).toHexString().substr(0, MAX_GROUP_NAME - 1);
}

}
}
}
