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
#include <cstdlib>
#include <cstring>

#include <utils/Log.h>

namespace filaflat {

ShaderBuilder::ShaderBuilder() : mCursor(0), mCapacity(16384) {
    mShader = static_cast<char*>(malloc(mCapacity));
}

ShaderBuilder::~ShaderBuilder() {
    free(mShader);
}

void ShaderBuilder::reset() {
    mCursor = 0;
    mShader[mCursor] = '\0';
}

void ShaderBuilder::announce(size_t size) {
    if (size > mCapacity) {
        mShader = (char*)realloc(mShader, size);
        mCapacity = size;
    }
}

bool ShaderBuilder::appendPart(const char *data, size_t size) {
    size_t available = mCapacity - mCursor;
    if (size > available) {
        assert(!"Not enough capacity in ShaderBuilder.");
        return false;
    }
    memcpy(mShader + mCursor, data, size);
    mCursor += size;
    return true;
}

}
