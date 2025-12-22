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

#ifdef __ANDROID__
#include <android/bitmap.h>
#endif

#include <filament/Engine.h>
#include <filament/Stream.h>
#include <filament/Texture.h>

#include <filament-generatePrefilterMipmap/generatePrefilterMipmap.h>

#include <backend/BufferDescriptor.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

#include "private/backend/VirtualMachineEnv.h"
#include "../../../../common/JniExceptionBridge.h"


using namespace filament;
using namespace backend;

static size_t getTextureDataSize(const Texture *texture,
        size_t level, Texture::Format format, Texture::Type type,
        size_t stride, size_t height, size_t alignment) {
    // Zero stride implies tight row-to-row packing.
    stride = stride == 0 ? texture->getWidth(level)  : std::max(size_t(1), stride >> level);
    height = height == 0 ? texture->getHeight(level) : std::max(size_t(1), height >> level);
    return Texture::computeTextureDataSize(format, type, stride, height, alignment);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsTextureFormatSupported(JNIEnv* env, jclass,
        jlong nativeEngine, jint internalFormat) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_Texture_nIsTextureFormatSupported", JNI_FALSE, [&]() -> jboolean {
            Engine *engine = (Engine *) nativeEngine;
            return (jboolean) Texture::isTextureFormatSupported(*engine,
                    (Texture::InternalFormat) internalFormat);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsTextureFormatMipmappable(JNIEnv* env, jclass,
        jlong nativeEngine, jint internalFormat) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_Texture_nIsTextureFormatMipmappable", JNI_FALSE, [&]() -> jboolean {
            Engine *engine = (Engine *) nativeEngine;
            return (jboolean) Texture::isTextureFormatMipmappable(*engine,
                    (Texture::InternalFormat) internalFormat);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsTextureSwizzleSupported(JNIEnv* env, jclass,
        jlong nativeEngine) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_Texture_nIsTextureSwizzleSupported", JNI_FALSE, [&]() -> jboolean {
            Engine *engine = (Engine *) nativeEngine;
            return (jboolean) Texture::isTextureSwizzleSupported(*engine);
    });
}


extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetMaxTextureSize(JNIEnv * env, jclass,
        jlong nativeEngine, jint sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetMaxTextureSize", 0, [&]() -> jint {
            Engine *engine = (Engine *) nativeEngine;
            return Texture::getMaxTextureSize(*engine, (Texture::Sampler)sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetMaxArrayTextureLayers(JNIEnv * env, jclass,
        jlong nativeEngine) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetMaxArrayTextureLayers", 0, [&]() -> jint {
            Engine *engine = (Engine *) nativeEngine;
            return Texture::getMaxArrayTextureLayers(*engine);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nValidatePixelFormatAndType(JNIEnv* env, jclass,
        jint internalFormat, jint pixelDataFormat, jint pixelDataType) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_Texture_nValidatePixelFormatAndType", JNI_FALSE, [&]() -> jboolean {
            return (jboolean) Texture::validatePixelFormatAndType(
                (Texture::InternalFormat) internalFormat,
                (Texture::Format) pixelDataFormat,
                (Texture::Type) pixelDataType
            );
    });
}

// Texture::Builder...

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nCreateBuilder(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_Texture_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new Texture::Builder();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nDestroyBuilder", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderWidth(JNIEnv* env, jclass,
        jlong nativeBuilder, jint width) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderWidth", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->width((uint32_t) width);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderHeight(JNIEnv* env, jclass,
        jlong nativeBuilder, jint height) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderHeight", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->height((uint32_t) height);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderDepth(JNIEnv* env, jclass,
        jlong nativeBuilder, jint depth) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderDepth", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->depth((uint32_t) depth);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderLevels(JNIEnv* env, jclass,
        jlong nativeBuilder, jint levels) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderLevels", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->levels((uint8_t) levels);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderSampler(JNIEnv* env, jclass,
        jlong nativeBuilder, jint sampler) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderSampler", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->sampler((Texture::Sampler) sampler);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderFormat(JNIEnv* env, jclass,
        jlong nativeBuilder, jint format) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderFormat", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->format((Texture::InternalFormat) format);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderUsage(JNIEnv* env, jclass,
        jlong nativeBuilder, jint flags) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderUsage", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->usage((Texture::Usage) flags);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderSwizzle(JNIEnv * env, jclass ,
        jlong nativeBuilder, jint r, jint g, jint b, jint a) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderSwizzle", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->swizzle(
                    (Texture::Swizzle)r, (Texture::Swizzle)g, (Texture::Swizzle)b, (Texture::Swizzle)a);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderImportTexture(JNIEnv* env, jclass, jlong nativeBuilder, jlong id) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderImportTexture", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->import((intptr_t)id);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderExternal(JNIEnv* env, jclass, jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nBuilderExternal", [&]() {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            builder->external();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nBuilderBuild(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_Texture_nBuilderBuild", 0, [&]() -> jlong {
            Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return (jlong) builder->build(*engine);
    });
}

