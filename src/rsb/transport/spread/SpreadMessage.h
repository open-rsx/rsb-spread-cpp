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

#pragma once

#include <string>
#include <set>

#include <boost/shared_ptr.hpp>

#include "rsb/transport/spread/rsbspreadexports.h"

namespace rsb {
namespace transport {
namespace spread {

/**
 * Default message QOS for sending is RELIABLE.
 *
 * @author swrede
 * @author jmoringe
 */
class RSBSPREAD_EXPORT SpreadMessage {
public:

    enum Type {
        REGULAR    = 0x0001,
        MEMBERSHIP = 0x0002,
        OTHER      = 0xFFFF,
    };

    /**
     * Message reliability and QoS types. For some strange reasons the int
     * values directly resemble the sp.h defines. ;)
     *
     * @author jwienke
     */
    enum QOS {
        UNRELIABLE = 0x00000001,
        RELIABLE   = 0x00000002,
        FIFO       = 0x00000004,
        CAUSAL     = 0x00000008,
        AGREED     = 0x00000010,
        SAFE       = 0x00000020
    };

    /**
     * Creates a new empty message with undefined type #OTHER and QoS
     * #UNRELIABLE.
     */
    SpreadMessage();

    /**
     * Creates a new message with the specified type and QoS #UNRELIABLE.
     *
     * @param mt message type
     */
    SpreadMessage(const Type& mt);

    /**
     * Creates a message with the specified data and message type #OTHER and QoS
     * #UNRELIABLE.
     *
     * @param d data to set
     */
    SpreadMessage(const std::string& d);

    /**
     * Creates a message with the specified data and message type #OTHER and QoS
     * #UNRELIABLE.
     *
     * @param d data to set
     */
    SpreadMessage(const char* d);

    virtual ~SpreadMessage();

    Type getType() const;
    void setType(Type type);

    QOS getQOS() const;
    void setQOS(const QOS& qos);

    const std::string& getData() const;
    std::string& mutableData();
    void setData(const std::string& data);
    void setData(const char* d);

    int getSize() const;

    const std::set<std::string>& getGroups() const;
    void addGroup(const std::string& name);
private:
    Type                  type;
    QOS                   qos;
    std::string           data;
    std::set<std::string> groups;
};

typedef boost::shared_ptr<SpreadMessage> SpreadMessagePtr;

}
}
}
