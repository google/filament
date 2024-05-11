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

#ifndef TNT_FILAMAT_BLOBDICTIONARY_H
#define TNT_FILAMAT_BLOBDICTIONARY_H

#include <memory>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>

namespace filamat {

// Establish a blob <-> id mapping. Note that std::string may have binary data with null characters.
class BlobDictionary {
public:
    BlobDictionary() = default;

    // Due to the presence of unique_ptr, disallow copy construction but allow move construction.
    BlobDictionary(BlobDictionary const&) = delete;
    BlobDictionary(BlobDictionary&&) = default;

    // Adds a blob if it's not already a duplicate and returns its index.
    size_t addBlob(const std::vector<uint8_t>& blob) noexcept;

    size_t getBlobCount() const noexcept {
        return mBlobs.size();
    }

    bool isEmpty() const noexcept {
        return mBlobs.size() == 0;
    }

    std::string_view getBlob(size_t index) const noexcept {
        return *mBlobs[index];
    }

private:
    std::unordered_map<std::string_view, size_t> mBlobIndices;
    std::vector<std::unique_ptr<std::string>> mBlobs;
};

} // namespace filamat

#endif // TNT_FILAMAT_BLOBDICTIONARY_H