// Texture...

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetWidth(JNIEnv* env, jclass, jlong nativeTexture,
        jint level) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetWidth", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getWidth((size_t) level);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetHeight(JNIEnv* env, jclass, jlong nativeTexture,
        jint level) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetHeight", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getHeight((size_t) level);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetDepth(JNIEnv* env, jclass, jlong nativeTexture,
        jint level) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetDepth", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getDepth((size_t) level);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetLevels(JNIEnv* env, jclass, jlong nativeTexture) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetLevels", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getLevels();
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetTarget(JNIEnv* env, jclass, jlong nativeTexture) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetTarget", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getTarget();
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetInternalFormat(JNIEnv* env, jclass,
        jlong nativeTexture) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGetInternalFormat", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            return (jint) texture->getFormat();
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImage3D(JNIEnv* env, jclass, jlong nativeTexture,
        jlong nativeEngine, jint level,
        jint xoffset, jint yoffset, jint zoffset,
        jint width, jint height, jint depth,
        jobject storage,  jint remaining,
        jint left, jint top, jint type, jint alignment,
        jint stride, jint format,
        jobject handler, jobject runnable) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nSetImage3D", 0, [&]() -> jint {
            Texture* texture = (Texture*) nativeTexture;
            Engine* engine = (Engine*) nativeEngine;

            size_t sizeInBytes = getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
                    (Texture::Type) type, (size_t) stride, (size_t) height, (size_t) alignment) * depth;

            AutoBuffer nioBuffer(env, storage, 0);
            if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }

            void *buffer = nioBuffer.getData();
            auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

            Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
                    (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) top,
                    (uint32_t) stride,
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

            texture->setImage(*engine, (size_t) level,
                    (uint32_t) xoffset, (uint32_t) yoffset, (uint32_t) zoffset,
                    (uint32_t) width, (uint32_t) height, (uint32_t) depth,
                    std::move(desc));

            return 0;
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImage3DCompressed(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level,
        jint xoffset, jint yoffset, jint zoffset,
        jint width, jint height, jint depth,
        jobject storage,  jint remaining,
        jint, jint, jint, jint, jint compressedSizeInBytes, jint compressedFormat,
        jobject handler, jobject runnable) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nSetImage3DCompressed", 0, [&]() -> jint {
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
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

            texture->setImage(*engine, (size_t) level,
                    (uint32_t) xoffset, (uint32_t) yoffset, (uint32_t) zoffset,
                    (uint32_t) width, (uint32_t) height, (uint32_t) depth,
                    std::move(desc));

            return 0;
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemap(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment, jint stride, jint format,
        jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nSetImageCubemap", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;

            jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, nullptr);
            Texture::FaceOffsets faceOffsets;
            std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
            env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

            size_t sizeInBytes = 6 * getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
                    (Texture::Type) type, (size_t) stride, 0, (size_t) alignment);

            AutoBuffer nioBuffer(env, storage, 0);
            if (sizeInBytes > (size_t(remaining) << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }

            void *buffer = nioBuffer.getData();
            auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

            Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
                    (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) top,
                    (uint32_t) stride,
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);
        #pragma clang diagnostic pop

            return 0;
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemapCompressed(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment,
        jint compressedSizeInBytes, jint compressedFormat, jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nSetImageCubemapCompressed", 0, [&]() -> jint {
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
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);
        #pragma clang diagnostic pop

            return 0;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalImage(JNIEnv* env, jclass, jlong nativeTexture,
        jlong nativeEngine, jlong eglImage) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nSetExternalImage", [&]() {
            Texture *texture = (Texture *) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;
            texture->setExternalImage(*engine, (void*)eglImage);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalStream(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jlong nativeStream) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nSetExternalStream", [&]() {
            Texture *texture = (Texture *) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;
            Stream *stream = (Stream *) nativeStream;
            texture->setExternalStream(*engine, stream);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nGenerateMipmaps(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Texture_nGenerateMipmaps", [&]() {
            Texture *texture = (Texture *) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;
            texture->generateMipmaps(*engine);
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsStreamValidForTexture(JNIEnv* env, jclass,
        jlong nativeTexture, jlong) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_Texture_nIsStreamValidForTexture", JNI_FALSE, [&]() -> jboolean {
            Texture* texture = (Texture*) nativeTexture;
            return (jboolean) (texture->getTarget() == SamplerType::SAMPLER_EXTERNAL);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGeneratePrefilterMipmap(JNIEnv *env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint width, jint height,
        jobject storage, jint remaining, jint left,
        jint top, jint type, jint alignment, jint stride, jint format,
        jintArray faceOffsetsInBytes_, jobject handler, jobject runnable, jint sampleCount,
        jboolean mirror) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Texture_nGeneratePrefilterMipmap", 0, [&]() -> jint {
            Texture *texture = (Texture *) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;

            jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, nullptr);
            filament::FaceOffsets faceOffsets;
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
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

            filament::PrefilterOptions options;
            options.sampleCount = sampleCount;
            options.mirror = mirror;

            filament::generatePrefilterMipmap(texture, *engine, std::move(desc), faceOffsets, &options);

            return 0;
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ANDROID SPECIFIC BITS
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __ANDROID__

