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

#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/thread/shared_mutex.hpp>

#include <rsb/Scope.h>

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * A bounded cache for the mapping between scopes and Spread groups.
 *
 * @author jmoringe
 */
class RSBSPREAD_EXPORT GroupNameCache {
public:
    /**
     * Returns Spread group names for @a scope and its super-scopes.
     *
     * @param scope The scope for which Spread group names should be
     *              computed.
     * @return The computed group names.
     */
    const std::vector<std::string>& scopeToGroups(const Scope& scope);

    /**
     * Returns the Spread group corresponding to @a scope.
     *
     * @return The name of the Spread group.
     */
    static std::string scopeToGroup(const Scope& scope);
private:
    typedef std::map< Scope, std::vector<std::string> > Cache;

    Cache               cache;
    boost::shared_mutex mutex;
};

}
}
}
