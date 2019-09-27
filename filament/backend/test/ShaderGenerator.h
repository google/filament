/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_SHADERGENERATOR_H
#define TNT_SHADERGENERATOR_H

#include "PlatformRunner.h"

#include "private/backend/DriverApi.h"

#include <string>

namespace test {

class ShaderGenerator {

public:

    static void init();
    static void shutdown();

    /**
     * Generates shaders transpiled for the given backend / mobile combination.
     * @param vertex The vertex shader, written in GLSL 450 core.
     * @param fragment The framgnet shader, written in GLSL 450 core.
     */
    ShaderGenerator(std::string vertex, std::string fragment, Backend backend, bool isMobile) noexcept;

    ShaderGenerator(const ShaderGenerator& rhs) = delete;
    ShaderGenerator& operator=(const ShaderGenerator& rhs) = delete;

    filament::backend::Program getProgram() noexcept;

private:

    enum class ShaderStage {
        VERTEX,
        FRAGMENT
    };

    using Blob = std::vector<char>;
    static Blob transpileShader(Backend backend, bool isMobile, std::string shader,
            ShaderStage stage) noexcept;

    Backend mBackend;
    bool mIsMobile;

    Blob mVertexBlob;
    Blob mFragmentBlob;
    std::string mCompiledVertexShader;
    std::string mCompiledFragmentShader;

};

} // namespace test

#endif
