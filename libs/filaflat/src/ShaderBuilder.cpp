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

#include <filaflat/ShaderBuilder.h>

#include <assert.h>
#include <stdlib.h>

#include <utils/Log.h>

namespace filaflat {

ShaderBuilder::ShaderBuilder()
        : mShader(static_cast<char*>(malloc(mCapacity))) {
}

ShaderBuilder::~ShaderBuilder() {
    free(mShader);
}

void ShaderBuilder::reset() {
    mCursor = 0;
    mShader[0] = '\0';
}

void ShaderBuilder::announce(size_t size) {
    if (size > mCapacity) {
        mCapacity = (uint32_t)size;
        mShader = (char *)realloc(mShader, size);
    }
}

void ShaderBuilder::appendPart(const char* data, size_t size) noexcept {
    size_t available = mCapacity - mCursor;
    assert(size <= available);
    memcpy(mShader + mCursor, data, size);
    mCursor += size;
}

}
