/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <gltfio/TextureProvider.h>

#include <string>
#include <vector>
#include <ostream>

#include <utils/JobSystem.h>
#include <utils/Log.h>

#include <filament/Engine.h>

#include "DdsProvider.h"

#include "FFilamentAsset.h"

using namespace filament;
using namespace utils;

using std::atomic;
using std::vector;
using std::unique_ptr;

namespace filament::gltfio {

class DdsProvider final : public TextureProvider {
public:
    DdsProvider(Engine* engine);
    ~DdsProvider();

    Texture* pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureFlags flags) final;

    Texture* popTexture() final;
    void updateQueue() final;
    void waitForCompletion() final;
    void cancelDecoding() final;
    const char* getPushMessage() const final;
    const char* getPopMessage() const final;
    size_t getPushedCount() const final { return mPushedCount; }
    size_t getPoppedCount() const final { return mPoppedCount; }
    size_t getDecodedCount() const final { return mDecodedCount; }

private:
    enum class TextureState {
        DECODING, // Texture has been pushed, mipmap levels are not yet complete.
        READY,    // Mipmap levels are available but texture has not been popped yet.
        POPPED,   // Client has popped the texture from the queue.
    };

    struct TextureInfo {
        Texture* texture;
        TextureState state;
        std::shared_ptr<gli::texture> gliTexture;
    };

    size_t mPushedCount = 0;
    size_t mPoppedCount = 0;
    size_t mDecodedCount = 0;
    vector<unique_ptr<TextureInfo> > mTextures;
    std::string mRecentPushMessage;
    std::string mRecentPopMessage;
    Engine* const mEngine;
};

Texture* DdsProvider::pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureFlags flags) {
    const gli::texture gliTexture = gli::load(reinterpret_cast<const char*>(data), byteCount); // 使用 gli 库加载纹理

    if (gliTexture.empty()) {
        mRecentPushMessage = std::string("Unable to parse texture with gli.");
        return nullptr;
    }

    const auto dimensions = gliTexture.extent();
    if (GLTFIO_VERBOSE) {
        slog.d << "dimensions : x=" << dimensions.x << ", y=" << dimensions.y << ", z=" << dimensions.z << io::endl;
    }

    const auto levels = gliTexture.levels();
    const auto layers = gliTexture.layers();
    const auto faces = gliTexture.faces();
    const auto size = gliTexture.size();
    if (GLTFIO_VERBOSE) {
        slog.d << "levels: " << levels << ", layers: " << layers << ", faces: " << faces << ", size: " << size << io::endl;
    }

    const auto gliTarget = gliTexture.target();
    const auto sampler = convertSampler(gliTarget);
    if (GLTFIO_VERBOSE) {
        slog.d << "gliTarget: " << gliTargetString(gliTarget) << " => sampler: " << samplerTypeString(sampler) << io::endl;
    }

    const auto gliSwizzles = gliTexture.swizzles();
    const auto swizzles = convertSwizzles(gliSwizzles);
    if (GLTFIO_VERBOSE) {
        slog.d << "gliSwizzles : r=" << gliSwizzleString(gliSwizzles.r) << ", g=" << gliSwizzleString(gliSwizzles.g)
                           << ", b=" << gliSwizzleString(gliSwizzles.b) << ", a=" << gliSwizzleString(gliSwizzles.a) <<
                  " => " <<
                  "swizzles : r=" << textureSwizzleString(swizzles.r) << ", g=" << textureSwizzleString(swizzles.g)
                        << ", b=" << textureSwizzleString(swizzles.b) << ", a=" <<textureSwizzleString(swizzles.a) << io::endl;
    }

    const auto gliFormat = gliTexture.format();
    const auto format = convertTextureFormat(gliFormat);
    if (GLTFIO_VERBOSE) {
        slog.d << "gliFormat: " << gliFormatString(gliFormat) << " => format: " << textureFormatString(format) << io::endl;
    }

    Texture* texture = Texture::Builder()
            .width(dimensions.x)
            .height(dimensions.y)
            .depth(dimensions.z)
            .levels(levels)
            .sampler(sampler)
            .swizzle(swizzles.r, swizzles.g, swizzles.b, swizzles.a)
            .format(format)
            .build(*mEngine);

    if (texture == nullptr) {
        mRecentPushMessage = "Unable to build Texture object.";
        return nullptr;
    }

    mRecentPushMessage.clear();
    TextureInfo* info = mTextures.emplace_back(new TextureInfo).get();
    ++mPushedCount;

    info->texture = texture;
    info->state = TextureState::DECODING;
    info->gliTexture = std::make_shared<gli::texture>(gliTexture);
    return texture;
}

Texture* DdsProvider::popTexture() {
    // We don't bother shrinking the mTextures vector here, instead we periodically clean it up in
    // the updateQueue method, since popTexture is typically called more frequently. Textures
    // can become ready in non-deterministic order due to concurrency.
    for (auto& texture : mTextures) {
        if (texture->state == TextureState::READY) {
            texture->state = TextureState::POPPED;
            ++mPoppedCount;
            mRecentPopMessage.clear();
            return texture->texture;
        }
    }
    return nullptr;
}

void DdsProvider::updateQueue() {
    for (auto& info : mTextures) {
        if (info->state != TextureState::DECODING) {
            continue;
        }
        const Texture* texture = info->texture;

        const auto gliTexture = info->gliTexture;
        const auto levels = gliTexture->levels();
        // const auto layers = gliTexture->layers();
        // const auto faces = gliTexture->faces();
        // const auto size = gliTexture->size();
        // slog.d << "levels: " << levels << ", layers: " << layers << ", faces: " << faces << ", size: " << size << io::endl;

        const auto gliFormat = gliTexture->format();
        const auto isCompressed = gli::is_compressed(gliFormat);
        const auto pixelDataFormat = convertPixelDataFormat(gliFormat);
        const auto pixelDataType = convertPixelDataType(gliFormat);
        const auto compressedPixelDataType = convertCompressedPixelDataType(gliFormat);
        if (GLTFIO_VERBOSE) {
            slog.d << "gliFormat: " << gliFormatString(gliFormat)
                   << ", isCompressed: " << (isCompressed ? "true" : "false")
                   << ", pixelDataFormat: " << pixelDataFormatString(pixelDataFormat)
                   << ", pixelDataType: " << pixelDataTypeString(pixelDataType)
                   << ", compressedPixelDataType: " << compressedPixelDataTypeString(compressedPixelDataType)
                   << io::endl;
        }

        // 上传纹理数据
        for (std::size_t level = 0; level < levels; ++level) {
            // 获取当前层级、层的数据指针
            const size_t levelSize = gliTexture->size(level);
            // slog.d << "     levels[" << level << "]: levelSize=" << levelSize << io::endl;
            const void* buffer = gliTexture->data(0, 0, level);

            if (isCompressed) {
                texture->setImage(*mEngine, level, filament::Texture::PixelBufferDescriptor(
                    buffer,
                    levelSize,
                    compressedPixelDataType,
                    levelSize,
                    [](void* buffer, size_t size, void* user) {
                        // 将 void* 转换回 shared_ptr 并释放
                        auto* sptr = static_cast<std::shared_ptr<gli::texture>*>(user);
                        delete sptr; // 当 lambda 调用完成后，shared_ptr 的引用计数会减少，可能会导致对象被删除
                    },
                    new std::shared_ptr<gli::texture>(gliTexture)
                ));
            } else {
                texture->setImage(*mEngine, level, filament::Texture::PixelBufferDescriptor(
                    buffer,
                    levelSize,
                    pixelDataFormat,
                    pixelDataType,
                    [](void* buffer, size_t size, void* user) {
                        // 将 void* 转换回 shared_ptr 并释放
                        auto* sptr = static_cast<std::shared_ptr<gli::texture>*>(user);
                        delete sptr; // 当 lambda 调用完成后，shared_ptr 的引用计数会减少，可能会导致对象被删除
                    },
                    new std::shared_ptr<gli::texture>(gliTexture)
                ));
            }
        }
        info->state = TextureState::READY;
        ++mDecodedCount;
    }

    // Here we periodically clean up the "queue" (which is really just a vector) by removing unused
    // items from the front. This might ignore a popped texture that occurs in the middle of the
    // vector, but that's okay, it will be cleaned up eventually.
    decltype(mTextures)::iterator last = mTextures.begin();
    while (last != mTextures.end() && (*last)->state == TextureState::POPPED) ++last;
    mTextures.erase(mTextures.begin(), last);
}

