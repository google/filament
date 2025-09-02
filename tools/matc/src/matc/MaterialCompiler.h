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

#ifndef TNT_MATERIALCOMPILER_H
#define TNT_MATERIALCOMPILER_H

#include <string>
#include <unordered_map>

#include <filament-matp/Config.h>
#include <filament-matp/MaterialParser.h>
#include "Compiler.h"

namespace filamat {
class MaterialBuilder;
}

namespace matc {

class MaterialCompiler final: public Compiler {
public:

    bool run(const matp::Config& config) override;

    bool checkParameters(const matp::Config& config) override;

private:

    matp::MaterialParser mParser;

    bool compileRawShader(const char* glsl, size_t size, bool isDebug, matp::Config::Output* output,
                const char* ext) const noexcept;
};

} // namespace matc

#endif //TNT_MATERIALCOMPILER_H
