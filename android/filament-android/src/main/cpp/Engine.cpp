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

#include <filament/Engine.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateEngine(JNIEnv*, jclass, jlong backend,
        jlong sharedContext) {
    return (jlong) Engine::create((Engine::Backend) backend, nullptr, (void*) sharedContext);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyEngine(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    Engine::destroy(&engine);
}

// SwapChain

extern "C" {
// A new JNI platform must implement this method which retrieves the surface
// handle. Whatever object is returned from this method must match what is in
// folder filament/src/driver/opengl/Context* in particular pay attention to
// the object type in makeCurrent method.
extern void *getNativeWindow(JNIEnv *env, jclass, jobject surface);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetBackend(JNIEnv* env,
        jclass klass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->getBackend();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateSwapChain(JNIEnv* env,
        jclass klass, jlong nativeEngine, jobject surface, jlong flags) {
    Engine* engine = (Engine*) nativeEngine;
    void* win = getNativeWindow(env, klass, surface);
    return (jlong) engine->createSwapChain(win, (uint64_t) flags);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateSwapChainHeadless(JNIEnv* env,
        jclass klass, jlong nativeEngine, jint width, jint height, jlong flags) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createSwapChain(width, height, (uint64_t) flags);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateSwapChainFromRawPointer(JNIEnv*,
        jclass, jlong nativeEngine, jlong pointer, jlong flags) {
     Engine* engine = (Engine*) nativeEngine;
     return (jlong) engine->createSwapChain((void*)pointer, (uint64_t) flags);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroySwapChain(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeSwapChain) {
    Engine* engine = (Engine*) nativeEngine;
    SwapChain *swapChain = (SwapChain *) nativeSwapChain;
    return engine->destroy(swapChain);
}

// View

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateView(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createView();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyView(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeView) {
    Engine* engine = (Engine*) nativeEngine;
    View *view = (View *) nativeView;
    return engine->destroy(view);
}

// Renderer

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateRenderer(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createRenderer();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyRenderer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeRenderer) {
    Engine* engine = (Engine*) nativeEngine;
    Renderer *renderer = (Renderer *) nativeRenderer;
    return engine->destroy(renderer);
}

// Camera

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateCamera(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createCamera();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateCameraWithEntity(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    return (jlong) engine->createCamera(entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyCamera(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeCamera) {
    Engine* engine = (Engine*) nativeEngine;
    Camera *camera = (Camera *) nativeCamera;
    engine->destroy(camera);
}

// Scene

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateScene(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createScene();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyScene(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeScene) {
    Engine* engine = (Engine*) nativeEngine;
    Scene *scene = (Scene *) nativeScene;
    return engine->destroy(scene);
}

// Fence

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateFence(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createFence();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyFence(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeFence) {
    Engine* engine = (Engine*) nativeEngine;
    Fence *fence = (Fence *) nativeFence;
    return engine->destroy(fence);
}

// Stream

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyStream(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeStream) {
    Engine* engine = (Engine*) nativeEngine;
    Stream *stream = (Stream *) nativeStream;
    return engine->destroy(stream);
}

// Others...

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyIndexBuffer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeIndexBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
    return engine->destroy(indexBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyVertexBuffer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeVertexBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    return engine->destroy(vertexBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyIndirectLight(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeIndirectLight) {
    Engine* engine = (Engine*) nativeEngine;
    IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
    return engine->destroy(indirectLight);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyMaterial(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeMaterial) {
    Engine* engine = (Engine*) nativeEngine;
    Material *material = (Material *) nativeMaterial;
    return engine->destroy(material);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyMaterialInstance(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeMaterialInstance) {
    Engine* engine = (Engine*) nativeEngine;
    MaterialInstance* materialInstance = (MaterialInstance*) nativeMaterialInstance;
    return engine->destroy(materialInstance);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroySkybox(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeSkybox) {
    Engine* engine = (Engine*) nativeEngine;
    Skybox *skybox = (Skybox *) nativeSkybox;
    return engine->destroy(skybox);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyTexture(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeTexture) {
    Engine* engine = (Engine*) nativeEngine;
    Texture *texture = (Texture *) nativeTexture;
    return engine->destroy(texture);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyRenderTarget(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeTarget) {
    Engine* engine = (Engine*) nativeEngine;
    RenderTarget* target = (RenderTarget*) nativeTarget;
    return engine->destroy(target);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyEntity(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    engine->destroy(entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nFlushAndWait(JNIEnv *env, jclass clazz,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    engine->flushAndWait();
}

// Managers...

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetTransformManager(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getTransformManager();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetLightManager(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getLightManager();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetRenderableManager(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getRenderableManager();
}