void DdsProvider::waitForCompletion() {
}

void DdsProvider::cancelDecoding() {
}

const char* DdsProvider::getPushMessage() const {
    return mRecentPushMessage.empty() ? nullptr : mRecentPushMessage.c_str();
}

const char* DdsProvider::getPopMessage() const {
    return mRecentPopMessage.empty() ? nullptr : mRecentPopMessage.c_str();
}

DdsProvider::DdsProvider(Engine* engine) : mEngine(engine) {
#ifndef NDEBUG
    slog.i << "Dds Texture Decoder has "
            << mEngine->getJobSystem().getThreadCount()
            << " background threads." << io::endl;
#endif
}

DdsProvider::~DdsProvider() {
    cancelDecoding();
}

TextureProvider* createDdsProvider(Engine* engine) {
    return new DdsProvider(engine);
}

backend::SamplerType convertSampler(const gli::target gliTarget) {
    switch (gliTarget) {
    case gli::TARGET_1D:
        // Filament没有明确的一维纹理采样器类型。
            throw std::invalid_argument("Filament does not support 1D textures.");
    case gli::TARGET_1D_ARRAY:
        // Filament没有明确的一维数组纹理采样器类型。
            throw std::invalid_argument("Filament does not support 1D array textures.");
    case gli::TARGET_2D:
        return backend::SamplerType::SAMPLER_2D;
    case gli::TARGET_2D_ARRAY:
        return backend::SamplerType::SAMPLER_2D_ARRAY;
    case gli::TARGET_3D:
        return backend::SamplerType::SAMPLER_3D;
    case gli::TARGET_RECT:
        // Filament没有明确的矩形纹理采样器类型。
            throw std::invalid_argument("Filament does not support rectangle textures.");
    case gli::TARGET_RECT_ARRAY:
        // Filament没有明确的矩形数组纹理采样器类型。
            throw std::invalid_argument("Filament does not support rectangle array textures.");
    case gli::TARGET_CUBE:
        return backend::SamplerType::SAMPLER_CUBEMAP;
    case gli::TARGET_CUBE_ARRAY:
        return backend::SamplerType::SAMPLER_CUBEMAP_ARRAY;
    default:
        throw std::invalid_argument("Unsupported GLI target type for Filament: " + gliTargetString(gliTarget));
    }
}

inline glm::vec<SIZE_SWIZZLE, backend::TextureSwizzle> convertSwizzles(const gli::swizzles& gliSwizzles) {
    // 固定顺序：r g b a
    glm::vec<SIZE_SWIZZLE, backend::TextureSwizzle> swizzles{backend::TextureSwizzle::SUBSTITUTE_ZERO};
    for (int i = 0; i < SIZE_SWIZZLE; ++i) {
        switch (gliSwizzles[i]) {
        case gli::SWIZZLE_RED:
            swizzles[0] = SWIZZLE(i);
            // swizzles[i] = backend::TextureSwizzle::CHANNEL_0;
            break;
        case gli::SWIZZLE_GREEN:
            swizzles[1] = SWIZZLE(i);
            // swizzles[i] = backend::TextureSwizzle::CHANNEL_1;
            break;
        case gli::SWIZZLE_BLUE:
            swizzles[2] = SWIZZLE(i);
            // swizzles[i] = backend::TextureSwizzle::CHANNEL_2;
            break;
        case gli::SWIZZLE_ALPHA:
            swizzles[3] = SWIZZLE(i);
            // swizzles[i] = backend::TextureSwizzle::CHANNEL_3;
            break;
        case gli::SWIZZLE_ZERO:
            swizzles[i] = backend::TextureSwizzle::SUBSTITUTE_ZERO;
            break;
        case gli::SWIZZLE_ONE:
            swizzles[i] = backend::TextureSwizzle::SUBSTITUTE_ONE;
            break;
        default:
            throw std::invalid_argument("Invalid gli::swizzle value: " + gliSwizzleString(gliSwizzles[i]));
        }
    }

    return swizzles;
}

backend::PixelDataFormat convertPixelDataFormat(const gli::format gliFormat) {
    using namespace gli;
    using namespace filament::backend;

    if (FORMAT_A8_UNORM_PACK8 == gliFormat || FORMAT_A16_UNORM_PACK16 == gliFormat) {
        return PixelDataFormat::ALPHA;
    }

    if (is_compressed(gliFormat)) {
        return PixelDataFormat::UNUSED;
    }

    if (is_depth_stencil(gliFormat)) {
        return PixelDataFormat::DEPTH_STENCIL;
    }
    if (is_depth(gliFormat)) {
        return PixelDataFormat::DEPTH_COMPONENT;
    }
    if (is_stencil(gliFormat)) {
        return PixelDataFormat::UNUSED;
    }

    const size_t components = component_count(gliFormat);
    switch (components) {
    case 1:
        {
            if (is_float(gliFormat)) {
                return PixelDataFormat::R;
            }
            if (is_integer(gliFormat)) {
                return PixelDataFormat::R_INTEGER;
            }
            return PixelDataFormat::UNUSED;
        }
    case 2:
        {
            if (is_float(gliFormat)) {
                return PixelDataFormat::RG;
            }
            if (is_integer(gliFormat)) {
                return PixelDataFormat::RG_INTEGER;
            }
            return PixelDataFormat::UNUSED;
        }
    case 3:
        {
            if (is_float(gliFormat)) {
                return PixelDataFormat::RGB;
            }
            if (is_integer(gliFormat)) {
                return PixelDataFormat::RGB_INTEGER;
            }
            return PixelDataFormat::UNUSED;
        }
    case 4:
        {
            if (is_float(gliFormat)) {
                return PixelDataFormat::RGBA;
            }
            if (is_integer(gliFormat)) {
                return PixelDataFormat::RGBA_INTEGER;
            }
            return PixelDataFormat::UNUSED;
        }
    default:
        throw std::invalid_argument("Invalid gli::format value for PixelDataFormat: " + gliFormatString(gliFormat));
    }
}

