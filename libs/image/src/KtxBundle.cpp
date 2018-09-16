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

#include <image/KtxBundle.h>

#include <utils/Panic.h>

#include <vector>

namespace {

using Blob = std::vector<uint8_t>;

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
inline size_t flatten(const image::KtxBundle* bundle, image::KtxBlobIndex index) {
    const uint32_t nfaces = bundle->isCubemap() ? 6 : 1;
    const uint32_t nlayers = bundle->getArrayLength();
    return index.cubeFace + index.arrayIndex * nfaces + index.mipLevel * nfaces * nlayers;
}

const uint8_t MAGIC[] = {0xab, 0x4b, 0x54, 0x58, 0x20, 0x31, 0x31, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a};

}

namespace image  {

// This little wrapper exists so that we can keep STL out of the interface.
struct KtxBlobList {
    std::vector<Blob> blobs;
};

KtxBundle::~KtxBundle() = default;

KtxBundle::KtxBundle(uint32_t numMipLevels, uint32_t arrayLength, bool isCubemap) :
        mBlobs(new KtxBlobList) {
    mNumMipLevels = numMipLevels;
    mArrayLength = arrayLength;
    mNumCubeFaces = isCubemap ? 6 : 1;
    mBlobs->blobs.resize(numMipLevels * arrayLength * mNumCubeFaces);
}

KtxBundle::KtxBundle(uint8_t const* bytes, uint32_t nbytes) : mBlobs(new KtxBlobList) {
    ASSERT_PRECONDITION(sizeof(SerializationHeader) <= nbytes, "KTX buffer is too small");

    // First, "parse" the header by casting it to a struct.
    SerializationHeader const* header = (SerializationHeader const*) bytes;
    ASSERT_PRECONDITION(memcmp(header->magic, MAGIC, 12) == 0, "KTX has unexpected identifier");
    mInfo = header->info;

    // The spec allows 0 or 1 for the number of array layers and mipmap levels, but we replace 0
    // with 1 for simplicity. Technically this is a loss of information because 0 mipmaps means
    // "please generate the mips" and an array size of 1 means "make this an array texture, but
    // with only one element". For now, ignoring this distinction seems fine.
    mNumMipLevels = header->numberOfMipmapLevels ? header->numberOfMipmapLevels : 1;
    mArrayLength = header->numberOfArrayElements ? header->numberOfArrayElements : 1;
    mNumCubeFaces = header->numberOfFaces ? header->numberOfFaces : 1;
    mBlobs->blobs.resize(mNumMipLevels * mArrayLength * mNumCubeFaces);

    // For now, we discard the key-value metadata. Note that this may be useful for storing
    // spherical harmonics coefficients.
    uint8_t const* pdata = bytes + sizeof(SerializationHeader);
    uint8_t const* end = pdata + header->bytesOfKeyValueData;
    while (pdata < end) {
        const uint32_t keyAndValueByteSize = *((uint32_t const*) pdata);
        pdata += sizeof(keyAndValueByteSize);
        // ...this is a good spot for stashing the keyAndValue block...
        pdata += keyAndValueByteSize;
        const uint32_t paddingSize = 3 - ((keyAndValueByteSize + 3) % 4);
        pdata += paddingSize;
    }

    // There is no compressed format that has a block size that is not a multiple of 4, so these
    // two padding constants can be safely hardcoded to 0. They are here for spec consistency.
    const uint32_t cubePadding = 0;
    const uint32_t mipPadding = 0;

    // One aspect of the KTX spec is that the semantics differ for non-array cubemaps.
    const bool isNonArrayCube = mNumCubeFaces > 1 && mArrayLength == 1;
    const uint32_t facesPerMip = mArrayLength * mNumCubeFaces;

    // Extract blobs from the serialized byte stream.
    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {
        const uint32_t imageSize = *((uint32_t const*) pdata);
        const uint32_t faceSize = isNonArrayCube ? imageSize : (imageSize / facesPerMip);
        pdata += sizeof(imageSize);
        for (uint32_t layer = 0; layer < mArrayLength; ++layer) {
            for (uint32_t face = 0; face < mNumCubeFaces; ++face) {
                setBlob({mipmap, layer, face}, pdata, faceSize);
                pdata += faceSize;
                pdata += cubePadding;
            }
        }
        pdata += mipPadding;
    }
}

bool KtxBundle::serialize(uint8_t* destination, uint32_t numBytes) const {
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

    // For simplicity, KtxBundle does not allow non-zero array length, but to be conformant we
    // should set this field to zero for non-array textures.
    if (mArrayLength == 1) {
        header.numberOfArrayElements =  0;
    }

    // Copy the header into the destination memory.
    memcpy(destination, &header, sizeof(header));
    uint8_t* pdata = destination + sizeof(SerializationHeader);

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
        getBlob({mipmap, 0, 0}, &blobData, &faceSize);
        uint32_t imageSize = isNonArrayCube ? faceSize : (faceSize * facesPerMip);
        *((uint32_t*) pdata) = imageSize;
        pdata += sizeof(imageSize);

        // Next, copy out the actual blobs.
        for (uint32_t layer = 0; layer < mArrayLength; ++layer) {
            for (uint32_t face = 0; face < mNumCubeFaces; ++face) {
                if (!getBlob({mipmap, layer, face}, &blobData, &faceSize)) {
                    return false;
                }
                memcpy(pdata, blobData, faceSize);
                pdata += faceSize;
            }
        }
    }
    return true;
}

uint32_t KtxBundle::getSerializedLength() const {
    uint32_t total = sizeof(SerializationHeader);
    for (uint32_t mipmap = 0; mipmap < mNumMipLevels; ++mipmap) {
        total += sizeof(uint32_t);
        size_t blobSize = 0;
        for (uint32_t layer = 0; layer < mArrayLength; ++layer) {
            for (uint32_t face = 0; face < mNumCubeFaces; ++face) {
                auto& blob = mBlobs->blobs[flatten(this, {mipmap, layer, face})];
                if (blobSize == 0) {
                    blobSize = blob.size();
                }
                ASSERT_PRECONDITION(blobSize == blob.size(), "Inconsistent blob sizes within LOD");
                total += blobSize;
            }
        }
    }
    return total;
}

bool KtxBundle::getBlob(KtxBlobIndex index, uint8_t** data, uint32_t* size) const {
    if (index.mipLevel >= mNumMipLevels || index.arrayIndex >= mArrayLength ||
            index.cubeFace >= mNumCubeFaces) {
        return false;
    }
    auto& blob = mBlobs->blobs[flatten(this, index)];
    if (blob.empty()) {
        return false;
    }
    *data = blob.data();
    *size = blob.size();
    return true;
}

bool KtxBundle::setBlob(KtxBlobIndex index, uint8_t const* data, uint32_t size) {
    if (index.mipLevel >= mNumMipLevels || index.arrayIndex >= mArrayLength ||
            index.cubeFace >= mNumCubeFaces) {
        return false;
    }
    auto& blob = mBlobs->blobs[flatten(this, index)];
    blob.resize(size);
    memcpy(blob.data(), data, size);
    return true;
}

}  // namespace image
