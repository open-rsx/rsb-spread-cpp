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