backend::PixelDataType convertPixelDataType(const gli::format gliFormat) {
    using namespace gli;
    using namespace filament::backend;

    if (FORMAT_RG11B10_UFLOAT_PACK32 == gliFormat) {
        return PixelDataType::UINT_10F_11F_11F_REV;
    }

    if (FORMAT_R5G6B5_UNORM_PACK16 == gliFormat || FORMAT_B5G6R5_UNORM_PACK16 == gliFormat) {
        return PixelDataType::USHORT_565;
    }

    if (FORMAT_RGB10A2_UINT_PACK32 == gliFormat || FORMAT_BGR10A2_UINT_PACK32 == gliFormat) {
        return PixelDataType::UINT_2_10_10_10_REV;
    }

    if (is_compressed(gliFormat)) {
        return PixelDataType::COMPRESSED;
    }

    const size_t componentCount = component_count(gliFormat);
    const size_t blockSize = block_size(gliFormat);

    const bool isSigned = is_signed(gliFormat);
    const bool isInteger = is_integer(gliFormat);
    const bool isFloat = is_float(gliFormat);

    if (isFloat) {
        if (blockSize == 2 * componentCount) {
            return PixelDataType::HALF;
        }
        if (blockSize == 4 * componentCount) {
            return PixelDataType::FLOAT;
        }
    }

    if (isInteger) {
        if (isSigned) {
            if (blockSize == 1 * componentCount) {
                return PixelDataType::BYTE;
            }
            if (blockSize == 2 * componentCount) {
                return PixelDataType::SHORT;
            }
            if (blockSize == 4 * componentCount) {
                return PixelDataType::INT;
            }
        } else {
            if (blockSize == 1 * componentCount) {
                return PixelDataType::UBYTE;
            }
            if (blockSize == 2 * componentCount) {
                return PixelDataType::USHORT;
            }
            if (blockSize == 4 * componentCount) {
                return PixelDataType::UINT;
            }
        }
    }
    throw std::invalid_argument("Invalid gli::format value for PixelDataType: " + gliFormatString(gliFormat));
}

backend::CompressedPixelDataType convertCompressedPixelDataType(const gli::format gliFormat) {
    using namespace gli;
    using namespace filament::backend;
    switch (gliFormat) {
    // Mandatory in GLES 3.0 and GL 4.3
    case FORMAT_R_EAC_UNORM_BLOCK8:
        return CompressedPixelDataType::EAC_R11;
    case FORMAT_R_EAC_SNORM_BLOCK8:
        return CompressedPixelDataType::EAC_R11_SIGNED;
    case FORMAT_RG_EAC_UNORM_BLOCK16:
        return CompressedPixelDataType::EAC_RG11;
    case FORMAT_RG_EAC_SNORM_BLOCK16:
        return CompressedPixelDataType::EAC_RG11_SIGNED;
    case FORMAT_RGB_ETC2_UNORM_BLOCK8:
        return CompressedPixelDataType::ETC2_RGB8;
    case FORMAT_RGB_ETC2_SRGB_BLOCK8:
        return CompressedPixelDataType::ETC2_SRGB8;
    case FORMAT_RGBA_ETC2_UNORM_BLOCK8:
        return CompressedPixelDataType::ETC2_RGB8_A1;
    case FORMAT_RGBA_ETC2_SRGB_BLOCK8:
        return CompressedPixelDataType::ETC2_SRGB8_A1;
    case FORMAT_RGBA_ETC2_UNORM_BLOCK16:
        return CompressedPixelDataType::ETC2_EAC_RGBA8;
    case FORMAT_RGBA_ETC2_SRGB_BLOCK16:
        return CompressedPixelDataType::ETC2_EAC_SRGBA8;

    // Available everywhere except Android/iOS
    case FORMAT_RGB_DXT1_UNORM_BLOCK8:
        return CompressedPixelDataType::DXT1_RGB;
    case FORMAT_RGBA_DXT1_UNORM_BLOCK8:
        return CompressedPixelDataType::DXT1_RGBA;
    case FORMAT_RGBA_DXT3_UNORM_BLOCK16:
        return CompressedPixelDataType::DXT3_RGBA;
    case FORMAT_RGBA_DXT5_UNORM_BLOCK16:
        return CompressedPixelDataType::DXT5_RGBA;
    case FORMAT_RGB_DXT1_SRGB_BLOCK8:
        return CompressedPixelDataType::DXT1_SRGB;
    case FORMAT_RGBA_DXT1_SRGB_BLOCK8:
        return CompressedPixelDataType::DXT1_SRGBA;
    case FORMAT_RGBA_DXT3_SRGB_BLOCK16:
        return CompressedPixelDataType::DXT3_SRGBA;
    case FORMAT_RGBA_DXT5_SRGB_BLOCK16:
        return CompressedPixelDataType::DXT5_SRGBA;

    // ASTC formats are available with a GLES extension
    case FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_4x4;
    case FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_5x4;
    case FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_5x5;
    case FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_6x5;
    case FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_6x6;
    case FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_8x5;
    case FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_8x6;
    case FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_8x8;
    case FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_10x5;
    case FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_10x6;
    case FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_10x8;
    case FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_10x10;
    case FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_12x10;
    case FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_ASTC_12x12;
    case FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_4x4;
    case FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_5x4;
    case FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_5x5;
    case FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_6x5;
    case FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_6x6;
    case FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x5;
    case FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x6;
    case FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x8;
    case FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x5;
    case FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x6;
    case FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x8;
    case FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x10;
    case FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_12x10;
    case FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB8_ALPHA8_ASTC_12x12;

    // RGTC formats available with a GLES extension
    case FORMAT_R_ATI1N_UNORM_BLOCK8:
        return CompressedPixelDataType::RED_RGTC1;                  // BC4 unsigned
    case FORMAT_R_ATI1N_SNORM_BLOCK8:
        return CompressedPixelDataType::SIGNED_RED_RGTC1;           // BC4 signed
    case FORMAT_RG_ATI2N_UNORM_BLOCK16:
        return CompressedPixelDataType::RED_GREEN_RGTC2;            // BC5 unsigned
    case FORMAT_RG_ATI2N_SNORM_BLOCK16:
        return CompressedPixelDataType::SIGNED_RED_GREEN_RGTC2;     // BC5 signed

    // BPTC formats available with a GLES extension
    case FORMAT_RGB_BP_SFLOAT_BLOCK16:
        return CompressedPixelDataType::RGB_BPTC_SIGNED_FLOAT;      // BC6H signed
    case FORMAT_RGB_BP_UFLOAT_BLOCK16:
        return CompressedPixelDataType::RGB_BPTC_UNSIGNED_FLOAT;    // BC6H unsigned
    case FORMAT_RGBA_BP_UNORM_BLOCK16:
        return CompressedPixelDataType::RGBA_BPTC_UNORM;            // BC7
    case FORMAT_RGBA_BP_SRGB_BLOCK16:
        return CompressedPixelDataType::SRGB_ALPHA_BPTC_UNORM;      // BC7 sRGB
    default:
        throw std::invalid_argument("Invalid gli::format value for CompressedPixelDataType: " + gliFormatString(gliFormat));
    }
}

