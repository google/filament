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

#include <image/Ktx1Bundle.h>

#include <utils/Panic.h>
#include <utils/string.h>

#include <algorithm>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct SerializationHeader {
    uint8_t magic[12];
    image::KtxInfo info;
    uint32_t numberOfArrayElements;
    uint32_t numberOfFaces;
    uint32_t numberOfMipmapLevels;
    uint32_t bytesOfKeyValueData;
};

static_assert(sizeof(SerializationHeader) == 16 * 4, "Unexpected header size.");

// We flatten the three-dimensional blob index using the ordering defined by the KTX spec.
inline size_t flatten(const image::Ktx1Bundle* bundle, image::KtxBlobIndex index) {
    const uint32_t nfaces = bundle->isCubemap() ? 6 : 1;
    const uint32_t nlayers = bundle->getArrayLength();
    return index.cubeFace + index.arrayIndex * nfaces + index.mipLevel * nfaces * nlayers;
}

const uint8_t MAGIC[] = {0xab, 0x4b, 0x54, 0x58, 0x20, 0x31, 0x31, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a};

}

namespace image  {

// This little wrapper lets us avoid having an STL container in the header file.
struct KtxMetadata {
    std::unordered_map<std::string, std::string> keyvals;
};

// Extremely simple contiguous storage for an array of blobs. Assumes that the total number of blobs
// is relatively small compared to the size of each blob, and that resizing individual blobs does
// not occur frequently.
struct KtxBlobList {
    std::vector<uint8_t> blobs;
    std::vector<uint32_t> sizes;

    // Obtains a pointer to the given blob.
    uint8_t* get(uint32_t blobIndex) {
        uint8_t* result = blobs.data();
        for (uint32_t i = 0; i < blobIndex; ++i) {
            result += sizes[i];
        }
        return result;
    }

