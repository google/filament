/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_COMPILER_H
#define TNT_COMPILER_H

#include "CommandlineConfig.h"
#include <filament-matp/Config.h>

#include <filamat/Package.h>

namespace matc {

class Compiler {
public:
    virtual ~Compiler() = default;

    bool compile(const matp::Config& config) {
        if (!checkParameters(config)) {
            return false;
        }
        return run(config);
    }

protected:
    bool writePackage(const filamat::Package& package, const matp::Config& config) {
        if (config.getOutputFormat() == CommandlineConfig::OutputFormat::BLOB) {
            return writeBlob(package, config);
        } else {
            return writeBlobAsHeader(package, config);
        }
    }
    virtual bool run(const matp::Config& config) = 0;
    virtual bool checkParameters(const matp::Config& config) = 0;

    // Write Package as binary to target filename
    bool writeBlob(const filamat::Package& pkg, const matp::Config& config) const noexcept;

    // Write package as a C++ array content. Use this to include material
    // in your executable/library.
    bool writeBlobAsHeader(const filamat::Package& pkg, const matp::Config& config) const noexcept;
};

} // namespace matc
#endif // TNT_COMPILER_H