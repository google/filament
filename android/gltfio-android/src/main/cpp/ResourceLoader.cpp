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

#include <utils/Log.h>

#include "common/NioUtils.h"

using namespace filament;
using namespace gltfio;
using namespace utils;

static void destroy(void* data, size_t size, void *userData) {
    AutoBuffer* buffer = (AutoBuffer*) userData;
    delete buffer;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nCreateResourceLoader(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) new ResourceLoader({ engine, utils::Path(), true, true });
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
    const char* curl = env->GetStringUTFChars(url, nullptr);
    loader->addResourceData(curl,
            ResourceLoader::BufferDescriptor(buffer->getData(), buffer->getSize(), &destroy,
                    buffer));
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_ResourceLoader_nLoadResources(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    ResourceLoader* loader = (ResourceLoader*) nativeLoader;
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    loader->loadResources(asset);
}