backend::TextureFormat convertTextureFormat(const gli::format gliFormat) {
    using namespace gli;
    using namespace filament::backend;
    switch (gliFormat) {
    // 8-bits per element
    case FORMAT_R8_UNORM_PACK8:
        return TextureFormat::R8;
    case FORMAT_R8_SNORM_PACK8:
        return TextureFormat::R8_SNORM;
    case FORMAT_R8_UINT_PACK8:
        return TextureFormat::R8UI;
    case FORMAT_R8_SINT_PACK8:
        return TextureFormat::R8I;
    // case :
    //     return TextureFormat::STENCIL8;

    // 16-bits per element
    case FORMAT_R16_SFLOAT_PACK16:
        return TextureFormat::R16F;
    case FORMAT_R16_UINT_PACK16:
        return TextureFormat::R16UI;
    case FORMAT_R16_SINT_PACK16:
        return TextureFormat::R16I;
    case FORMAT_RG8_UNORM_PACK8:
        return TextureFormat::RG8;
    case FORMAT_RG8_SNORM_PACK8:
        return TextureFormat::RG8_SNORM;
    case FORMAT_RG8_UINT_PACK8:
        return TextureFormat::RG8UI;
    case FORMAT_RG8_SINT_PACK8:
        return TextureFormat::RG8I;
    case FORMAT_R5G6B5_UNORM_PACK16:
        return TextureFormat::RGB565;
    case FORMAT_RGB9E5_UFLOAT_PACK32:
        return TextureFormat::RGB9_E5; // 9995 is actually 32 bpp but it's here for historical reasons.
    case FORMAT_RGB5A1_UNORM_PACK16:
        return TextureFormat::RGB5_A1;
    case FORMAT_RGBA4_UNORM_PACK16:
        return TextureFormat::RGBA4;
    // case :
    //     return TextureFormat::DEPTH16;

    // 24-bits per element
    case FORMAT_RGB8_UNORM_PACK8:
        return TextureFormat::RGB8;
    case FORMAT_RGB8_SRGB_PACK8:
        return TextureFormat::SRGB8;
    case FORMAT_RGB8_SNORM_PACK8:
        return TextureFormat::RGB8_SNORM;
    case FORMAT_RGB8_UINT_PACK8:
        return TextureFormat::RGB8UI;
    case FORMAT_RGB8_SINT_PACK8:
        return TextureFormat::RGB8I;
    // case :
    //     return TextureFormat::DEPTH24;

    // 32-bits per element
    case FORMAT_R32_SFLOAT_PACK32:
        return TextureFormat::R32F;
    case FORMAT_R32_UINT_PACK32:
        return TextureFormat::R32UI;
    case FORMAT_R32_SINT_PACK32:
        return TextureFormat::R32I;
    case FORMAT_RG16_SFLOAT_PACK16:
        return TextureFormat::RG16F;
    case FORMAT_RG16_UINT_PACK16:
        return TextureFormat::RG16UI;
    case FORMAT_RG16_SINT_PACK16:
        return TextureFormat::RG16I;
    case FORMAT_RG11B10_UFLOAT_PACK32:
        return TextureFormat::R11F_G11F_B10F;
    case FORMAT_RGBA8_UNORM_PACK32:
        return TextureFormat::RGBA8;
    case FORMAT_RGBA8_SRGB_PACK32:
        return TextureFormat::SRGB8_A8;
    case FORMAT_RGBA8_SNORM_PACK32:
        return TextureFormat::RGBA8_SNORM;
    // case :
    //     return TextureFormat::UNUSED; // used to be rgbm
    case FORMAT_RGB10A2_UNORM_PACK32:
        return TextureFormat::RGB10_A2;
    case FORMAT_RGBA8_UINT_PACK32:
        return TextureFormat::RGBA8UI;
    case FORMAT_RGBA8_SINT_PACK32:
        return TextureFormat::RGBA8I;
    // case :
    //     return TextureFormat::DEPTH32F;
    // case :
    //     return TextureFormat::DEPTH24_STENCIL8;
    // case :
    //     return TextureFormat::DEPTH32F_STENCIL8;

    // 48-bits per element
    case FORMAT_RGB16_SFLOAT_PACK16:
        return TextureFormat::RGB16F;
    case FORMAT_RGB16_UINT_PACK16:
        return TextureFormat::RGB16UI;
    case FORMAT_RGB16_SINT_PACK16:
        return TextureFormat::RGB16I;

    // 64-bits per element
    case FORMAT_RG32_SFLOAT_PACK32:
        return TextureFormat::RG32F;
    case FORMAT_RG32_UINT_PACK32:
        return TextureFormat::RG32UI;
    case FORMAT_RG32_SINT_PACK32:
        return TextureFormat::RG32I;
    case FORMAT_RGBA16_SFLOAT_PACK16:
        return TextureFormat::RGBA16F;
    case FORMAT_RGBA16_UINT_PACK16:
        return TextureFormat::RGBA16UI;
    case FORMAT_RGBA16_SINT_PACK16:
        return TextureFormat::RGBA16I;

    // 96-bits per element
    case FORMAT_RGB32_SFLOAT_PACK32:
        return TextureFormat::RGB32F;
    case FORMAT_RGB32_UINT_PACK32:
        return TextureFormat::RGB32UI;
    case FORMAT_RGB32_SINT_PACK32:
        return TextureFormat::RGB32I;

    // 128-bits per element
    case FORMAT_RGBA32_SFLOAT_PACK32:
        return TextureFormat::RGBA32F;
    case FORMAT_RGBA32_UINT_PACK32:
        return TextureFormat::RGBA32UI;
    case FORMAT_RGBA32_SINT_PACK32:
        return TextureFormat::RGBA32I;

    // compressed formats

    // Mandatory in GLES 3.0 and GL 4.3
    case FORMAT_R_EAC_UNORM_BLOCK8:
        return TextureFormat::EAC_R11;
    case FORMAT_R_EAC_SNORM_BLOCK8:
        return TextureFormat::EAC_R11_SIGNED;
    case FORMAT_RG_EAC_UNORM_BLOCK16:
        return TextureFormat::EAC_RG11;
    case FORMAT_RG_EAC_SNORM_BLOCK16:
        return TextureFormat::EAC_RG11_SIGNED;
    case FORMAT_RGB_ETC2_UNORM_BLOCK8:
        return TextureFormat::ETC2_RGB8;
    case FORMAT_RGB_ETC2_SRGB_BLOCK8:
        return TextureFormat::ETC2_SRGB8;
    case FORMAT_RGBA_ETC2_UNORM_BLOCK8:
        return TextureFormat::ETC2_RGB8_A1;
    case FORMAT_RGBA_ETC2_SRGB_BLOCK8:
        return TextureFormat::ETC2_SRGB8_A1;
    case FORMAT_RGBA_ETC2_UNORM_BLOCK16:
        return TextureFormat::ETC2_EAC_RGBA8;
    case FORMAT_RGBA_ETC2_SRGB_BLOCK16:
        return TextureFormat::ETC2_EAC_SRGBA8;

    // Available everywhere except Android/iOS
    case FORMAT_RGB_DXT1_UNORM_BLOCK8:
        return TextureFormat::DXT1_RGB;
    case FORMAT_RGBA_DXT1_UNORM_BLOCK8:
        return TextureFormat::DXT1_RGBA;
    case FORMAT_RGBA_DXT3_UNORM_BLOCK16:
        return TextureFormat::DXT3_RGBA;
    case FORMAT_RGBA_DXT5_UNORM_BLOCK16:
        return TextureFormat::DXT5_RGBA;
    case FORMAT_RGB_DXT1_SRGB_BLOCK8:
        return TextureFormat::DXT1_SRGB;
    case FORMAT_RGBA_DXT1_SRGB_BLOCK8:
        return TextureFormat::DXT1_SRGBA;
    case FORMAT_RGBA_DXT3_SRGB_BLOCK16:
        return TextureFormat::DXT3_SRGBA;
    case FORMAT_RGBA_DXT5_SRGB_BLOCK16:
        return TextureFormat::DXT5_SRGBA;

    // ASTC formats are available with a GLES extension
    case FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_4x4;
    case FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_5x4;
    case FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_5x5;
    case FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_6x5;
    case FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_6x6;
    case FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_8x5;
    case FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_8x6;
    case FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_8x8;
    case FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_10x5;
    case FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_10x6;
    case FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_10x8;
    case FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_10x10;
    case FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_12x10;
    case FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
        return TextureFormat::RGBA_ASTC_12x12;
    case FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_4x4;
    case FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_5x4;
    case FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_5x5;
    case FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_6x5;
    case FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_6x6;
    case FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_8x5;
    case FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_8x6;
    case FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_8x8;
    case FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_10x5;
    case FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_10x6;
    case FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_10x8;
    case FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_10x10;
    case FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_12x10;
    case FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
        return TextureFormat::SRGB8_ALPHA8_ASTC_12x12;

    // RGTC formats available with a GLES extension
    case FORMAT_R_ATI1N_UNORM_BLOCK8:
        return TextureFormat::RED_RGTC1;                  // BC4 unsigned
    case FORMAT_R_ATI1N_SNORM_BLOCK8:
        return TextureFormat::SIGNED_RED_RGTC1;           // BC4 signed
    case FORMAT_RG_ATI2N_UNORM_BLOCK16:
        return TextureFormat::RED_GREEN_RGTC2;            // BC5 unsigned
    case FORMAT_RG_ATI2N_SNORM_BLOCK16:
        return TextureFormat::SIGNED_RED_GREEN_RGTC2;     // BC5 signed

    // BPTC formats available with a GLES extension
    case FORMAT_RGB_BP_SFLOAT_BLOCK16:
        return TextureFormat::RGB_BPTC_SIGNED_FLOAT;      // BC6H signed
    case FORMAT_RGB_BP_UFLOAT_BLOCK16:
        return TextureFormat::RGB_BPTC_UNSIGNED_FLOAT;    // BC6H unsigned
    case FORMAT_RGBA_BP_UNORM_BLOCK16:
        return TextureFormat::RGBA_BPTC_UNORM;            // BC7
    case FORMAT_RGBA_BP_SRGB_BLOCK16:
        return TextureFormat::SRGB_ALPHA_BPTC_UNORM;      // BC7 sRGB
    default:
        throw std::invalid_argument("Invalid gli::format value for TextureFormat: " + gliFormatString(gliFormat));
    }
}

