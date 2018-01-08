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

#include "SpreadMessage.h"

#include <stdexcept>

#include <rsc/logging/Logger.h>

#include <sp.h>

using namespace std;
using namespace rsc::logging;

namespace {
    LoggerPtr logger(Logger::getLogger("rsb.transport.spread.SpreadMessage"));
}

namespace rsb {
namespace transport {
namespace spread {

SpreadMessage::SpreadMessage() :
    type(OTHER), qos(UNRELIABLE) {
}

SpreadMessage::SpreadMessage(const Type& mt) :
    type(mt), qos(UNRELIABLE) {
}

SpreadMessage::SpreadMessage(const string& d) :
    type(OTHER), qos(UNRELIABLE), data(d) {
}

SpreadMessage::SpreadMessage(const char* buf) :
    type(OTHER), qos(UNRELIABLE), data(buf) {
}

SpreadMessage::~SpreadMessage() {
}

SpreadMessage::Type SpreadMessage::getType() const {
    return this->type;
}

void SpreadMessage::setType(Type type) {
    this->type = type;
}

SpreadMessage::QOS SpreadMessage::getQOS() const {
    return this->qos;
}

void SpreadMessage::setQOS(const QOS& qos) {
    this->qos = qos;
}

const std::string& SpreadMessage::getData() const {
    return this->data;
}

std::string& SpreadMessage::mutableData() {
    return this->data;
}

void SpreadMessage::setData(const std::string& data) {
    this->data = data;
}

void SpreadMessage::setData(const char* buf) {
    this->data = std::string(buf);
}

int SpreadMessage::getSize() const {
    return data.length();
}

const std::set<std::string>& SpreadMessage::getGroups() const {
    return this->groups;
}

void SpreadMessage::addGroup(const std::string& name) {
    if (name.size() > MAX_GROUP_NAME - 1) {
        throw std::invalid_argument(
                "Group name '" + name + "' is too long for spread.");
    }
    this->groups.insert(name);
}

}
}
}
