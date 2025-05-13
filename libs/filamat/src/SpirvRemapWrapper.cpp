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

#include "SpirvRemapWrapper.h"

#include <SPVRemapper.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <string>

namespace filamat {

void SpirvRemapWrapperSetUp() {
    // SPIRV error handler registration should occur only once.
    // Construct this SpirvRemapWrapper object only once.
    spv::spirvbin_t::registerErrorHandler([](const std::string& str) {
        utils::slog.e << str << utils::io::endl;
    });

    // Similar to above, we need to do a no-op remap to init a static
    // table in the remapper before the jobs start using remap().
    spv::spirvbin_t remapper(0);
    // We need to provide at least a valid header to not crash.
    std::vector<uint32_t> spirv {
        0x07230203,// MAGIC
        0,         // VERSION
        0,         // GENERATOR
        0,         // BOUND
        0          // SCHEMA, must be 0
    };
    remapper.remap(spirv, 0);
}

void SpirvRemapWrapperRemap(std::vector<uint32_t>& spirv) {
    // Remove dead module-level objects: functions, types, vars
    spv::spirvbin_t remapper(0);
    remapper.remap(spirv, spv::spirvbin_base_t::DCE_ALL);
}

} // namespace filamat
