/* ============================================================
 *
 * This file is part of the rsb-spread project
 *
 * Copyright (C) 2010 Sebastian Wrede <swrede@techfak.uni-bielefeld.de>
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rsb/transport/spread/MembershipManager.h>

#include "testconfig.h"

using namespace rsb::transport::spread;

using namespace testing;

TEST(MembershipManagerTest, testRoundtrip)
{
    // TODO convert this to a mock-only test case
    SpreadConnectionPtr sp(
            new SpreadConnection(defaultHost(), SPREAD_PORT));
    sp->activate();

    MembershipManager mm(sp);

    ASSERT_NO_THROW(mm.join("a"));
    mm.join("a");
    // join a different group
    mm.join("b");
    // leave it
    mm.leave("b");
    // leave a
    mm.leave("a");
    mm.leave("a");
    // re-join a previously left group
    mm.join("b");
    // left b as well
    ASSERT_NO_THROW(mm.leave("b"));
}
