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

#ifndef IMAGE_KTX1BUNDLE_H
#define IMAGE_KTX1BUNDLE_H

#include <math/vec3.h>

#include <utils/compiler.h>

#include <cstdint>
#include <memory>

namespace image {

struct KtxInfo {
    uint32_t endianness;
    uint32_t glType;
    uint32_t glTypeSize;
    uint32_t glFormat;
    uint32_t glInternalFormat;
    uint32_t glBaseInternalFormat;
    uint32_t pixelWidth;
    uint32_t pixelHeight;
    uint32_t pixelDepth;
};

struct KtxBlobIndex {
    uint32_t mipLevel;
    uint32_t arrayIndex;
    uint32_t cubeFace;
};

struct KtxBlobList;
struct KtxMetadata;

/**
 * Ktx1Bundle is a structured set of opaque data blobs that can be passed straight to the GPU, such
 * that a single bundle corresponds to a single texture object. It is well suited for storing
 * block-compressed texture data.
 *
 * One bundle may be comprised of several mipmap levels, cubemap faces, and array elements. The
 * number of blobs is immutable, and is determined as follows.
 *
 *     blob_count = mip_count * array_length * (cubemap ? 6 : 1)
 *
 * Bundles can be quickly serialized to a certain file format (see below link), but this class lives
 * in the image lib rather than imageio because it has no dependencies, and does not support CPU
 * decoding.
 *
 *     https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
 */
class UTILS_PUBLIC Ktx1Bundle {
public:

    ~Ktx1Bundle();

    /**
     * Creates a hierarchy of empty texture blobs, to be filled later via setBlob().
     */
    Ktx1Bundle(uint32_t numMipLevels, uint32_t arrayLength, bool isCubemap);

    /**
     * Creates a new bundle by deserializing the given data.
     *
     * Typically, this constructor is used to consume the contents of a KTX file.
     */
    Ktx1Bundle(uint8_t const* bytes, uint32_t nbytes);

    /**
     * Serializes the bundle into the given target memory. Returns false if there's not enough
     * memory.
     *
     * Typically, this method is used to write out the contents of a KTX file.
     */
    bool serialize(uint8_t* destination, uint32_t numBytes) const;

    /**
     * Computes the size (in bytes) of the serialized bundle.
     */
    uint32_t getSerializedLength() const;

    /**
     * Gets or sets information about the texture object, such as format and type.
     */
    KtxInfo const& getInfo() const { return mInfo; }
    KtxInfo& info() { return mInfo; }

    /**
     * Gets or sets key/value metadata.
     */
    const char* getMetadata(const char* key, size_t* valueSize = nullptr) const;
    void setMetadata(const char* key, const char* value);

    /**
     * Parses the key="sh" metadata and returns 3 bands of data.
     *
     * Assumes 3 bands for a total of 9 RGB coefficients.
     * Returns true if successful.
     */
    bool getSphericalHarmonics(filament::math::float3* result);

    /**
     * Gets the number of miplevels (this is never zero).
     */
    uint32_t getNumMipLevels() const { return mNumMipLevels; }

    /**
     * Gets the number of array elements (this is never zero).
     */
    uint32_t getArrayLength() const { return mArrayLength; }

    /**
     * Returns whether or not this is a cubemap.
     */
    bool isCubemap() const { return mNumCubeFaces > 1; }

    /**
     * Retrieves a weak reference to a given data blob. Returns false if the given blob index is out
     * of bounds, or if the blob at the given index is empty.
     */
    bool getBlob(KtxBlobIndex index, uint8_t** data, uint32_t* size) const;

    /**
     * Copies the given data into the blob at the given index, replacing whatever is already there.
     * Returns false if the given blob index is out of bounds.
     */
    bool setBlob(KtxBlobIndex index, uint8_t const* data, uint32_t size);

    /**
     * Allocates the blob at the given index to the given number of bytes. This allows subsequent
     * calls to setBlob to be thread-safe.
     */
    bool allocateBlob(KtxBlobIndex index, uint32_t size);

    // The following constants help clients populate the "info" struct. Most of them have corollary
    // constants in the OpenGL headers.

