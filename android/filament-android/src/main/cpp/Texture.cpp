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

#include <jni.h>

#include <algorithm>
#include <functional>

#ifdef ANDROID
#include <android/bitmap.h>
#endif

#include <backend/BufferDescriptor.h>
#include <filament/Engine.h>
#include <filament/Stream.h>
#include <filament/Texture.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace backend;

static size_t getTextureDataSize(const Texture *texture, size_t level,
        Texture::Format format, Texture::Type type, size_t stride, size_t alignment) {
    // Zero stride implies tight row-to-row packing.
    stride = stride == 0 ? texture->getWidth(level) : std::max(size_t(1), stride >> level);
    return Texture::computeTextureDataSize(format, type,
            stride, texture->getHeight(level), alignment);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsTextureFormatSupported(JNIEnv*, jclass,
        jlong nativeEngine, jint internalFormat) {
    Engine *engine = (Engine *) nativeEngine;
    return (jboolean) Texture::isTextureFormatSupported(*engine,
            (Texture::InternalFormat) internalFormat);
}

// Texture::Builder...

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new Texture::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderWidth(JNIEnv*, jclass,
        jlong nativeBuilder, jint width) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->width((uint32_t) width);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderHeight(JNIEnv*, jclass,
        jlong nativeBuilder, jint height) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->height((uint32_t) height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderDepth(JNIEnv*, jclass,
        jlong nativeBuilder, jint depth) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->depth((uint32_t) depth);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderLevels(JNIEnv*, jclass,
        jlong nativeBuilder, jint levels) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->levels((uint8_t) levels);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderSampler(JNIEnv*, jclass,
        jlong nativeBuilder, jint sampler) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->sampler((Texture::Sampler) sampler);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderFormat(JNIEnv*, jclass,
        jlong nativeBuilder, jint format) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->format((Texture::InternalFormat) format);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderUsage(JNIEnv*, jclass,
        jlong nativeBuilder, jint flags) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->usage((Texture::Usage) flags);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderSwizzle(JNIEnv *, jclass ,
        jlong nativeBuilder, jint r, jint g, jint b, jint a) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->swizzle(
            (Texture::Swizzle)r, (Texture::Swizzle)g, (Texture::Swizzle)b, (Texture::Swizzle)a);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

// Texture...

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetWidth(JNIEnv*, jclass, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getWidth((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetHeight(JNIEnv*, jclass, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getHeight((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetDepth(JNIEnv*, jclass, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getDepth((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetLevels(JNIEnv*, jclass, jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getLevels();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetTarget(JNIEnv*, jclass, jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getTarget();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetInternalFormat(JNIEnv*, jclass,
        jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getFormat();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImage(JNIEnv* env, jclass, jlong nativeTexture,
        jlong nativeEngine, jint level, jint xoffset, jint yoffset, jint width, jint height,
        jobject storage,  jint remaining,
        jint left, jint bottom, jint type, jint alignment,
        jint stride, jint format,
        jobject handler, jobject runnable) {
    Texture* texture = (Texture*) nativeTexture;
    Engine* engine = (Engine*) nativeEngine;

    size_t sizeInBytes = getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
            (Texture::Type) type, (size_t) stride, (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
            (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) bottom,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, (uint32_t) xoffset, (uint32_t) yoffset,
            (uint32_t) width, (uint32_t) height, std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCompressed(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject storage,  jint remaining,
        jint, jint, jint, jint, jint compressedSizeInBytes, jint compressedFormat,
        jobject handler, jobject runnable) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    size_t sizeInBytes = (size_t) compressedSizeInBytes;

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes,
            (backend::CompressedPixelDataType) compressedFormat, (uint32_t) compressedSizeInBytes,
            &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, (uint32_t) xoffset, (uint32_t) yoffset,
            (uint32_t) width, (uint32_t) height, std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemap(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint bottom, jint type, jint alignment, jint stride, jint format,
        jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, nullptr);
    Texture::FaceOffsets faceOffsets;
    std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
    env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

    size_t sizeInBytes = 6 * getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
            (Texture::Type) type, (size_t) stride, (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
            (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) bottom,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemapCompressed(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint bottom, jint type, jint alignment,
        jint compressedSizeInBytes, jint compressedFormat, jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {

    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, nullptr);
    Texture::FaceOffsets faceOffsets;
    std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
    env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

    size_t sizeInBytes = 6 * (size_t) compressedSizeInBytes;

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes,
            (backend::CompressedPixelDataType) compressedFormat, (uint32_t) compressedSizeInBytes,
            &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalImage(JNIEnv*, jclass, jlong nativeTexture,
        jlong nativeEngine, jlong eglImage) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    texture->setExternalImage(*engine, (void*)eglImage);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalStream(JNIEnv*, jclass,
        jlong nativeTexture, jlong nativeEngine, jlong nativeStream) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    Stream *stream = (Stream *) nativeStream;
    texture->setExternalStream(*engine, stream);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nGenerateMipmaps(JNIEnv*, jclass,
        jlong nativeTexture, jlong nativeEngine) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    texture->generateMipmaps(*engine);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsStreamValidForTexture(JNIEnv*, jclass,
        jlong nativeTexture, jlong) {
    Texture* texture = (Texture*) nativeTexture;
    return (jboolean) (texture->getTarget() == SamplerType::SAMPLER_EXTERNAL);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGeneratePrefilterMipmap(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint width, jint height,
        jobject storage, jint remaining, jint left,
        jint top, jint type, jint alignment, jint stride, jint format,
        jintArray faceOffsetsInBytes_, jobject handler, jobject runnable, jint sampleCount,
        jboolean mirror) {

    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, nullptr);
    Texture::FaceOffsets faceOffsets;
    std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
    env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

    stride = stride ? stride : width;
    size_t sizeInBytes = 6 *
            Texture::computeTextureDataSize((Texture::Format) format, (Texture::Type) type,
                                            (size_t) stride, (size_t) height, (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void* buffer = nioBuffer.getData();
    auto* callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
            (backend::PixelDataType) type, (uint8_t) alignment,
            (uint32_t) left, (uint32_t) top, (uint32_t) stride,
            &JniBufferCallback::invoke, callback);

    Texture::PrefilterOptions options;
    options.sampleCount = sampleCount;
    options.mirror = mirror;
    texture->generatePrefilterMipmap(*engine, std::move(desc), faceOffsets, &options);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ANDROID SPECIFIC BITS
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ANDROID

#define BITMAP_CONFIG_ALPHA_8   0
#define BITMAP_CONFIG_RGB_565   1
#define BITMAP_CONFIG_RGBA_4444 2
#define BITMAP_CONFIG_RGBA_8888 3
#define BITMAP_CONFIG_RGBA_F16  4
#define BITMAP_CONFIG_HARDWARE  5

class AutoBitmap {
public:
    AutoBitmap(JNIEnv* env, jobject bitmap) noexcept
            : mEnv(env)
            , mBitmap(env->NewGlobalRef(bitmap))
    {
        if (mBitmap) {
            AndroidBitmap_getInfo(mEnv, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(mEnv, mBitmap, &mData);
        }
    }

    ~AutoBitmap() noexcept {
        if (mBitmap) {
            AndroidBitmap_unlockPixels(mEnv, mBitmap);
            mEnv->DeleteGlobalRef(mBitmap);
        }
    }

    AutoBitmap(AutoBitmap &&rhs) noexcept {
        mEnv = rhs.mEnv;
        std::swap(mData, rhs.mData);
        std::swap(mBitmap, rhs.mBitmap);
        std::swap(mInfo, rhs.mInfo);
    }

    void* getData() const noexcept {
        return mData;
    }

    size_t getSizeInBytes() const noexcept {
        return mInfo.height * mInfo.stride;
    }

    PixelDataFormat getFormat(int format) const noexcept {
        // AndroidBitmapInfo does not capture the HARDWARE and RGBA_F16 formats
        // so we switch on the Bitmap.Config values directly
        switch (format) {
            case BITMAP_CONFIG_ALPHA_8: return PixelDataFormat::ALPHA;
            case BITMAP_CONFIG_RGB_565: return PixelDataFormat::RGB;
            default:                    return PixelDataFormat::RGBA;
        }
    }

    PixelDataType getType(int format) const noexcept {
        switch (format) {
            case BITMAP_CONFIG_RGB_565:  return PixelDataType::USHORT_565;
            case BITMAP_CONFIG_RGBA_F16: return PixelDataType::HALF;
            default:                     return PixelDataType::UBYTE;
        }
    }

    static void invoke(void* buffer, size_t n, void* user) {
        AutoBitmap* data = reinterpret_cast<AutoBitmap*>(user);
        delete data;
    }

    static AutoBitmap* make(Engine* engine, JNIEnv* env, jobject bitmap) {
        return new AutoBitmap(env, bitmap);
    }

private:
    JNIEnv* mEnv;
    void* mData = nullptr;
    jobject mBitmap = nullptr;
    AndroidBitmapInfo mInfo;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetBitmap(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject bitmap, jint format) {
    Texture* texture = (Texture*) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    auto* autoBitmap = AutoBitmap::make(engine, env, bitmap);

    Texture::PixelBufferDescriptor desc(
            autoBitmap->getData(),
            autoBitmap->getSizeInBytes(),
            autoBitmap->getFormat(format),
            autoBitmap->getType(format),
            &AutoBitmap::invoke, autoBitmap);

    texture->setImage(*engine, (size_t) level,
            (uint32_t) xoffset, (uint32_t) yoffset,
            (uint32_t) width, (uint32_t) height,
            std::move(desc));
}

#endif
