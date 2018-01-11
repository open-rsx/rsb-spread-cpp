#include "ConnectorBase.h"

#include <rsc/misc/IllegalStateException.h>

namespace rsb {
namespace transport {
namespace spread {

ConnectorBase::ConnectorBase(SpreadConnectionPtr connection) :
    active(false), connection(connection) {
}

ConnectorBase::~ConnectorBase() {}

void ConnectorBase::printContents(std::ostream& stream) const {
    stream << "active = " << this->active
           << ", connection = " << this->connection;
}

const std::string ConnectorBase::getTransportURL() const {
    return this->connection->getTransportURL();
}

bool ConnectorBase::isActive() const {
    return this->active;
}

void ConnectorBase::activate() {
    if (this->active) {
        throw rsc::misc::IllegalStateException("Connector is already active");
    }
}

void ConnectorBase::deactivate() {
    if (!this->active) {
        throw rsc::misc::IllegalStateException("Connector is not active");
    }
}

}
}
}
