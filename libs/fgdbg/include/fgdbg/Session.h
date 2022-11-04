//
// Created by Fernando Raviola on 4/11/2022.
//

#ifndef FGDBG_SESSION_H
#define FGDBG_SESSION_H

#include "DebugServer.h"
#include "Resource.h"
#include "Pass.h"
#include <memory>

namespace filament::fgdbg {
class Session {

public:
    Session(std::shared_ptr<DebugServer> server) : server{ server } {}

    void addPasses(const std::vector<Pass>& passes);
    void addResources(const std::vector<Resource>& resources);
    void update() const;

private:
    std::shared_ptr<DebugServer> server;
};
}

#endif //FGDBG_SESSION_H
