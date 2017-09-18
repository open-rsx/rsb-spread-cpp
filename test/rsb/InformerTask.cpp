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

#include "InformerTask.h"

#include <stdlib.h>
#include <time.h>

#include <rsc/misc/langutils.h>
#include <rsc/runtime/TypeStringTools.h>

using namespace std;

using namespace rsc::threading;

using namespace rsb;
using namespace rsb::transport;

namespace rsb {
namespace test {

InformerTask::InformerTask(OutConnectorPtr p,
                           const Scope& scope,
                           const unsigned int& numEvents,
                           const unsigned int& dataSizeInBytes) :
        scope(scope), numEvents(numEvents), sentEvents(0), connector(p),
        data(new string(rsc::misc::randAlnumStr(dataSizeInBytes))) {
}

InformerTask::~InformerTask() {
}

void InformerTask::execute() {

    ++this->sentEvents;

    Scope thisScope = this->scope;
    if (this->sentEvents % 2 == 0) {
        // should be filtered
        thisScope = Scope("/other").concat(this->scope);
    }
    EventPtr event(new Event(thisScope, this->data, rsc::runtime::typeName<string>()));
    event->setId(id, this->sentEvents);

    // add causing events
    unsigned int numCauses = rand() % 10;
    for (unsigned int i = 0; i < numCauses; ++i) {
        event->addCause(EventId(rsc::misc::UUID(), rand()));
    }

    connector->handle(event);
    if (this->sentEvents % 2 == 1) {
        this->events.push_back(event);
    }
    if (this->sentEvents == 2 * this->numEvents) {
        cancel();
    }

}

std::vector<EventPtr> InformerTask::getEvents() {
    return this->events;
}

// ------

WaitingObserver::WaitingObserver(const Scope& scope,
                                 const unsigned int& expectedEvents) :
    scope(scope), expectedEvents(expectedEvents), receivedEvents(0) {
}

void WaitingObserver::handler(EventPtr event) {
    boost::recursive_mutex::scoped_lock lock(this->m);

    this->events.push_back(event);
    if (++this->receivedEvents == this->expectedEvents) {
        this->condition.notify_all();
    }
}

vector<EventPtr> WaitingObserver::getEvents() {
    boost::recursive_mutex::scoped_lock lock(this->m);

    return this->events;
}

bool WaitingObserver::waitReceived(const unsigned int& timeoutMs) {
    boost::recursive_mutex::scoped_lock lock(this->m);

    while (this->receivedEvents < this->expectedEvents) {
        if (timeoutMs == 0) {
            this->condition.wait(lock);
        } else {
            bool normalWakeup
                = condition.timed_wait(lock,
                                       boost::posix_time::milliseconds(timeoutMs));
            if (!normalWakeup) {
                return false;
            }
        }
    }
    return true;
}

const Scope& WaitingObserver::getScope() const {
    return this->scope;
}

}
}
