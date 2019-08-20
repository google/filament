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

#ifndef MATDBG_JSONWRITER_H
#define MATDBG_JSONWRITER_H

#include <filaflat/ChunkContainer.h>

#include <utils/CString.h>

namespace filament {
namespace matdbg {

class JsonWriter {
public:
    bool writeMaterialInfo(const filaflat::ChunkContainer& package);
    const char* getJsonString() const;
    size_t getJsonSize() const;
private:
    utils::CString mJsonString;
};

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_JSONWRITER_H