    static constexpr uint32_t R8 = 0x8229;
    static constexpr uint32_t R8_SNORM = 0x8F94;
    static constexpr uint32_t R8UI = 0x8232;
    static constexpr uint32_t R8I = 0x8231;
    static constexpr uint32_t STENCIL_INDEX8 = 0x8D48;
    static constexpr uint32_t R16F = 0x822D;
    static constexpr uint32_t R16UI = 0x8234;
    static constexpr uint32_t R16I = 0x8233;
    static constexpr uint32_t RG8 = 0x822B;
    static constexpr uint32_t RG8_SNORM = 0x8F95;
    static constexpr uint32_t RG8UI = 0x8238;
    static constexpr uint32_t RG8I = 0x8237;
    static constexpr uint32_t RGB565 = 0x8D62;
    static constexpr uint32_t RGB5_A1 = 0x8057;
    static constexpr uint32_t RGBA4 = 0x8056;
    static constexpr uint32_t DEPTH_COMPONENT16 = 0x81A5;
    static constexpr uint32_t RGB8 = 0x8051;
    static constexpr uint32_t SRGB8 = 0x8C41;
    static constexpr uint32_t RGB8_SNORM = 0x8F96;
    static constexpr uint32_t RGB8UI = 0x8D7D;
    static constexpr uint32_t RGB8I = 0x8D8F;
    static constexpr uint32_t DEPTH_COMPONENT24 = 0x81A6;
    static constexpr uint32_t R32F = 0x822E;
    static constexpr uint32_t R32UI = 0x8236;
    static constexpr uint32_t R32I = 0x8235;
    static constexpr uint32_t RG16F = 0x822F;
    static constexpr uint32_t RG16UI = 0x823A;
    static constexpr uint32_t RG16I = 0x8239;
    static constexpr uint32_t R11F_G11F_B10F = 0x8C3A;
    static constexpr uint32_t RGB9_E5 = 0x8C3D;
    static constexpr uint32_t RGBA8 = 0x8058;
    static constexpr uint32_t SRGB8_ALPHA8 = 0x8C43;
    static constexpr uint32_t RGBA8_SNORM = 0x8F97;
    static constexpr uint32_t RGB10_A2 = 0x8059;
    static constexpr uint32_t RGBA8UI = 0x8D7C;
    static constexpr uint32_t RGBA8I = 0x8D8E;
    static constexpr uint32_t DEPTH_COMPONENT32F = 0x8CAC;
    static constexpr uint32_t DEPTH24_STENCIL8 = 0x88F0;
    static constexpr uint32_t DEPTH32F_STENCIL8 = 0x8CAD;
    static constexpr uint32_t RGB16F = 0x881B;
    static constexpr uint32_t RGB16UI = 0x8D77;
    static constexpr uint32_t RGB16I = 0x8D89;
    static constexpr uint32_t RG32F = 0x8230;
    static constexpr uint32_t RG32UI = 0x823C;
    static constexpr uint32_t RG32I = 0x823B;
    static constexpr uint32_t RGBA16F = 0x881A;
    static constexpr uint32_t RGBA16UI = 0x8D76;
    static constexpr uint32_t RGBA16I = 0x8D88;
    static constexpr uint32_t RGB32F = 0x8815;
    static constexpr uint32_t RGB32UI = 0x8D71;
    static constexpr uint32_t RGB32I = 0x8D83;
    static constexpr uint32_t RGBA32F = 0x8814;
    static constexpr uint32_t RGBA32UI = 0x8D70;
    static constexpr uint32_t RGBA32I = 0x8D82;

    static constexpr uint32_t RED = 0x1903;
    static constexpr uint32_t RG = 0x8227;
    static constexpr uint32_t RGB = 0x1907;
    static constexpr uint32_t RGBA = 0x1908;
    static constexpr uint32_t BGR = 0x80E0;
    static constexpr uint32_t BGRA = 0x80E1;
    static constexpr uint32_t LUMINANCE = 0x1909;
    static constexpr uint32_t LUMINANCE_ALPHA = 0x190A;

    static constexpr uint32_t UNSIGNED_BYTE = 0x1401;
    static constexpr uint32_t UNSIGNED_SHORT = 0x1403;
    static constexpr uint32_t HALF_FLOAT = 0x140B;
    static constexpr uint32_t FLOAT = 0x1406;

