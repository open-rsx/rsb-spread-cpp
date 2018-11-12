/* ============================================================
 *
 * This file is part of the rsb-spread project
 *
 * Copyright (C) 2010 Sebastian Wrede <swrede@techfak.uni-bielefeld.de>
 * Copyright (C) 2012-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "rsb/converter/Repository.h"

#include <rsb/transport/spread/BusImpl.h>
#include <rsb/transport/spread/InConnector.h>
#include <rsb/transport/spread/OutConnector.h>

#include "../ConnectorTest.h"

#include "testconfig.h"

using namespace std;

using namespace testing;

using namespace rsb::converter;
using namespace rsb::transport;
using namespace rsb::transport::spread;

static int dummy = pullInConnectorTest();

// Creates and returns an InConnector that uses a Bus which connects
// to a real Spread daemon.
InConnectorPtr createConnectingInConnector() {
    BusPtr bus(BusImpl::create(SpreadConnectionPtr(new SpreadConnection(
            defaultHost(), SPREAD_PORT))));
    bus->activate();
    return InConnectorPtr(new rsb::transport::spread::InConnector
                              (converterRepository<string>()
                               ->getConvertersForDeserialization(),
                               bus));
}

// Like createConnectingInConnector but creates an OutConnector.
OutConnectorPtr createConnectingOutConnector() {
    BusPtr bus(BusImpl::create(SpreadConnectionPtr(new SpreadConnection(
            defaultHost(), SPREAD_PORT))));
    bus->activate();
    return OutConnectorPtr(new rsb::transport::spread::OutConnector
                           (converterRepository<string>()
                            ->getConvertersForSerialization(),
                            bus));
}

// Creates and returns an InConnector that uses a given Bus (which
// will typically be a mock object.)
InConnectorPtr createInConnectorWithBus(BusPtr bus) {
    return InConnectorPtr(new rsb::transport::spread::InConnector
                          (converterRepository<string>()
                           ->getConvertersForDeserialization(),
                           bus));
}

// Like createInConnectorWithBus but creates an OutConnector.
OutConnectorPtr createOutConnectorWithBus(BusPtr bus) {
    return OutConnectorPtr(new rsb::transport::spread::OutConnector
                           (converterRepository<string>()
                            ->getConvertersForSerialization(),
                            bus));
}

const
ConnectorTestSetup spreadSetup(createConnectingInConnector,
                               createConnectingOutConnector,
                               createInConnectorWithBus,
                               createOutConnectorWithBus);

INSTANTIATE_TEST_CASE_P(SpreadConnector,
        ConnectorTest,
        ::testing::Values(spreadSetup))
;