#define BITMAP_CONFIG_ALPHA_8   0
#define BITMAP_CONFIG_RGB_565   1
#define BITMAP_CONFIG_RGBA_4444 2
#define BITMAP_CONFIG_RGBA_8888 3
#define BITMAP_CONFIG_RGBA_F16  4
#define BITMAP_CONFIG_HARDWARE  5

class AutoBitmap : public JniCallback {
private:

    AutoBitmap(JNIEnv* env, jobject bitmap) noexcept
            : JniCallback(),
              mBitmap(env->NewGlobalRef(bitmap)) {
        if (mBitmap) {
            AndroidBitmap_getInfo(env, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(env, mBitmap, &mData);
        }
    }

    AutoBitmap(JNIEnv* env, jobject bitmap, jobject handler, jobject runnable) noexcept
            : JniCallback(env, handler, runnable),
              mBitmap(env->NewGlobalRef(bitmap)) {
        if (mBitmap) {
            AndroidBitmap_getInfo(env, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(env, mBitmap, &mData);
        }
    }

    void release(JNIEnv* env) {
        if (mBitmap) {
            AndroidBitmap_unlockPixels(env, mBitmap);
            env->DeleteGlobalRef(mBitmap);
        }
    }

    ~AutoBitmap() override = default;

public:
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

    // create a AutoBitmap
    static AutoBitmap* make(JNIEnv* env, jobject bitmap, jobject handler, jobject runnable) {
        return new AutoBitmap(env, bitmap, handler, runnable);
    }

    // execute the callback on the java thread and destroy ourselves
    static void invoke(void*, size_t, void* user) {
        auto* autoBitmap = reinterpret_cast<AutoBitmap*>(user);
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        releaseCallbackJni(env, autoBitmap->mCallbackUtils, autoBitmap->mHandler, autoBitmap->mCallback);
        autoBitmap->release(env);
        delete autoBitmap;
    }

    // create a AutoBitmap without a handler
    static AutoBitmap* make(JNIEnv* env, jobject bitmap) {
        return new AutoBitmap(env, bitmap);
    }

    // just destroy ourselves
    static void invokeNoCallback(void*, size_t, void* user) {
        auto* autoBitmap = reinterpret_cast<AutoBitmap*>(user);
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        autoBitmap->release(env);
        delete autoBitmap;
    }

private:
    void* mData = nullptr;
    jobject mBitmap{};
    AndroidBitmapInfo mInfo{};
};

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetBitmap(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject bitmap, jint format) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_android_TextureHelper_nSetBitmap", [&]() {
            Texture* texture = (Texture*) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;

            auto* autoBitmap = AutoBitmap::make(env, bitmap);

            Texture::PixelBufferDescriptor desc(
                    autoBitmap->getData(),
                    autoBitmap->getSizeInBytes(),
                    autoBitmap->getFormat(format),
                    autoBitmap->getType(format),
                    &AutoBitmap::invokeNoCallback, autoBitmap);

            texture->setImage(*engine, (size_t) level,
                    (uint32_t) xoffset, (uint32_t) yoffset,
                    (uint32_t) width, (uint32_t) height,
                    std::move(desc));
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetBitmapWithCallback(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject bitmap, jint format, jobject handler, jobject runnable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_android_TextureHelper_nSetBitmapWithCallback", [&]() {
            Texture* texture = (Texture*) nativeTexture;
            Engine *engine = (Engine *) nativeEngine;

            auto* autoBitmap = AutoBitmap::make(env, bitmap, handler, runnable);

            Texture::PixelBufferDescriptor desc(
                    autoBitmap->getData(),
                    autoBitmap->getSizeInBytes(),
                    autoBitmap->getFormat(format),
                    autoBitmap->getType(format),
                    autoBitmap->getHandler(), &AutoBitmap::invoke, autoBitmap);

            texture->setImage(*engine, (size_t) level,
                    (uint32_t) xoffset, (uint32_t) yoffset,
                    (uint32_t) width, (uint32_t) height,
                    std::move(desc));
    });
}

#endif
