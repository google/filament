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

#include <filament/driver/BufferDescriptor.h>
#include <filament/Engine.h>
#include <filament/Stream.h>
#include <filament/Texture.h>

#include "CallbackUtils.h"
#include "NioUtils.h"

using namespace filament;
using namespace driver;

static size_t getTextureDataSize(const Texture *texture, size_t level,
        Texture::Format format, Texture::Type type, size_t stride, size_t alignment) {
    return Texture::computeTextureDataSize(format, type,
            std::max(size_t(1), stride >> level), texture->getHeight(level), alignment);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsTextureFormatSupported(JNIEnv *env, jclass type,
        jlong nativeEngine, jint internalFormat) {
    Engine *engine = (Engine *) nativeEngine;
    return (jboolean) Texture::isTextureFormatSupported(*engine,
            (Texture::InternalFormat) internalFormat);
}

// Texture::Builder...

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nCreateBuilder(JNIEnv *env, jclass type) {
    return (jlong) new Texture::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeBuilder) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderWidth(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint width) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->width((uint32_t) width);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderHeight(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint height) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->height((uint32_t) height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderDepth(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint depth) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->depth((uint32_t) depth);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderLevels(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint levels) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->levels((uint8_t) levels);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderSampler(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint sampler) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->sampler((Texture::Sampler) sampler);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderFormat(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint format) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->format((Texture::InternalFormat) format);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nBuilderRgbm(JNIEnv *env, jclass type,
        jlong nativeBuilder, jboolean enable) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    builder->rgbm(enable);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Texture_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeBuilder, jlong nativeEngine) {
    Texture::Builder *builder = (Texture::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

// Texture...

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetWidth(JNIEnv *env, jclass type, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getWidth((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetHeight(JNIEnv *env, jclass type, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getHeight((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetDepth(JNIEnv *env, jclass type, jlong nativeTexture,
        jint level) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getDepth((size_t) level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetLevels(JNIEnv *env, jclass type, jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getLevels();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetTarget(JNIEnv *env, jclass type, jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getTarget();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nGetInternalFormat(JNIEnv *env, jclass type,
        jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return (jint) texture->getFormat();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nGetRgbm(JNIEnv *env, jclass type, jlong nativeTexture) {
    Texture *texture = (Texture *) nativeTexture;
    return texture->isRgbm();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImage(JNIEnv *env, jclass type_, jlong nativeTexture,
        jlong nativeEngine, jint level, jint xoffset, jint yoffset, jint width, jint height,
        jobject storage,  jint remaining,
        jint left, jint bottom, jint type, jint alignment,
        jint stride, jint format,
        jobject handler, jobject runnable) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    size_t sizeInBytes = getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
            (Texture::Type) type, (size_t) stride, (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (driver::PixelDataFormat) format,
            (driver::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) bottom,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, (uint32_t) xoffset, (uint32_t) yoffset,
            (uint32_t) width, (uint32_t) height, std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCompressed(JNIEnv *env, jclass type_, jlong nativeTexture,
        jlong nativeEngine, jint level, jint xoffset, jint yoffset, jint width, jint height,
        jobject storage,  jint remaining,
        jint left, jint bottom, jint type, jint alignment,
        jint compressedSizeInBytes, jint compressedFormat,
        jobject handler, jobject runnable) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    size_t sizeInBytes = (size_t) compressedSizeInBytes;

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes,
            (driver::CompressedPixelDataType) compressedFormat, (uint32_t) compressedSizeInBytes,
            &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, (uint32_t) xoffset, (uint32_t) yoffset,
            (uint32_t) width, (uint32_t) height, std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemap(JNIEnv *env, jclass type_,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint bottom, jint type, jint alignment, jint stride, jint format,
        jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, NULL);
    Texture::FaceOffsets faceOffsets;
    std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
    env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

    size_t sizeInBytes = 6 * getTextureDataSize(texture, (size_t) level, (Texture::Format) format,
            (Texture::Type) type, (size_t) stride, (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes, (driver::PixelDataFormat) format,
            (driver::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) bottom,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Texture_nSetImageCubemapCompressed(JNIEnv *env, jclass type_,
        jlong nativeTexture, jlong nativeEngine, jint level, jobject storage, jint remaining,
        jint left, jint bottom, jint type, jint alignment,
        jint compressedSizeInBytes, jint compressedFormat, jintArray faceOffsetsInBytes_,
        jobject handler, jobject runnable) {

    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    jint *faceOffsetsInBytes = env->GetIntArrayElements(faceOffsetsInBytes_, NULL);
    Texture::FaceOffsets faceOffsets;
    std::copy_n(faceOffsetsInBytes, 6, faceOffsets.offsets);
    env->ReleaseIntArrayElements(faceOffsetsInBytes_, faceOffsetsInBytes, JNI_ABORT);

    size_t sizeInBytes = 6 * (size_t) compressedSizeInBytes;

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    Texture::PixelBufferDescriptor desc(buffer, sizeInBytes,
            (driver::CompressedPixelDataType) compressedFormat, (uint32_t) compressedSizeInBytes,
            &JniBufferCallback::invoke, callback);

    texture->setImage(*engine, (size_t) level, std::move(desc), faceOffsets);

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalImage(JNIEnv*, jclass, jlong nativeTexture, jlong nativeEngine, jlong eglImage) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    texture->setExternalImage(*engine, (void*)eglImage);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nSetExternalStream(JNIEnv *env, jclass type,
        jlong nativeTexture, jlong nativeEngine, jlong nativeStream) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    Stream *stream = (Stream *) nativeStream;
    texture->setExternalStream(*engine, stream);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Texture_nGenerateMipmaps(JNIEnv *env, jclass type,
        jlong nativeTexture, jlong nativeEngine) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;
    texture->generateMipmaps(*engine);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Texture_nIsStreamValidForTexture(JNIEnv*, jclass,
        jlong nativeTexture, jlong nativeStream) {
    Texture* texture = (Texture*) nativeTexture;
    Stream* stream = (Stream*) nativeStream;
    return (jboolean) (texture->getTarget() == SamplerType::SAMPLER_EXTERNAL);
}

