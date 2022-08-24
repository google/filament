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

#include <jni.h>

#include <filament/Engine.h>

#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <utils/Log.h>

#include "common/NioUtils.h"

using namespace filament;
using namespace filament::gltfio;
using namespace utils;

static void destroy(void*, size_t, void *userData) {
    AutoBuffer* buffer = (AutoBuffer*) userData;
    delete buffer;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nCreateResourceLoader(JNIEnv*, jclass,
        jlong nativeEngine, jboolean normalizeSkinningWeights) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) new ResourceLoader({ engine, {}, (bool) normalizeSkinningWeights});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nDestroyResourceLoader(JNIEnv*, jclass,
        jlong nativeLoader) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    delete loader;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAddResourceData(JNIEnv* env, jclass,
        jlong nativeLoader, jstring url, jobject javaBuffer, jint remaining) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    AutoBuffer* buffer = new AutoBuffer(env, javaBuffer, remaining);
    const char* cstring = env->GetStringUTFChars(url, nullptr);
    loader->addResourceData(cstring,
            ResourceLoader::BufferDescriptor(buffer->getData(), buffer->getSize(), &destroy,
                    buffer));
    env->ReleaseStringUTFChars(url, cstring);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nHasResourceData(JNIEnv* env, jclass,
        jlong nativeLoader, jstring url) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    const char* cstring = env->GetStringUTFChars(url, nullptr);
    bool status = loader->hasResourceData(cstring);
    env->ReleaseStringUTFChars(url, cstring);
    return status;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nEvictResourceData(JNIEnv*, jclass,
        jlong nativeLoader) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    loader->evictResourceData();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nLoadResources(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    loader->loadResources(asset);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAsyncBeginLoad(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    return loader->asyncBeginLoad(asset);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAsyncGetLoadProgress(JNIEnv*, jclass,
        jlong nativeLoader) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    return loader->asyncGetLoadProgress();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAsyncUpdateLoad(JNIEnv*, jclass,
        jlong nativeLoader) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    loader->asyncUpdateLoad();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAsyncCancelLoad(JNIEnv*, jclass,
        jlong nativeLoader) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    loader->asyncCancelLoad();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nCreateStbProvider(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) createStbProvider(engine);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nCreateKtx2Provider(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) createKtx2Provider(engine);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nDestroyTextureProvider(JNIEnv*, jclass,
        jlong nativeProvider) {
    TextureProvider* provider = (TextureProvider*) nativeProvider;
    delete provider;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nAddTextureProvider(JNIEnv* env, jclass,
        jlong nativeLoader, jstring url, jlong nativeProvider) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    TextureProvider* provider = (TextureProvider*) nativeProvider;
    const char* cstring = env->GetStringUTFChars(url, nullptr);
    loader->addTextureProvider(cstring, provider);
    env->ReleaseStringUTFChars(url, cstring);
}