std::string pixelDataFormatString(const backend::PixelDataFormat pixelDataFormat) {
    using namespace backend;
    switch (pixelDataFormat) {
#define STR(r)                                                                      \
    case PixelDataFormat::r:                                                        \
        return #r

    STR(R);
    STR(R_INTEGER);
    STR(RG);
    STR(RG_INTEGER);
    STR(RGB);
    STR(RGB_INTEGER);
    STR(RGBA);
    STR(RGBA_INTEGER);
    STR(UNUSED);
    STR(DEPTH_COMPONENT);
    STR(DEPTH_STENCIL);
    STR(ALPHA);
#undef STR
    default:
        return "Unknown_PixelDataFormat_" + std::to_string(static_cast<uint8_t>(pixelDataFormat));
    }
}

std::string pixelDataTypeString(const backend::PixelDataType pixelDataType) {
    using namespace backend;
    switch (pixelDataType) {
#define STR(r)                                                                      \
    case PixelDataType::r:                                                          \
        return #r

    STR(UBYTE);
    STR(BYTE);
    STR(USHORT);
    STR(SHORT);
    STR(UINT);
    STR(INT);
    STR(HALF);
    STR(FLOAT);
    STR(COMPRESSED);
    STR(UINT_10F_11F_11F_REV);
    STR(USHORT_565);
    STR(UINT_2_10_10_10_REV);
#undef STR
    default:
        return "Unknown_PixelDataType_" + std::to_string(static_cast<uint8_t>(pixelDataType));
    }
}

std::string compressedPixelDataTypeString(const backend::CompressedPixelDataType compressedPixelDataType) {
    using namespace backend;
    switch (compressedPixelDataType) {
#define STR(r)                                                                      \
    case CompressedPixelDataType::r:                                                          \
        return #r
    // Mandatory in GLES 3.0 and GL 4.3
    STR(EAC_R11);
    STR(EAC_R11_SIGNED);
    STR(EAC_RG11);
    STR(EAC_RG11_SIGNED);
    STR(ETC2_RGB8);
    STR(ETC2_SRGB8);
    STR(ETC2_RGB8_A1);
    STR(ETC2_SRGB8_A1);
    STR(ETC2_EAC_RGBA8);
    STR(ETC2_EAC_SRGBA8);

    // Available everywhere except Android/iOS
    STR(DXT1_RGB);
    STR(DXT1_RGBA);
    STR(DXT3_RGBA);
    STR(DXT5_RGBA);
    STR(DXT1_SRGB);
    STR(DXT1_SRGBA);
    STR(DXT3_SRGBA);
    STR(DXT5_SRGBA);

    // ASTC formats are available with a GLES extension
    STR(RGBA_ASTC_4x4);
    STR(RGBA_ASTC_5x4);
    STR(RGBA_ASTC_5x5);
    STR(RGBA_ASTC_6x5);
    STR(RGBA_ASTC_6x6);
    STR(RGBA_ASTC_8x5);
    STR(RGBA_ASTC_8x6);
    STR(RGBA_ASTC_8x8);
    STR(RGBA_ASTC_10x5);
    STR(RGBA_ASTC_10x6);
    STR(RGBA_ASTC_10x8);
    STR(RGBA_ASTC_10x10);
    STR(RGBA_ASTC_12x10);
    STR(RGBA_ASTC_12x12);
    STR(SRGB8_ALPHA8_ASTC_4x4);
    STR(SRGB8_ALPHA8_ASTC_5x4);
    STR(SRGB8_ALPHA8_ASTC_5x5);
    STR(SRGB8_ALPHA8_ASTC_6x5);
    STR(SRGB8_ALPHA8_ASTC_6x6);
    STR(SRGB8_ALPHA8_ASTC_8x5);
    STR(SRGB8_ALPHA8_ASTC_8x6);
    STR(SRGB8_ALPHA8_ASTC_8x8);
    STR(SRGB8_ALPHA8_ASTC_10x5);
    STR(SRGB8_ALPHA8_ASTC_10x6);
    STR(SRGB8_ALPHA8_ASTC_10x8);
    STR(SRGB8_ALPHA8_ASTC_10x10);
    STR(SRGB8_ALPHA8_ASTC_12x10);
    STR(SRGB8_ALPHA8_ASTC_12x12);

    // RGTC formats available with a GLES extension
    STR(RED_RGTC1);
    STR(SIGNED_RED_RGTC1);
    STR(RED_GREEN_RGTC2);
    STR(SIGNED_RED_GREEN_RGTC2);

    // BPTC formats available with a GLES extension
    STR(RGB_BPTC_SIGNED_FLOAT);
    STR(RGB_BPTC_UNSIGNED_FLOAT);
    STR(RGBA_BPTC_UNORM);
    STR(SRGB_ALPHA_BPTC_UNORM);
#undef STR
    default:
        return "Unknown_CompressedPixelDataType_" + std::to_string(static_cast<uint16_t>(compressedPixelDataType));
    }
}

