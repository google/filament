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

#include <exception>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/MorphTargetBuffer.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/tribool.h>

#include <filament/Material.h>
#include <filament/View.h>

#include "common/CallbackUtils.h"
#include "common/JniUtils.h"

using namespace filament;
using namespace utils;
using namespace filament::android;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyEngine(JNIEnv *env, jclass, jlong nativeEngine) {
    wrapJni(env, [=]() {
        Engine* engine = (Engine*) nativeEngine;
        Engine::destroy(&engine);
    });
}

// SwapChain

extern "C" {
// A new JNI platform must implement this method which retrieves the surface
// handle. Whatever object is returned from this method must match what is in
// folder filament/src/driver/opengl/Context* in particular pay attention to
// the object type in makeCurrent method.
extern void* getNativeWindow(JNIEnv* env, jclass, jobject surface);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetBackend(JNIEnv*, jclass, jlong nativeEngine) {
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
Java_com_google_android_filament_Engine_nCreateSwapChainHeadless(JNIEnv*,
        jclass, jlong nativeEngine, jint width, jint height, jlong flags) {
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
Java_com_google_android_filament_Engine_nDestroySwapChain(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeSwapChain) {
    Engine* engine = (Engine*) nativeEngine;
    SwapChain* swapChain = (SwapChain*) nativeSwapChain;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(swapChain);
    });
}

// View

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateView(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createView();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyView(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeView) {
    Engine* engine = (Engine*) nativeEngine;
    View* view = (View*) nativeView;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(view);
    });
}

// Renderer

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateRenderer(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createRenderer();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyRenderer(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeRenderer) {
    Engine* engine = (Engine*) nativeEngine;
    Renderer* renderer = (Renderer*) nativeRenderer;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(renderer);
    });
}

// Camera

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateCamera(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    return (jlong) engine->createCamera(entity);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetCameraComponent(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    return (jlong) engine->getCameraComponent(entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyCameraComponent(JNIEnv *env, jclass,
        jlong nativeEngine, jint entity_) {
    wrapJni(env, [=]() {
        Engine* engine = (Engine*) nativeEngine;
        Entity entity = *reinterpret_cast<const Entity*>(&entity_);
        engine->destroyCameraComponent(entity);
    });
}

// Scene

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateScene(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createScene();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyScene(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeScene) {
    Engine* engine = (Engine*) nativeEngine;
    Scene* scene = (Scene*) nativeScene;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(scene);
    });
}

// Fence

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nCreateFence(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createFence();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyFence(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeFence) {
    Engine* engine = (Engine*) nativeEngine;
    Fence* fence = (Fence*) nativeFence;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(fence);
    });
}

// Stream

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyStream(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeStream) {
    Engine* engine = (Engine*) nativeEngine;
    Stream* stream = (Stream*) nativeStream;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(stream);
    });
}

