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

#ifndef TNT_FILAFLAT_SHADERBUILDER_H
#define TNT_FILAFLAT_SHADERBUILDER_H

#include <utils/CString.h>

#include <stddef.h>
#include <stdint.h>

namespace filaflat {

class ShaderBuilder {
public:
    ShaderBuilder();
    ~ShaderBuilder();

    // Before using a shader buffer to create a shader you should reset it.
    void reset();

    // Tells the buffer how many characters will be appended for the full shader.
    void announce(size_t size);

    // Append a data blob to the shader. Returns true if successful.
    void appendPart(const char* data, size_t size) noexcept;

    // returns a copy of the shader string
    utils::CString getShader() const { return { mShader, mCursor }; }

    // returns the shader string. valid until next api call.
    char const* c_str() const noexcept { return mShader; }

    size_t size() const { return mCursor; }

private:
    uint32_t mCapacity = 16384;
    uint32_t mCursor = 0;
    char* mShader = nullptr;
};

} // namespace filaflat
#endif // TNT_FILAFLAT_SHADERBUILDER_H