std::string textureFormatString(const backend::TextureFormat textureFormat) {
    using namespace backend;
    switch (textureFormat) {
#define STR(r)                                                                      \
case TextureFormat::r:                                                              \
return #r
        // 8-bits per element
        STR(R8);
        STR(R8_SNORM);
        STR(R8UI);
        STR(R8I);
        STR(STENCIL8);

        // 16-bits per element
        STR(R16F);
        STR(R16UI);
        STR(R16I);
        STR(RG8);
        STR(RG8_SNORM);
        STR(RG8UI);
        STR(RG8I);
        STR(RGB565);
        STR(RGB9_E5); // 9995 is actually 32 bpp but it's here for historical reasons.
        STR(RGB5_A1);
        STR(RGBA4);
        STR(DEPTH16);

        // 24-bits per element
        STR(RGB8);
        STR(SRGB8);
        STR(RGB8_SNORM);
        STR(RGB8UI);
        STR(RGB8I);
        STR(DEPTH24);

        // 32-bits per element
        STR(R32F);
        STR(R32UI);
        STR(R32I);
        STR(RG16F);
        STR(RG16UI);
        STR(RG16I);
        STR(R11F_G11F_B10F);
        STR(RGBA8);
        STR(SRGB8_A8);
        STR(RGBA8_SNORM);
        STR(UNUSED);// used to be rgbm
        STR(RGB10_A2);
        STR(RGBA8UI);
        STR(RGBA8I);
        STR(DEPTH32F);
        STR(DEPTH24_STENCIL8);
        STR(DEPTH32F_STENCIL8);

        // 48-bits per element
        STR(RGB16F);
        STR(RGB16UI);
        STR(RGB16I);

        // 64-bits per element
        STR(RG32F);
        STR(RG32UI);
        STR(RG32I);
        STR(RGBA16F);
        STR(RGBA16UI);
        STR(RGBA16I);

        // 96-bits per element
        STR(RGB32F);
        STR(RGB32UI);
        STR(RGB32I);

        // 128-bits per element
        STR(RGBA32F);
        STR(RGBA32UI);
        STR(RGBA32I);

        // compressed formats

        // Mandatory in GLES 3.0 and GL 4.3
        STR(EAC_R11);
        STR(EAC_R11_SIGNED);
        STR(EAC_RG11);
        STR(EAC_RG11_SIGNED);
        STR(ETC2_RGB8);
        STR(ETC2_SRGB8);
        STR(ETC2_RGB8_A1);
        STR(ETC2_SRGB8_A1);
        STR(ETC2_EAC_RGBA8);
        STR(ETC2_EAC_SRGBA8);

        // Available everywhere except Android/iOS
        STR(DXT1_RGB);
        STR(DXT1_RGBA);
        STR(DXT3_RGBA);
        STR(DXT5_RGBA);
        STR(DXT1_SRGB);
        STR(DXT1_SRGBA);
        STR(DXT3_SRGBA);
        STR(DXT5_SRGBA);

        // ASTC formats are available with a GLES extension
        STR(RGBA_ASTC_4x4);
        STR(RGBA_ASTC_5x4);
        STR(RGBA_ASTC_5x5);
        STR(RGBA_ASTC_6x5);
        STR(RGBA_ASTC_6x6);
        STR(RGBA_ASTC_8x5);
        STR(RGBA_ASTC_8x6);
        STR(RGBA_ASTC_8x8);
        STR(RGBA_ASTC_10x5);
        STR(RGBA_ASTC_10x6);
        STR(RGBA_ASTC_10x8);
        STR(RGBA_ASTC_10x10);
        STR(RGBA_ASTC_12x10);
        STR(RGBA_ASTC_12x12);
        STR(SRGB8_ALPHA8_ASTC_4x4);
        STR(SRGB8_ALPHA8_ASTC_5x4);
        STR(SRGB8_ALPHA8_ASTC_5x5);
        STR(SRGB8_ALPHA8_ASTC_6x5);
        STR(SRGB8_ALPHA8_ASTC_6x6);
        STR(SRGB8_ALPHA8_ASTC_8x5);
        STR(SRGB8_ALPHA8_ASTC_8x6);
        STR(SRGB8_ALPHA8_ASTC_8x8);
        STR(SRGB8_ALPHA8_ASTC_10x5);
        STR(SRGB8_ALPHA8_ASTC_10x6);
        STR(SRGB8_ALPHA8_ASTC_10x8);
        STR(SRGB8_ALPHA8_ASTC_10x10);
        STR(SRGB8_ALPHA8_ASTC_12x10);
        STR(SRGB8_ALPHA8_ASTC_12x12);

        // RGTC formats available with a GLES extension
        STR(RED_RGTC1);
        STR(SIGNED_RED_RGTC1);
        STR(RED_GREEN_RGTC2);
        STR(SIGNED_RED_GREEN_RGTC2);

        // BPTC formats available with a GLES extension
        STR(RGB_BPTC_SIGNED_FLOAT);
        STR(RGB_BPTC_UNSIGNED_FLOAT);
        STR(RGBA_BPTC_UNORM);
        STR(SRGB_ALPHA_BPTC_UNORM);
#undef STR
    default:
        return "Unknown_TextureFormat_" + std::to_string(static_cast<uint16_t>(textureFormat));
    }
}

