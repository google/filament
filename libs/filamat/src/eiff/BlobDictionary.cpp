/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "BlobDictionary.h"

#include <assert.h>

namespace filamat {

size_t BlobDictionary::addBlob(const std::vector<uint8_t>& vblob) noexcept {
    std::string_view blob((char*) vblob.data(), vblob.size());
    auto iter = mBlobIndices.find(blob);
    if (iter != mBlobIndices.end()) {
        return iter->second;
    }
    mBlobs.emplace_back(std::make_unique<std::string>(blob));
    mBlobIndices.emplace(*mBlobs.back(), mBlobs.size() - 1);
    return mBlobs.size() - 1;
}

} // namespace filamat