// Others...

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyIndexBuffer(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeIndexBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    IndexBuffer* indexBuffer = (IndexBuffer*) nativeIndexBuffer;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(indexBuffer);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyVertexBuffer(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeVertexBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    VertexBuffer* vertexBuffer = (VertexBuffer*) nativeVertexBuffer;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(vertexBuffer);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroySkinningBuffer(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeSkinningBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    SkinningBuffer* skinningBuffer = (SkinningBuffer*) nativeSkinningBuffer;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(skinningBuffer);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyMorphTargetBuffer(JNIEnv *env, jclass,
                jlong nativeEngine, jlong nativeMorphTargetBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    MorphTargetBuffer* mtb = (MorphTargetBuffer*) nativeMorphTargetBuffer;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(mtb);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyIndirectLight(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeIndirectLight) {
    Engine* engine = (Engine*) nativeEngine;
    IndirectLight* indirectLight = (IndirectLight*) nativeIndirectLight;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(indirectLight);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyMaterial(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeMaterial) {
    Engine* engine = (Engine*) nativeEngine;
    Material* material = (Material*) nativeMaterial;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(material);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyMaterialInstance(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeMaterialInstance) {
    Engine* engine = (Engine*) nativeEngine;
    MaterialInstance* materialInstance = (MaterialInstance*) nativeMaterialInstance;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(materialInstance);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroySkybox(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeSkybox) {
    Engine* engine = (Engine*) nativeEngine;
    Skybox* skybox = (Skybox*) nativeSkybox;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(skybox);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyColorGrading(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeColorGrading) {
    Engine* engine = (Engine*) nativeEngine;
    ColorGrading* colorGrading = (ColorGrading*) nativeColorGrading;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(colorGrading);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyTexture(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeTexture) {
    Engine* engine = (Engine*) nativeEngine;
    Texture* texture = (Texture*) nativeTexture;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(texture);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nDestroyRenderTarget(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeTarget) {
    Engine* engine = (Engine*) nativeEngine;
    RenderTarget* target = (RenderTarget*) nativeTarget;
    return wrapJni<jboolean>(env, [=]() {
        return engine->destroy(target);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nDestroyEntity(JNIEnv *env, jclass,
        jlong nativeEngine, jint entity_) {
    wrapJni(env, [=]() {
        Engine* engine = (Engine*) nativeEngine;
        Entity entity = *reinterpret_cast<const Entity*>(&entity_);
        engine->destroy(entity);
    });
}


extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidRenderer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeRenderer) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Renderer*)nativeRenderer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidView(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeView) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((View*)nativeView);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidScene(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeScene) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Scene*)nativeScene);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidFence(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeFence) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Fence*)nativeFence);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidStream(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeStream) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Stream*)nativeStream);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidIndexBuffer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeIndexBuffer) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((IndexBuffer*)nativeIndexBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidVertexBuffer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeVertexBuffer) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((VertexBuffer*)nativeVertexBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidSkinningBuffer(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeSkinningBuffer) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((SkinningBuffer*)nativeSkinningBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidMorphTargetBuffer(JNIEnv*, jclass,
                jlong nativeEngine, jlong nativeMorphTargetBuffer) {
    Engine* engine = (Engine*) nativeEngine;
    return (jboolean) engine->isValid((MorphTargetBuffer*) nativeMorphTargetBuffer);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidIndirectLight(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeIndirectLight) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((IndirectLight*)nativeIndirectLight);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidMaterial(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeMaterial) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Material*)nativeMaterial);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidMaterialInstance(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeMaterial, jlong nativeMaterialInstance) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Material*)nativeMaterial,
            (MaterialInstance*)nativeMaterialInstance);
}

extern "C" JNIEXPORT jboolean JNICALL
        Java_com_google_android_filament_Engine_nIsValidExpensiveMaterialInstance(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeMaterialInstance) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValidExpensive((MaterialInstance*)nativeMaterialInstance);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidSkybox(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeSkybox) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Skybox*)nativeSkybox);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidColorGrading(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeColorGrading) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((ColorGrading*)nativeColorGrading);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidTexture(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeTexture) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((Texture*)nativeTexture);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidRenderTarget(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeTarget) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((RenderTarget*)nativeTarget);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsValidSwapChain(JNIEnv*, jclass,
        jlong nativeEngine, jlong nativeSwapChain) {
    Engine* engine = (Engine *)nativeEngine;
    return (jboolean)engine->isValid((SwapChain*)nativeSwapChain);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nFlushAndWait(JNIEnv *env, jclass,
        jlong nativeEngine, jlong timeout) {
    Engine* engine = (Engine*) nativeEngine;
    return wrapJni<jboolean>(env, [=]() {
        return engine->flushAndWait((uint64_t)timeout);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nFlush(JNIEnv *env, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    wrapJni(env, [=]() {
        engine->flush();
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsPaused(JNIEnv*, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jboolean)engine->isPaused();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nCompile(JNIEnv *env, jclass,
        jlong nativeEngine, jint priority, jlong nativeMaterial, jlong nativeView,
        jint shadowReceiver, jint skinning, jobject handler, jobject runnable) {
    Engine* engine = (Engine*) nativeEngine;
    Material* material = (Material*) nativeMaterial;
    View* view = (View*) nativeView;
    JniCallback* jniCallback = JniCallback::make(env, handler, runnable);
    wrapJni(env, [=]() {
#if defined(__EXCEPTIONS)
        try {
#endif
            engine->compile(
                    (backend::CompilerPriorityQueue) priority,
                    material, view,
                    (utils::tribool::value_t) shadowReceiver,
                    (utils::tribool::value_t) skinning,
                    jniCallback->getHandler(), [jniCallback](Material*){
                        JniCallback::postToJavaAndDestroy(jniCallback);
                    });
#if defined(__EXCEPTIONS)
        } catch (...) {
            JniCallback::destroy(jniCallback);
            throw;
        }
#endif
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nSetPaused(JNIEnv*, jclass,
        jlong nativeEngine, jboolean paused) {
    Engine* engine = (Engine*) nativeEngine;
    engine->setPaused(paused);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nUnprotected(JNIEnv*, jclass,
        jlong nativeEngine, jboolean paused) {
    Engine* engine = (Engine*) nativeEngine;
    engine->unprotected();
}

// Managers...

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetTransformManager(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getTransformManager();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetLightManager(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getLightManager();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetRenderableManager(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getRenderableManager();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetJobSystem(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getJobSystem();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetEntityManager(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) &engine->getEntityManager();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nSetAutomaticInstancingEnabled(JNIEnv*, jclass, jlong nativeEngine, jboolean enable) {
    Engine* engine = (Engine*) nativeEngine;
    engine->setAutomaticInstancingEnabled(enable);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nIsAutomaticInstancingEnabled(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jboolean)engine->isAutomaticInstancingEnabled();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nHasUnrecoverableFailure(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jboolean)engine->hasUnrecoverableFailure();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nGetMaxStereoscopicEyes(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->getMaxStereoscopicEyes();
}


extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Engine_nGetSupportedFeatureLevel(JNIEnv *, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jint)engine->getSupportedFeatureLevel();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Engine_nSetActiveFeatureLevel(JNIEnv *env, jclass,
        jlong nativeEngine, jint ordinal) {
    Engine* engine = (Engine*) nativeEngine;
    return wrapJni<jint>(env, [=]() {
        return (jint)engine->setActiveFeatureLevel((Engine::FeatureLevel)ordinal);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Engine_nGetActiveFeatureLevel(JNIEnv *, jclass,
        jlong nativeEngine) {
    Engine* engine = (Engine*) nativeEngine;
    return (jint)engine->getActiveFeatureLevel();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nHasFeatureFlag(JNIEnv *env, jclass clazz,
        jlong nativeEngine, jstring name_) {
    Engine* engine = (Engine*) nativeEngine;
    const char *name = env->GetStringUTFChars(name_, 0);
    std::optional<bool> result = engine->getFeatureFlag(name);
    env->ReleaseStringUTFChars(name_, name);
    return result.has_value();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nSetFeatureFlag(JNIEnv *env, jclass clazz,
        jlong nativeEngine, jstring name_, jboolean value) {
    Engine* engine = (Engine*) nativeEngine;
    const char *name = env->GetStringUTFChars(name_, 0);
    jboolean result = engine->setFeatureFlag(name, (bool)value);
    env->ReleaseStringUTFChars(name_, name);
    return result;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Engine_nGetFeatureFlag(JNIEnv *env, jclass clazz,
        jlong nativeEngine, jstring name_) {
    Engine* engine = (Engine*) nativeEngine;
    const char *name = env->GetStringUTFChars(name_, 0);
    std::optional<bool> result = engine->getFeatureFlag(name);
    env->ReleaseStringUTFChars(name_, name);
    return result.value_or(false); // we should never fail here
}

extern "C" JNIEXPORT jlong JNICALL Java_com_google_android_filament_Engine_nCreateBuilder(JNIEnv*,
        jclass) {
    Engine::Builder* builder = new Engine::Builder{};
    return (jlong) builder;
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nDestroyBuilder(JNIEnv*,
        jclass, jlong nativeBuilder) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nSetBuilderBackend(
        JNIEnv*, jclass, jlong nativeBuilder, jlong backend) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->backend((Engine::Backend) backend);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nSetBuilderConfig(JNIEnv*,
        jclass, jlong nativeBuilder, jlong commandBufferSizeMB, jlong perRenderPassArenaSizeMB,
        jlong driverHandleArenaSizeMB, jlong minCommandBufferSizeMB, jlong perFrameCommandsSizeMB,
        jlong jobSystemThreadCount, jboolean disableParallelShaderCompile,
        jint stereoscopicType, jlong stereoscopicEyeCount,
        jlong resourceAllocatorCacheSizeMB, jlong resourceAllocatorCacheMaxAge,
        jboolean disableHandleUseAfterFreeCheck,
        jint preferredShaderLanguage,
        jboolean forceGLES2Context, jboolean assertNativeWindowIsValid,
        jint gpuContextPriority,
        jlong sharedUboInitialSizeInBytes) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    Engine::Config config = {
            .commandBufferSizeMB = (uint32_t) commandBufferSizeMB,
            .perRenderPassArenaSizeMB = (uint32_t) perRenderPassArenaSizeMB,
            .driverHandleArenaSizeMB = (uint32_t) driverHandleArenaSizeMB,
            .minCommandBufferSizeMB = (uint32_t) minCommandBufferSizeMB,
            .perFrameCommandsSizeMB = (uint32_t) perFrameCommandsSizeMB,
            .jobSystemThreadCount = (uint32_t) jobSystemThreadCount,
            .disableParallelShaderCompile = (bool) disableParallelShaderCompile,
            .stereoscopicType = (Engine::StereoscopicType) stereoscopicType,
            .stereoscopicEyeCount = (uint8_t) stereoscopicEyeCount,
            .resourceAllocatorCacheSizeMB = (uint32_t) resourceAllocatorCacheSizeMB,
            .resourceAllocatorCacheMaxAge = (uint8_t) resourceAllocatorCacheMaxAge,
            .disableHandleUseAfterFreeCheck = (bool) disableHandleUseAfterFreeCheck,
            .preferredShaderLanguage = (Engine::Config::ShaderLanguage) preferredShaderLanguage,
            .forceGLES2Context = (bool) forceGLES2Context,
            .assertNativeWindowIsValid = (bool) assertNativeWindowIsValid,
            .gpuContextPriority = (Engine::GpuContextPriority) gpuContextPriority,
            .sharedUboInitialSizeInBytes = (uint32_t) sharedUboInitialSizeInBytes,
    };
    builder->config(&config);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nSetBuilderFeatureLevel(
        JNIEnv*, jclass, jlong nativeBuilder, jint ordinal) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->featureLevel((Engine::FeatureLevel)ordinal);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nSetBuilderSharedContext(
        JNIEnv*, jclass, jlong nativeBuilder, jlong sharedContext) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->sharedContext((void*) sharedContext);
}

extern "C" JNIEXPORT void JNICALL Java_com_google_android_filament_Engine_nSetBuilderPaused(
        JNIEnv*, jclass, jlong nativeBuilder, jboolean paused) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->paused((bool) paused);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_Engine_nSetBuilderFeature(JNIEnv *env, jclass clazz,
        jlong nativeBuilder, jstring name_, jboolean value) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    const char *name = env->GetStringUTFChars(name_, 0);
    builder->feature(name, (bool)value);
    env->ReleaseStringUTFChars(name_, name);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_nBuilderBuild(JNIEnv *env, jclass, jlong nativeBuilder) {
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    return wrapJniBackend<jlong>(env, [=]() {
        return (jlong) builder->build();
    });
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Engine_getSteadyClockTimeNano(JNIEnv *env, jclass clazz) {
    return (jlong)Engine::getSteadyClockTimeNano();
}
