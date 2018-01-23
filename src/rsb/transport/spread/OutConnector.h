/* ============================================================
 *
 * This file is a part of the rsb-spread project.
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

#pragma once

#include <string>

#include <rsb/QualityOfServiceSpec.h>

#include <rsb/transport/OutConnector.h>
#include <rsb/transport/ConverterSelectingConnector.h>

#include "ConnectorBase.h"

#include "Bus.h"

#include "GroupNameCache.h"
#include "SpreadMessage.h"

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * @author jmoringe
 */
class RSBSPREAD_EXPORT OutConnector: public virtual transport::OutConnector,
                                     public virtual transport::ConverterSelectingConnector<std::string>,
                                     public virtual ConnectorBase {
public:
    OutConnector(ConverterSelectionStrategyPtr converters,
                 BusPtr                        bus,
                 unsigned int                  maxFragmentSize = 100000);
    virtual ~OutConnector();

    void setScope(const Scope& scope);

    void handle(rsb::EventPtr event);

    void activate();
    void deactivate();

    void setQualityOfServiceSpecs(const QualityOfServiceSpec& specs);

private:

    rsc::logging::LoggerPtr logger;

    GroupNameCache          groupNameCache;

    QualityOfServiceSpec    qosSpecs;
    SpreadMessage::QOS      messageQOS;

    unsigned int            maxFragmentSize;
    /**
     * The number of bytes minimally required to successfully
     * serialize the notification with the limited size for each
     * fragment.
     */
    unsigned int            minDataSpace;

};

}
}
}