    static constexpr uint32_t ENDIAN_DEFAULT = 0x04030201;

    static constexpr uint32_t RGB_S3TC_DXT1 = 0x83F0;
    static constexpr uint32_t RGBA_S3TC_DXT1 = 0x83F1;
    static constexpr uint32_t RGBA_S3TC_DXT3 = 0x83F2;
    static constexpr uint32_t RGBA_S3TC_DXT5 = 0x83F3;

    static constexpr uint32_t R_RGTC_BC4_UNORM = 0x8DBB;
    static constexpr uint32_t R_RGTC_BC4_SNORM = 0x8DBC;
    static constexpr uint32_t RG_RGTC_BC5_UNORM = 0x8DBD;
    static constexpr uint32_t RG_RGTC_BC5_SNORM = 0x8DBE;

    static constexpr uint32_t RGBA_BPTC_BC7 = 0x8E8C;
    static constexpr uint32_t SRGB8_ALPHA8_BPTC_BC7 = 0x8E8D;
    static constexpr uint32_t RGB_BPTC_BC6H_SNORM = 0x8E8E;
    static constexpr uint32_t RGB_BPTC_BC6H_UNORM = 0x8E8F;

    static constexpr uint32_t RGBA_ASTC_4x4 = 0x93B0;
    static constexpr uint32_t RGBA_ASTC_5x4 = 0x93B1;
    static constexpr uint32_t RGBA_ASTC_5x5 = 0x93B2;
    static constexpr uint32_t RGBA_ASTC_6x5 = 0x93B3;
    static constexpr uint32_t RGBA_ASTC_6x6 = 0x93B4;
    static constexpr uint32_t RGBA_ASTC_8x5 = 0x93B5;
    static constexpr uint32_t RGBA_ASTC_8x6 = 0x93B6;
    static constexpr uint32_t RGBA_ASTC_8x8 = 0x93B7;
    static constexpr uint32_t RGBA_ASTC_10x5 = 0x93B8;
    static constexpr uint32_t RGBA_ASTC_10x6 = 0x93B9;
    static constexpr uint32_t RGBA_ASTC_10x8 = 0x93BA;
    static constexpr uint32_t RGBA_ASTC_10x10 = 0x93BB;
    static constexpr uint32_t RGBA_ASTC_12x10 = 0x93BC;
    static constexpr uint32_t RGBA_ASTC_12x12 = 0x93BD;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_4x4 = 0x93D0;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_5x4 = 0x93D1;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_5x5 = 0x93D2;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_6x5 = 0x93D3;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_6x6 = 0x93D4;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_8x5 = 0x93D5;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_8x6 = 0x93D6;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_8x8 = 0x93D7;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_10x5 = 0x93D8;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_10x6 = 0x93D9;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_10x8 = 0x93DA;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_10x10 = 0x93DB;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_12x10 = 0x93DC;
    static constexpr uint32_t SRGB8_ALPHA8_ASTC_12x12 = 0x93DD;

    static constexpr uint32_t R11_EAC = 0x9270;
    static constexpr uint32_t SIGNED_R11_EAC = 0x9271;
    static constexpr uint32_t RG11_EAC = 0x9272;
    static constexpr uint32_t SIGNED_RG11_EAC = 0x9273;
    static constexpr uint32_t RGB8_ETC2 = 0x9274;
    static constexpr uint32_t SRGB8_ETC2 = 0x9275;
    static constexpr uint32_t RGB8_ALPHA1_ETC2 = 0x9276;
    static constexpr uint32_t SRGB8_ALPHA1_ETC = 0x9277;
    static constexpr uint32_t RGBA8_ETC2_EAC = 0x9278;
    static constexpr uint32_t SRGB8_ALPHA8_ETC2_EAC = 0x9279;

private:
    image::KtxInfo mInfo = {};
    uint32_t mNumMipLevels;
    uint32_t mArrayLength;
    uint32_t mNumCubeFaces;
    std::unique_ptr<KtxBlobList> mBlobs;
    std::unique_ptr<KtxMetadata> mMetadata;
};

} // namespace image

#endif /* IMAGE_Ktx1Bundle_H */
