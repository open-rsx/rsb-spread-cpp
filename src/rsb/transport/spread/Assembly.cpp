/* ============================================================
 *
 * This file is part of the rsb-spread project.
 *
 * Copyright (C) 2011-2018 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include "Assembly.h"

#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

#include <rsb/protocol/ProtocolException.h>

using namespace boost;
using namespace boost::posix_time;

using namespace rsc::threading;

namespace rsb {
namespace transport {
namespace spread {

Assembly::Assembly(rsb::protocol::FragmentedNotificationPtr notification) :
    logger(rsc::logging::Logger::getLogger(boost::str(boost::format("rsb.transport.spread.Assembly[%1%]")
                                                      % notification->notification().event_id().sequence_number()))),
    receivedParts(0), birthTime(microsec_clock::local_time()) {
    this->store.resize(notification->num_data_parts());
    add(notification);
}

Assembly::~Assembly() {
}

rsb::protocol::NotificationPtr Assembly::getCompleteNotification() const {
    RSCTRACE(this->logger, "Joining fragments");
    assert(isComplete());

    rsb::protocol::NotificationPtr notification
        (store[0]->mutable_notification(),
         rsc::misc::ParentSharedPtrDeleter
         < rsb::protocol::FragmentedNotification > (store[0]));

    // Concatenate data parts
    std::string* resultData = notification->mutable_data();
    for (unsigned int i = 1; i < this->store.size(); ++i) {
        resultData->append(store[i]->notification().data());
    }
    return notification;
}

bool Assembly::add(rsb::protocol::FragmentedNotificationPtr fragment) {
    RSCTRACE(this->logger,
             "Adding notification " << fragment->notification().event_id().sequence_number()
             << " (part " << fragment->data_part() << "/" << this->store.size() << ")"
             << " to assembly");
    assert(fragment->num_data_parts() == this->store.size());
    //assert(!store[fragment->data_part()]);

    if (this->store[fragment->data_part()]) {
        throw rsb::protocol::ProtocolException
            (boost::str(boost::format("Received fragment (%d/%d) of notification "
                                      "for event with sender id %x and sequence "
                                      "number %d twice!.")
                        % fragment->data_part() % fragment->num_data_parts()
                        % fragment->notification().event_id().sender_id()
                        % fragment->notification().event_id().sequence_number()));
    }
    this->store[fragment->data_part()] = fragment;
    ++this->receivedParts;
    return isComplete();
}

bool Assembly::isComplete() const {
    return this->receivedParts == this->store.size();
}

unsigned int Assembly::age() const {
    return (microsec_clock::local_time() - this->birthTime).total_seconds();
}

AssemblyPool::PruningTask::PruningTask(Pool&                   pool,
                                       boost::recursive_mutex& poolMutex,
                                       const unsigned&         ageS,
                                       const unsigned int&     pruningIntervalMs) :
    PeriodicTask(pruningIntervalMs),
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.AssemblyPool.PruningTask")),
    pool(pool), poolMutex(poolMutex), maxAge(ageS) {
}

void AssemblyPool::PruningTask::execute() {
    boost::recursive_mutex::scoped_lock lock(this->poolMutex);

    RSCDEBUG(this->logger, "Scanning for old assemblies");
    Pool::iterator it = this->pool.begin();
    while (it != this->pool.end()) {
        if (it->second->age() > maxAge) {
            RSCDEBUG(logger, "Pruning old assembly " << it->second);
            Pool::iterator temp = it++;
            this->pool.erase(temp); // FIXME returns next iterator in C++11
        } else {
            ++it;
        }
    }

}

AssemblyPool::AssemblyPool(const unsigned int& ageS,
                           const unsigned int& pruningIntervalMs) :
    logger(rsc::logging::Logger::getLogger("rsb.transport.spread.AssemblyPool")),
    pruningAgeS(ageS), pruningIntervalMs(pruningIntervalMs) {
    if (ageS == 0) {
        throw std::domain_error("Age must not be 0.");
    }
    if (pruningIntervalMs == 0) {
        throw std::domain_error("Pruning interval must not be 0");
    }
}

AssemblyPool::~AssemblyPool() {
    setPruning(false);
}

bool AssemblyPool::isPruning() const {
    boost::recursive_mutex::scoped_lock lock(this->pruningMutex);
    return this->pruningTask.get();
}

void AssemblyPool::setPruning(bool prune) {
    boost::recursive_mutex::scoped_lock lock(this->pruningMutex);

    if (!isPruning() && prune) {
        RSCDEBUG(this->logger, "Starting Assembly pruning");
        this->pruningTask.reset
            (new PruningTask(this->pool, this->poolMutex,
                             this->pruningAgeS, this->pruningIntervalMs));
        this->executor.schedule(this->pruningTask);
    } else if (isPruning() && !prune) {
        RSCDEBUG(this->logger, "Stopping Assembly pruning");
        assert(this->pruningTask);
        this->pruningTask->cancel();
        this->pruningTask->waitDone();
        RSCDEBUG(this->logger, "Assembly pruning stopped");
    }
}

rsb::protocol::NotificationPtr
AssemblyPool::add(rsb::protocol::FragmentedNotificationPtr notification) {
    boost::recursive_mutex::scoped_lock lock(this->poolMutex);

    std::string key = notification->notification().event_id().sender_id();
    boost::uint64_t sequenceNumber
      = notification->notification().event_id().sequence_number();
    key.push_back((sequenceNumber & 0x000000ff) >> 0);
    key.push_back((sequenceNumber & 0x0000ff00) >> 8);
    key.push_back((sequenceNumber & 0x00ff0000) >> 16);
    key.push_back((sequenceNumber & 0xff000000) >> 24);
    Pool::iterator it = this->pool.find(key);
    rsb::protocol::NotificationPtr result;
    AssemblyPtr assembly;
    if (it != this->pool.end()) {
        // Push message to existing Assembly
        assembly = it->second;
        RSCTRACE(this->logger,
                "Adding notification "
                 << notification->notification().event_id().sequence_number()
                 << " to existing assembly " << assembly);
        assembly->add(notification);
    } else {
        // Create new Assembly
        RSCTRACE(this->logger,
                "Creating new assembly for notification "
                 << notification->notification().event_id().sequence_number());
        assembly.reset(new Assembly(notification));
        it = this->pool.insert(std::make_pair(key, assembly)).first;
    }

    if (assembly->isComplete()) {
        result = assembly->getCompleteNotification();
        this->pool.erase(it);
    }

    RSCTRACE(this->logger, "dataPool size: " << this->pool.size());

    return result;
}

}
}
}