    // Resizes the blob at the given index by building a new contiguous array and swapping.
    void resize(uint32_t blobIndex, uint32_t newSize) {
        uint32_t preSize = 0;
        uint32_t postSize = 0;
        for (uint32_t i = 0; i < sizes.size(); ++i) {
            if (i < blobIndex) {
                preSize += sizes[i];
            } else if (i > blobIndex) {
                postSize += sizes[i];
            }
        }
        uint32_t oldSize = sizes[blobIndex];
        std::vector<uint8_t> newBlobs(blobs.size() + newSize - oldSize);
        uint8_t const* src = blobs.data();
        uint8_t* dst = newBlobs.data();
        memcpy(dst, src, preSize);
        src += preSize;
        dst += preSize;
        memcpy(dst, src, std::min(oldSize, newSize));
        src += oldSize;
        dst += newSize;
        memcpy(dst, src, postSize);
        sizes[blobIndex] = newSize;
        blobs.swap(newBlobs);
    }
};

Ktx1Bundle::~Ktx1Bundle() = default;

Ktx1Bundle::Ktx1Bundle(uint32_t numMipLevels, uint32_t arrayLength, bool isCubemap) :
        mBlobs(new KtxBlobList), mMetadata(new KtxMetadata) {
    mNumMipLevels = numMipLevels;
    mArrayLength = arrayLength;
    mNumCubeFaces = isCubemap ? 6 : 1;
    uint64_t const totalBlobs = (uint64_t)numMipLevels * arrayLength * mNumCubeFaces;
    FILAMENT_CHECK_POSTCONDITION(totalBlobs <= (uint64_t)std::numeric_limits<uint32_t>::max())
            << "KTX dimensions overflow";
    mBlobs->sizes.resize((uint32_t)totalBlobs);
}

Ktx1Bundle::Ktx1Bundle(uint8_t const* bytes, uint32_t nbytes) :
        mBlobs(new KtxBlobList), mMetadata(new KtxMetadata) {
    FILAMENT_CHECK_POSTCONDITION(sizeof(SerializationHeader) <= nbytes) << "KTX buffer is too small";

    // First, "parse" the header by casting it to a struct.
    SerializationHeader const* header = (SerializationHeader const*) bytes;
    FILAMENT_CHECK_POSTCONDITION(memcmp(header->magic, MAGIC, 12) == 0)
            << "KTX has unexpected identifier";
    mInfo = header->info;

    // The spec allows 0 or 1 for the number of array layers and mipmap levels, but we replace 0
    // with 1 for simplicity. Technically this is a loss of information because 0 mipmaps means
    // "please generate the mips" and an array size of 1 means "make this an array texture, but
    // with only one element". For now, ignoring this distinction seems fine.
    mNumMipLevels = header->numberOfMipmapLevels ? header->numberOfMipmapLevels : 1;
    mArrayLength = header->numberOfArrayElements ? header->numberOfArrayElements : 1;
    mNumCubeFaces = header->numberOfFaces ? header->numberOfFaces : 1;

    uint64_t const totalBlobs = (uint64_t)mNumMipLevels * mArrayLength * mNumCubeFaces;
    FILAMENT_CHECK_POSTCONDITION(totalBlobs <= (uint64_t)std::numeric_limits<uint32_t>::max()) << "KTX dimensions overflow";
    mBlobs->sizes.resize((uint32_t)totalBlobs);

    FILAMENT_CHECK_POSTCONDITION(nbytes - sizeof(SerializationHeader) >= header->bytesOfKeyValueData) << "KTX metadata length exceeds buffer";

    // We use std::string to store both the key and the value. Note that the spec says the value can
    // be a binary blob that contains null characters.
    uint8_t const* pdata = bytes + sizeof(SerializationHeader);
    uint8_t const* end = pdata + header->bytesOfKeyValueData;
    while (pdata < end) {
        FILAMENT_CHECK_POSTCONDITION((size_t)(end - pdata) >= sizeof(uint32_t)) << "KTX truncation in metadata";
        const uint32_t keyAndValueByteSize = *((uint32_t const*) pdata);
        pdata += sizeof(uint32_t);
        FILAMENT_CHECK_POSTCONDITION(keyAndValueByteSize <= (size_t)(end - pdata)) << "KTX metadata entry exceeds bounds";

        // Use std::find to safely find the null terminator
        const char* keyStart = (const char*) pdata;
        const char* keyEnd = (const char*) std::find(keyStart, keyStart + keyAndValueByteSize, '\0');
        size_t const keyLength = keyEnd - keyStart;
        FILAMENT_CHECK_POSTCONDITION(keyLength < keyAndValueByteSize) << "KTX metadata key is not null terminated";

        std::string key(keyStart, keyLength);
        uint8_t const* pval = pdata + keyLength + 1;
        size_t const valLength = keyAndValueByteSize - (keyLength + 1);

        pdata += keyAndValueByteSize;
        std::string val((const char*) pval, valLength);
        mMetadata->keyvals.insert({key, val});

        const uint32_t paddingSize = 3 - ((keyAndValueByteSize + 3) % 4);
        FILAMENT_CHECK_POSTCONDITION(paddingSize <= (size_t)(end - pdata)) << "KTX metadata padding exceeds bounds";
        pdata += paddingSize;
    }

    // There is no compressed format that has a block size that is not a multiple of 4, so these
    // two padding constants can be safely hardcoded to 0. They are here for spec consistency.
    const uint32_t cubePadding = 0;
    const uint32_t mipPadding = 0;

    // One aspect of the KTX spec is that the semantics differ for non-array cubemaps.
    const bool isNonArrayCube = mNumCubeFaces > 1 && mArrayLength == 1;
    const uint32_t facesPerMip = mArrayLength * mNumCubeFaces;

    // Extract blobs from the serialized byte stream. First measure required memory to avoid heap overflow and state mismatch.
    uint8_t const* scan_pdata = pdata;
    uint8_t const* b_end = bytes + nbytes;
    uint64_t measuredTotalSize = 0;

    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {
        FILAMENT_CHECK_POSTCONDITION((size_t)(b_end - scan_pdata) >= sizeof(uint32_t)) << "KTX truncation during image sizes";
        const uint32_t imageSize = *((uint32_t const*) scan_pdata);
        scan_pdata += sizeof(uint32_t);

        const uint32_t faceSize = isNonArrayCube ? imageSize : (imageSize / facesPerMip);
        const uint64_t levelSize = (uint64_t)faceSize * mNumCubeFaces * mArrayLength;

        FILAMENT_CHECK_POSTCONDITION(levelSize <= (size_t)(b_end - scan_pdata)) << "KTX image data exceeds buffer";
        scan_pdata += levelSize;

        measuredTotalSize += levelSize;
        FILAMENT_CHECK_POSTCONDITION(measuredTotalSize <= (uint64_t)std::numeric_limits<uint32_t>::max()) << "KTX images total size overflow";

        const uint64_t numElements = (uint64_t)mArrayLength * mNumCubeFaces;
        std::fill_n(&mBlobs->sizes[flatten(this, {mipmap, 0, 0})], numElements, faceSize);

        const uint64_t paddingAdvances = (uint64_t)cubePadding * mNumCubeFaces * mArrayLength + mipPadding;
        FILAMENT_CHECK_POSTCONDITION(paddingAdvances <= (size_t)(b_end - scan_pdata)) << "KTX padding data exceeds buffer";
        scan_pdata += paddingAdvances;
    }

    mBlobs->blobs.resize((uint32_t)measuredTotalSize);
    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {
        pdata += sizeof(uint32_t);
        const uint32_t faceSize = mBlobs->sizes[flatten(this, {mipmap, 0, 0})];
        const uint32_t levelSize = faceSize * mNumCubeFaces * mArrayLength;
        memcpy(mBlobs->get(flatten(this, {mipmap, 0, 0})), pdata, levelSize);
        pdata += levelSize;
        pdata += cubePadding * mNumCubeFaces * mArrayLength;
        pdata += mipPadding;
    }
}

bool Ktx1Bundle::serialize(uint8_t* destination, uint32_t numBytes) const {
    uint32_t requiredLength = getSerializedLength();
    if (numBytes < requiredLength) {
        return false;
    }

    // Fill in the header with the magic identifier, format info, and dimensions.
    SerializationHeader header = {};
    memcpy(header.magic, MAGIC, sizeof(MAGIC));
    header.info = mInfo;
    header.numberOfMipmapLevels = mNumMipLevels;
    header.numberOfArrayElements = mArrayLength;
    header.numberOfFaces = mNumCubeFaces;

    // For simplicity, Ktx1Bundle does not allow non-zero array length, but to be conformant we
    // should set this field to zero for non-array textures.
    if (mArrayLength == 1) {
        header.numberOfArrayElements = 0;
    }

    // Compute space required for metadata, padding up to 4-byte alignment.
    for (const auto& iter : mMetadata->keyvals) {
        const uint32_t kvsize = iter.first.size() + 1 + iter.second.size();
        const uint32_t kvpadding = 3 - ((kvsize + 3) % 4);
        header.bytesOfKeyValueData += sizeof(uint32_t) + kvsize + kvpadding;
    }

    // Copy the header into the destination memory.
    memcpy(destination, &header, sizeof(header));
    uint8_t* pdata = destination + sizeof(SerializationHeader);

    // Write out the metadata. Note that keys are null-terminated strings: they are constructed from
    // C strings, and we obtain their contents with c_str(). Values are binary strings: they are
    // constructed from begin-end pairs, and we obtain their contents with data().
    for (const auto& iter : mMetadata->keyvals) {
        const uint32_t kvsize = iter.first.size() + 1 + iter.second.size();
        const uint32_t kvpadding = 3 - ((kvsize + 3) % 4);
        memcpy(pdata, &kvsize, sizeof(uint32_t));
        pdata += sizeof(uint32_t);
        memcpy(pdata, iter.first.c_str(), iter.first.size() + 1);
        pdata += iter.first.size() + 1;
        memcpy(pdata, iter.second.data(), iter.second.size());
        pdata += iter.second.size();
        pdata += kvpadding;
    }

    // One aspect of the KTX spec is that the semantics differ for non-array cubemaps.
    const bool isNonArrayCube = mNumCubeFaces > 1 && mArrayLength == 1;
    const uint32_t facesPerMip = mArrayLength * mNumCubeFaces;

    // Extract blobs from the serialized byte stream.
    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {

        // Every blob in a given miplevel has the same size, and each miplevel has at least one
        // blob. Therefore we can safely determine each of the so-called "imageSize" fields in KTX
        // by simply looking at the first blob in the LOD.
        uint32_t faceSize;
        uint8_t* blobData;
        if (!getBlob({mipmap, 0, 0}, &blobData, &faceSize)) {
            return false;
        }
        uint32_t imageSize = isNonArrayCube ? faceSize : (faceSize * facesPerMip);
        *((uint32_t*) pdata) = imageSize;
        pdata += sizeof(imageSize);

        // Copy out all layer and face blobs for this mipmap at once, since they are contiguous
        const uint32_t levelSize = faceSize * facesPerMip;
        memcpy(pdata, blobData, levelSize);
        pdata += levelSize;
    }
    return true;
}

uint32_t Ktx1Bundle::getSerializedLength() const {
    uint64_t total = sizeof(SerializationHeader);
    for (const auto& iter : mMetadata->keyvals) {
        const uint32_t kvsize = iter.first.size() + 1 + iter.second.size();
        const uint32_t kvpadding = 3 - ((kvsize + 3) % 4);
        total += sizeof(uint32_t) + kvsize + kvpadding;
    }
    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {
        total += sizeof(uint32_t);
        const size_t startIndex = flatten(this, {mipmap, 0, 0});
        const size_t numElements = (size_t)mArrayLength * mNumCubeFaces;
        const uint32_t blobSize = mBlobs->sizes[startIndex];
        
        for (size_t i = 0; i < numElements; ++i) {
            FILAMENT_CHECK_POSTCONDITION(mBlobs->sizes[startIndex + i] == blobSize)
                    << "Inconsistent blob sizes within LOD";
        }
        total += (uint64_t)blobSize * numElements;
    }
    FILAMENT_CHECK_POSTCONDITION(total <= (uint64_t)std::numeric_limits<uint32_t>::max())
            << "KTX serialization size overflow";
    return (uint32_t)total;
}

const char* Ktx1Bundle::getMetadata(const char* key, size_t* valueSize) const {
    auto iter = mMetadata->keyvals.find(key);
    if (iter == mMetadata->keyvals.end()) {
        return nullptr;
    }
    if (valueSize) {
        *valueSize = iter->second.size();
    }
    // This returns data() rather than c_str() because values need not be null terminated.
    return iter->second.data();
}

void Ktx1Bundle::setMetadata(const char* key, const char* value) {
    mMetadata->keyvals.insert({key, value});
}

bool Ktx1Bundle::getSphericalHarmonics(filament::math::float3* result) {
    char const* src = getMetadata("sh");
    if (!src) {
        return false;
    }
    float* flat = &result->x;
    // 3 bands, 9 RGB coefficients for a total of 27 floats.
    for (int i = 0; i < 9 * 3; i++) {
        char* next;
        *flat++ = utils::strtof_c(src, &next);
        if (next == src) {
            return false;
        }
        src = next;
    }
    return true;
}

bool Ktx1Bundle::getBlob(KtxBlobIndex index, uint8_t** data, uint32_t* size) const {
    if (index.mipLevel >= mNumMipLevels || index.arrayIndex >= mArrayLength ||
            index.cubeFace >= mNumCubeFaces) {
        return false;
    }
    uint32_t flatIndex = flatten(this, index);
    auto blobSize = mBlobs->sizes[flatIndex];
    if (blobSize == 0) {
        return false;
    }
    *data = mBlobs->get(flatIndex);
    *size = blobSize;
    return true;
}

bool Ktx1Bundle::setBlob(KtxBlobIndex index, uint8_t const* data, uint32_t size) {
    if (index.mipLevel >= mNumMipLevels || index.arrayIndex >= mArrayLength ||
            index.cubeFace >= mNumCubeFaces) {
        return false;
    }
    uint32_t flatIndex = flatten(this, index);
    uint32_t blobSize = mBlobs->sizes[flatIndex];
    if (blobSize != size) {
        mBlobs->resize(flatIndex, size);
    }
    memcpy(mBlobs->get(flatIndex), data, size);
    return true;
}

bool Ktx1Bundle::allocateBlob(KtxBlobIndex index, uint32_t size) {
    if (index.mipLevel >= mNumMipLevels || index.arrayIndex >= mArrayLength ||
            index.cubeFace >= mNumCubeFaces) {
        return false;
    }
    uint32_t flatIndex = flatten(this, index);
    mBlobs->resize(flatIndex, size);
    return true;
}

}  // namespace image
