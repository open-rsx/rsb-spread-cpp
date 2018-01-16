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
#include <ostream>

#include <rsb/transport/Connector.h>

#include "SpreadConnection.h"

namespace rsb {
namespace transport {
namespace spread {

class ConnectorBase : public virtual transport::Connector {
public:
    ConnectorBase(SpreadConnectionPtr connection);
    virtual ~ConnectorBase();

    virtual void printContents(std::ostream& stream) const;

    virtual const std::string getTransportURL() const;

    virtual bool isActive() const;

    virtual void activate();

    virtual void deactivate();
protected:
    bool                active;

    SpreadConnectionPtr connection;
};

}
}
}