std::string samplerTypeString(const backend::SamplerType samplerType) {
    using namespace backend;
    switch (samplerType) {
#define STR(r)                                                                      \
    case SamplerType::r:                                                            \
        return #r

    STR(SAMPLER_2D);
    STR(SAMPLER_2D_ARRAY);
    STR(SAMPLER_CUBEMAP);
    STR(SAMPLER_EXTERNAL);
    STR(SAMPLER_3D);
    STR(SAMPLER_CUBEMAP_ARRAY);
#undef STR
    default:
        return "Unknown_SamplerType_" + std::to_string(static_cast<uint8_t>(samplerType));
    }
}

std::string textureSwizzleString(const backend::TextureSwizzle textureSwizzle) {
    using namespace backend;
    switch (textureSwizzle) {
#define STR(r)                                                                      \
    case TextureSwizzle::r:                                                         \
        return #r

    STR(SUBSTITUTE_ZERO);
    STR(SUBSTITUTE_ONE);
    STR(CHANNEL_0);
    STR(CHANNEL_1);
    STR(CHANNEL_2);
    STR(CHANNEL_3);
#undef STR
    default:
        return "Unknown_TextureSwizzle_" + std::to_string(static_cast<uint8_t>(textureSwizzle));
    }
}

std::string gliFormatString(const gli::format gliFormat) {
    using namespace gli;
    switch (gliFormat) {
#define STR(r)                                                                      \
    case r:                                                                \
        return #r

    STR(FORMAT_UNDEFINED);

    STR(FORMAT_RG4_UNORM_PACK8);
    STR(FORMAT_RGBA4_UNORM_PACK16);
    STR(FORMAT_BGRA4_UNORM_PACK16);
    STR(FORMAT_R5G6B5_UNORM_PACK16);
    STR(FORMAT_B5G6R5_UNORM_PACK16);
    STR(FORMAT_RGB5A1_UNORM_PACK16);
    STR(FORMAT_BGR5A1_UNORM_PACK16);
    STR(FORMAT_A1RGB5_UNORM_PACK16);

    STR(FORMAT_R8_UNORM_PACK8);
    STR(FORMAT_R8_SNORM_PACK8);
    STR(FORMAT_R8_USCALED_PACK8);
    STR(FORMAT_R8_SSCALED_PACK8);
    STR(FORMAT_R8_UINT_PACK8);
    STR(FORMAT_R8_SINT_PACK8);
    STR(FORMAT_R8_SRGB_PACK8);

    STR(FORMAT_RG8_UNORM_PACK8);
    STR(FORMAT_RG8_SNORM_PACK8);
    STR(FORMAT_RG8_USCALED_PACK8);
    STR(FORMAT_RG8_SSCALED_PACK8);
    STR(FORMAT_RG8_UINT_PACK8);
    STR(FORMAT_RG8_SINT_PACK8);
    STR(FORMAT_RG8_SRGB_PACK8);

    STR(FORMAT_RGB8_UNORM_PACK8);
    STR(FORMAT_RGB8_SNORM_PACK8);
    STR(FORMAT_RGB8_USCALED_PACK8);
    STR(FORMAT_RGB8_SSCALED_PACK8);
    STR(FORMAT_RGB8_UINT_PACK8);
    STR(FORMAT_RGB8_SINT_PACK8);
    STR(FORMAT_RGB8_SRGB_PACK8);

    STR(FORMAT_BGR8_UNORM_PACK8);
    STR(FORMAT_BGR8_SNORM_PACK8);
    STR(FORMAT_BGR8_USCALED_PACK8);
    STR(FORMAT_BGR8_SSCALED_PACK8);
    STR(FORMAT_BGR8_UINT_PACK8);
    STR(FORMAT_BGR8_SINT_PACK8);
    STR(FORMAT_BGR8_SRGB_PACK8);

    STR(FORMAT_RGBA8_UNORM_PACK8);
    STR(FORMAT_RGBA8_SNORM_PACK8);
    STR(FORMAT_RGBA8_USCALED_PACK8);
    STR(FORMAT_RGBA8_SSCALED_PACK8);
    STR(FORMAT_RGBA8_UINT_PACK8);
    STR(FORMAT_RGBA8_SINT_PACK8);
    STR(FORMAT_RGBA8_SRGB_PACK8);

    STR(FORMAT_BGRA8_UNORM_PACK8);
    STR(FORMAT_BGRA8_SNORM_PACK8);
    STR(FORMAT_BGRA8_USCALED_PACK8);
    STR(FORMAT_BGRA8_SSCALED_PACK8);
    STR(FORMAT_BGRA8_UINT_PACK8);
    STR(FORMAT_BGRA8_SINT_PACK8);
    STR(FORMAT_BGRA8_SRGB_PACK8);

    STR(FORMAT_RGBA8_UNORM_PACK32);
    STR(FORMAT_RGBA8_SNORM_PACK32);
    STR(FORMAT_RGBA8_USCALED_PACK32);
    STR(FORMAT_RGBA8_SSCALED_PACK32);
    STR(FORMAT_RGBA8_UINT_PACK32);
    STR(FORMAT_RGBA8_SINT_PACK32);
    STR(FORMAT_RGBA8_SRGB_PACK32);

    STR(FORMAT_RGB10A2_UNORM_PACK32);
    STR(FORMAT_RGB10A2_SNORM_PACK32);
    STR(FORMAT_RGB10A2_USCALED_PACK32);
    STR(FORMAT_RGB10A2_SSCALED_PACK32);
    STR(FORMAT_RGB10A2_UINT_PACK32);
    STR(FORMAT_RGB10A2_SINT_PACK32);

    STR(FORMAT_BGR10A2_UNORM_PACK32);
    STR(FORMAT_BGR10A2_SNORM_PACK32);
    STR(FORMAT_BGR10A2_USCALED_PACK32);
    STR(FORMAT_BGR10A2_SSCALED_PACK32);
    STR(FORMAT_BGR10A2_UINT_PACK32);
    STR(FORMAT_BGR10A2_SINT_PACK32);

    STR(FORMAT_R16_UNORM_PACK16);
    STR(FORMAT_R16_SNORM_PACK16);
    STR(FORMAT_R16_USCALED_PACK16);
    STR(FORMAT_R16_SSCALED_PACK16);
    STR(FORMAT_R16_UINT_PACK16);
    STR(FORMAT_R16_SINT_PACK16);
    STR(FORMAT_R16_SFLOAT_PACK16);

    STR(FORMAT_RG16_UNORM_PACK16);
    STR(FORMAT_RG16_SNORM_PACK16);
    STR(FORMAT_RG16_USCALED_PACK16);
    STR(FORMAT_RG16_SSCALED_PACK16);
    STR(FORMAT_RG16_UINT_PACK16);
    STR(FORMAT_RG16_SINT_PACK16);
    STR(FORMAT_RG16_SFLOAT_PACK16);

    STR(FORMAT_RGB16_UNORM_PACK16);
    STR(FORMAT_RGB16_SNORM_PACK16);
    STR(FORMAT_RGB16_USCALED_PACK16);
    STR(FORMAT_RGB16_SSCALED_PACK16);
    STR(FORMAT_RGB16_UINT_PACK16);
    STR(FORMAT_RGB16_SINT_PACK16);
    STR(FORMAT_RGB16_SFLOAT_PACK16);

    STR(FORMAT_RGBA16_UNORM_PACK16);
    STR(FORMAT_RGBA16_SNORM_PACK16);
    STR(FORMAT_RGBA16_USCALED_PACK16);
    STR(FORMAT_RGBA16_SSCALED_PACK16);
    STR(FORMAT_RGBA16_UINT_PACK16);
    STR(FORMAT_RGBA16_SINT_PACK16);
    STR(FORMAT_RGBA16_SFLOAT_PACK16);

    STR(FORMAT_R32_UINT_PACK32);
    STR(FORMAT_R32_SINT_PACK32);
    STR(FORMAT_R32_SFLOAT_PACK32);

    STR(FORMAT_RG32_UINT_PACK32);
    STR(FORMAT_RG32_SINT_PACK32);
    STR(FORMAT_RG32_SFLOAT_PACK32);

    STR(FORMAT_RGB32_UINT_PACK32);
    STR(FORMAT_RGB32_SINT_PACK32);
    STR(FORMAT_RGB32_SFLOAT_PACK32);

    STR(FORMAT_RGBA32_UINT_PACK32);
    STR(FORMAT_RGBA32_SINT_PACK32);
    STR(FORMAT_RGBA32_SFLOAT_PACK32);

    STR(FORMAT_R64_UINT_PACK64);
    STR(FORMAT_R64_SINT_PACK64);
    STR(FORMAT_R64_SFLOAT_PACK64);

    STR(FORMAT_RG64_UINT_PACK64);
    STR(FORMAT_RG64_SINT_PACK64);
    STR(FORMAT_RG64_SFLOAT_PACK64);

    STR(FORMAT_RGB64_UINT_PACK64);
    STR(FORMAT_RGB64_SINT_PACK64);
    STR(FORMAT_RGB64_SFLOAT_PACK64);

    STR(FORMAT_RGBA64_UINT_PACK64);
    STR(FORMAT_RGBA64_SINT_PACK64);
    STR(FORMAT_RGBA64_SFLOAT_PACK64);

    STR(FORMAT_RG11B10_UFLOAT_PACK32);
    STR(FORMAT_RGB9E5_UFLOAT_PACK32);

    STR(FORMAT_D16_UNORM_PACK16);
    STR(FORMAT_D24_UNORM_PACK32);
    STR(FORMAT_D32_SFLOAT_PACK32);
    STR(FORMAT_S8_UINT_PACK8);
    STR(FORMAT_D16_UNORM_S8_UINT_PACK32);
    STR(FORMAT_D24_UNORM_S8_UINT_PACK32);
    STR(FORMAT_D32_SFLOAT_S8_UINT_PACK64);

    STR(FORMAT_RGB_DXT1_UNORM_BLOCK8);
    STR(FORMAT_RGB_DXT1_SRGB_BLOCK8);
    STR(FORMAT_RGBA_DXT1_UNORM_BLOCK8);
    STR(FORMAT_RGBA_DXT1_SRGB_BLOCK8);
    STR(FORMAT_RGBA_DXT3_UNORM_BLOCK16);
    STR(FORMAT_RGBA_DXT3_SRGB_BLOCK16);
    STR(FORMAT_RGBA_DXT5_UNORM_BLOCK16);
    STR(FORMAT_RGBA_DXT5_SRGB_BLOCK16);
    STR(FORMAT_R_ATI1N_UNORM_BLOCK8);
    STR(FORMAT_R_ATI1N_SNORM_BLOCK8);
    STR(FORMAT_RG_ATI2N_UNORM_BLOCK16);
    STR(FORMAT_RG_ATI2N_SNORM_BLOCK16);
    STR(FORMAT_RGB_BP_UFLOAT_BLOCK16);
    STR(FORMAT_RGB_BP_SFLOAT_BLOCK16);
    STR(FORMAT_RGBA_BP_UNORM_BLOCK16);
    STR(FORMAT_RGBA_BP_SRGB_BLOCK16);

    STR(FORMAT_RGB_ETC2_UNORM_BLOCK8);
    STR(FORMAT_RGB_ETC2_SRGB_BLOCK8);
    STR(FORMAT_RGBA_ETC2_UNORM_BLOCK8);
    STR(FORMAT_RGBA_ETC2_SRGB_BLOCK8);
    STR(FORMAT_RGBA_ETC2_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ETC2_SRGB_BLOCK16);
    STR(FORMAT_R_EAC_UNORM_BLOCK8);
    STR(FORMAT_R_EAC_SNORM_BLOCK8);
    STR(FORMAT_RG_EAC_UNORM_BLOCK16);
    STR(FORMAT_RG_EAC_SNORM_BLOCK16);

    STR(FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16);
    STR(FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16);

    STR(FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32);
    STR(FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32);
    STR(FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32);
    STR(FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32);
    STR(FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32);
    STR(FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32);
    STR(FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32);
    STR(FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32);
    STR(FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8);
    STR(FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8);
    STR(FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8);
    STR(FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8);

    STR(FORMAT_RGB_ETC_UNORM_BLOCK8);
    STR(FORMAT_RGB_ATC_UNORM_BLOCK8);
    STR(FORMAT_RGBA_ATCA_UNORM_BLOCK16);
    STR(FORMAT_RGBA_ATCI_UNORM_BLOCK16);

    STR(FORMAT_L8_UNORM_PACK8);
    STR(FORMAT_A8_UNORM_PACK8);
    STR(FORMAT_LA8_UNORM_PACK8);
    STR(FORMAT_L16_UNORM_PACK16);
    STR(FORMAT_A16_UNORM_PACK16);
    STR(FORMAT_LA16_UNORM_PACK16);

    STR(FORMAT_BGR8_UNORM_PACK32);
    STR(FORMAT_BGR8_SRGB_PACK32);

    STR(FORMAT_RG3B2_UNORM_PACK8);
#undef STR
    default:
        return "UNKNOWN_gli::format_" + std::to_string(gliFormat);
    }
}

