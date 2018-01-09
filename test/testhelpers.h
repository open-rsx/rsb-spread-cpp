/* ============================================================
 *
 * This file is part of the rsb-spread project
 *
 * Copyright (C) 2010 Johannes Wienke <jwienke@techfak.uni-bielefeld.de>
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

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdlib.h>
#endif

#include <iostream>
#include <ostream>
#include <stdexcept>

#include <boost/thread.hpp>

#include <rsc/logging/LoggerFactory.h>
#include <rsc/subprocess/Subprocess.h>

#include <gtest/gtest.h>

#include "testconfig.h"

inline void setupLogging() {

    rsc::logging::LoggerFactory::getInstance().reconfigure(
            rsc::logging::Logger::LEVEL_TRACE);

}

inline void disableExternalConfigFiles() {
#if defined(_WIN32)
    SetEnvironmentVariable("RSB_CONFIG_FILES", "%pwd");
#else
    setenv("RSB_CONFIG_FILES", "%pwd", 1);
#endif
}

inline rsc::subprocess::SubprocessPtr startSpread() {
    std::vector<std::string> spreadArgs;
    spreadArgs.push_back("-n");
    spreadArgs.push_back("localhost");
    spreadArgs.push_back("-c");
    spreadArgs.push_back(SPREAD_CONFIG_FILE);
    std::cout << "Calling " << SPREAD_EXECUTABLE << " with args:";
    for (std::vector<std::string>::iterator it = spreadArgs.begin();
            it != spreadArgs.end(); ++it) {
        std::cout << *it << ", ";
    }
    std::cout << std::endl;
    rsc::subprocess::SubprocessPtr proc =
            rsc::subprocess::Subprocess::newInstance(SPREAD_EXECUTABLE,
                    spreadArgs);
    boost::this_thread::sleep(boost::posix_time::seconds(2));
    return proc;
}

/**
 * An Environment for googletest which starts spread.
 *
 * @author jwienke
 */
class SpreadEnvironment: public testing::Environment {
public:
    inline virtual ~SpreadEnvironment() {
    }
    inline virtual void SetUp() {
        spreadProcess = startSpread();
    }
    inline virtual void TearDown() {
        spreadProcess.reset();
    }
private:
    rsc::subprocess::SubprocessPtr spreadProcess;
};
