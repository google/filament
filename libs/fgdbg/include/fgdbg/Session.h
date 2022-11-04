//
// Created by Fernando Raviola on 4/11/2022.
//

#ifndef FGDBG_SESSION_H
#define FGDBG_SESSION_H

#include "DebugServer.h"
#include "Resource.h"
#include "Pass.h"
#include <memory>
#include <string>

namespace filament::fgdbg {
class Session {

public:
    Session(const std::string& name, std::shared_ptr<DebugServer> server) : name{ name },
                                                                            server{ server } {}

    void addPasses(const std::vector<Pass>& passes);
    void addResources(const std::vector<Resource>& resources);
    void update() const;

private:
    std::shared_ptr<DebugServer> server;
    const std::string& name;
};
}

#endif //FGDBG_SESSION_H