std::string gliSwizzleString(const gli::swizzle gliSwizzle) {
    using namespace gli;
    switch (gliSwizzle) {
#define STR(r)                                                                      \
    case r:                                                                \
        return #r

    STR(SWIZZLE_RED);
    STR(SWIZZLE_GREEN);
    STR(SWIZZLE_BLUE);
    STR(SWIZZLE_ALPHA);
    STR(SWIZZLE_ZERO);
    STR(SWIZZLE_ONE);
#undef STR
    default:
        return "Unknown_gli::swizzle_" + std::to_string(gliSwizzle);
    }
}

std::string gliTargetString(const gli::target gliTarget) {
    using namespace gli;
    switch (gliTarget) {
#define STR(r)                                                                      \
    case r:                                                                 \
        return #r

    STR(TARGET_1D);
    STR(TARGET_1D_ARRAY);
    STR(TARGET_2D);
    STR(TARGET_2D_ARRAY);
    STR(TARGET_3D);
    STR(TARGET_RECT);
    STR(TARGET_RECT_ARRAY);
    STR(TARGET_CUBE);
    STR(TARGET_CUBE_ARRAY);
#undef STR
    default:
        return "Unknown_gli::target_" + std::to_string(gliTarget);
    }
}

} // namespace filament::gltfio
