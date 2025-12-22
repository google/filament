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

#include <filament/RenderableManager.h>
#include <filament/MaterialInstance.h>

#include <utils/Entity.h>

#include "common/NioUtils.h"
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nHasComponent(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint entity) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nHasComponent", JNI_FALSE, [&]() -> jboolean {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jboolean) rm->hasComponent((Entity &) entity);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetInstance(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint entity) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nGetInstance", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return rm->getInstance((Entity &) entity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nDestroy(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint entity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nDestroy", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->destroy((Entity &) entity);
    });
}



extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderableManager_nCreateBuilder(JNIEnv* env, jclass,
        jint count) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_RenderableManager_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new RenderableManager::Builder((size_t) count);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nDestroyBuilder", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBuild(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeEngine, jint entity) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nBuilderBuild", JNI_FALSE, [&]() -> jboolean {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return jboolean(builder->build(*engine, (Entity &) entity) == RenderableManager::Builder::Success);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJ(JNIEnv* env, jclass,
        jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJ", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
                    vertexBuffer, indexBuffer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJII(JNIEnv* env,
        jclass, jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer, jint offset, jint count) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJII", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
                    vertexBuffer, indexBuffer, (size_t) offset, (size_t) count);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJIIII(JNIEnv* env,
        jclass, jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer, jint offset, jint minIndex, jint maxIndex, jint count) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJIIII", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
                    vertexBuffer, indexBuffer, (size_t) offset, (size_t) minIndex, (size_t) maxIndex,
                    (size_t) count);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometryType(JNIEnv* env, jclass,
        jlong nativeBuilder, int type) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderGeometryType", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->geometryType((RenderableManager::Builder::GeometryType)type);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderMaterial(JNIEnv* env, jclass,
        jlong nativeBuilder, jint index, jlong nativeMaterialInstance) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderMaterial", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->material((size_t) index, (const MaterialInstance*) nativeMaterialInstance);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBlendOrder(JNIEnv* env, jclass,
        jlong nativeBuilder, jint index, jint blendOrder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderBlendOrder", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->blendOrder((size_t) index, (uint16_t) blendOrder);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGlobalBlendOrderEnabled(JNIEnv* env, jclass,
        jlong nativeBuilder, jint index, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderGlobalBlendOrderEnabled", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->globalBlendOrderEnabled((size_t) index, (bool) enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBoundingBox(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat cx, jfloat cy, jfloat cz, jfloat ex, jfloat ey, jfloat ez) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderBoundingBox", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->boundingBox({{cx, cy, cz},
                                  {ex, ey, ez}});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderLayerMask(JNIEnv* env, jclass,
        jlong nativeBuilder, jint select, jint value) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderLayerMask", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->layerMask((uint8_t) select, (uint8_t) value);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderPriority(JNIEnv* env, jclass,
        jlong nativeBuilder, jint priority) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderPriority", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->priority((uint8_t) priority);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderChannel(JNIEnv* env, jclass,
        jlong nativeBuilder, jint channel) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderChannel", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->channel((uint8_t) channel);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderCulling(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderCulling", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->culling(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderCastShadows(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderCastShadows", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->castShadows(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderReceiveShadows(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderReceiveShadows", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->receiveShadows(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderScreenSpaceContactShadows(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderScreenSpaceContactShadows", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->screenSpaceContactShadows(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSkinningBuffer(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeSkinningBuffer, jint boneCount, jint offset) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderSkinningBuffer", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            SkinningBuffer *skinningBuffer = (SkinningBuffer *) nativeSkinningBuffer;
            builder->skinning(skinningBuffer, boneCount, offset);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSkinning(JNIEnv* env, jclass,
        jlong nativeBuilder, jint boneCount) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderSkinning", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->skinning((size_t)boneCount);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderEnableSkinningBuffers(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderEnableSkinningBuffers", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->enableSkinningBuffers(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderFog(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderFog", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->fog(enabled);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSkinningBones(JNIEnv* env, jclass,
        jlong nativeBuilder, jint boneCount, jobject bones, jint remaining) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nBuilderSkinningBones", 0, [&]() -> jint {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            AutoBuffer nioBuffer(env, bones, boneCount * 8);
            void* data = nioBuffer.getData();
            size_t sizeInBytes = nioBuffer.getSize();
            if (sizeInBytes > (remaining << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }
            builder->skinning((size_t)boneCount, static_cast<RenderableManager::Bone const*>(data));
            return 0;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderMorphing(JNIEnv* env, jclass,
        jlong nativeBuilder, jint targetCount) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderMorphing", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->morphing(targetCount);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderMorphingStandard(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeMorphTargetBuffer) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderMorphingStandard", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            MorphTargetBuffer *morphTargetBuffer = (MorphTargetBuffer *) nativeMorphTargetBuffer;
            builder->morphing(morphTargetBuffer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSetMorphTargetBufferOffsetAt(JNIEnv* env, jclass,
        jlong nativeBuilder, int level, int primitiveIndex, int offset) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderSetMorphTargetBufferOffsetAt", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->morphing(level, primitiveIndex, offset);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderLightChannel(JNIEnv* env, jclass,
        jlong nativeBuilder, jint channel, jboolean enable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderLightChannel", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->lightChannel(channel, (bool)enable);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderInstances(JNIEnv* env, jclass,
        jlong nativeBuilder, jint instanceCount) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nBuilderInstances", [&]() {
            RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
            builder->instances(instanceCount);
    });
}

// ------------------------------------------------------------------------------------------------

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetSkinningBuffer(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jlong nativeSkinningBuffer, jint count, jint offset) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetSkinningBuffer", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            SkinningBuffer *sb = (SkinningBuffer *) nativeSkinningBuffer;
            rm->setSkinningBuffer(i, sb, count, offset);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nSetBonesAsMatrices(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jobject matrices, jint remaining,
        jint boneCount, jint offset) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nSetBonesAsMatrices", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            AutoBuffer nioBuffer(env, matrices, boneCount * 16);
            void* data = nioBuffer.getData();
            size_t sizeInBytes = nioBuffer.getSize();
            if (sizeInBytes > (remaining << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }
            rm->setBones((RenderableManager::Instance)i,
                    static_cast<filament::math::mat4f const *>(data), (size_t)boneCount, (size_t)offset);
            return 0;
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nSetBonesAsQuaternions(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jobject quaternions, jint remaining,
        jint boneCount, jint offset) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nSetBonesAsQuaternions", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            AutoBuffer nioBuffer(env, quaternions, boneCount * 8);
            void* data = nioBuffer.getData();
            size_t sizeInBytes = nioBuffer.getSize();
            if (sizeInBytes > (remaining << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }
            rm->setBones((RenderableManager::Instance)i,
                    static_cast<RenderableManager::Bone const *>(data), (size_t)boneCount, (size_t)offset);
            return 0;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetMorphWeights(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint instance, jfloatArray weights, jint offset) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetMorphWeights", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            jfloat* vec = env->GetFloatArrayElements(weights, NULL);
            jsize count = env->GetArrayLength(weights);
            rm->setMorphWeights((RenderableManager::Instance)instance, vec, count, offset);
            env->ReleaseFloatArrayElements(weights, vec, JNI_ABORT);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetMorphTargetBufferOffsetAt(JNIEnv* env,
        jclass, jlong nativeRenderableManager, jint i, int level, jint primitiveIndex,
        jlong, jint offset) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetMorphTargetBufferOffsetAt", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setMorphTargetBufferOffsetAt((RenderableManager::Instance) i, (uint8_t) level,
                    (size_t) primitiveIndex, (size_t) offset);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetMorphTargetCount(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint instance) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nGetMorphTargetCount", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return rm->getMorphTargetCount((RenderableManager::Instance)instance);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetAxisAlignedBoundingBox(JNIEnv* env,
        jclass, jlong nativeRenderableManager, jint i, jfloat cx, jfloat cy, jfloat cz,
        jfloat ex, jfloat ey, jfloat ez) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetAxisAlignedBoundingBox", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setAxisAlignedBoundingBox((RenderableManager::Instance) i, {{cx, cy, cz},
                                                                            {ex, ey, ez}});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetLayerMask(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint select, jint value) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetLayerMask", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setLayerMask((RenderableManager::Instance) i, (uint8_t) select, (uint8_t) value);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetPriority(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint priority) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetPriority", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setPriority((RenderableManager::Instance) i, (uint8_t) priority);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetChannel(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint channel) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetChannel", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setChannel((RenderableManager::Instance) i, (uint8_t) channel);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetCulling(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetCulling", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setCulling((RenderableManager::Instance) i, enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetFogEnabled(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetFogEnabled", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setFogEnabled((RenderableManager::Instance) i, enabled);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nGetFogEnabled(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nGetFogEnabled", JNI_FALSE, [&]() -> jboolean {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jboolean)rm->getFogEnabled((RenderableManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetCastShadows(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetCastShadows", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setCastShadows((RenderableManager::Instance) i, enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetReceiveShadows(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetReceiveShadows", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setReceiveShadows((RenderableManager::Instance) i, enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetScreenSpaceContactShadows(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetScreenSpaceContactShadows", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setScreenSpaceContactShadows((RenderableManager::Instance) i, enabled);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nIsShadowCaster(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nIsShadowCaster", JNI_FALSE, [&]() -> jboolean {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jboolean) rm->isShadowCaster((RenderableManager::Instance) i);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nIsShadowReceiver(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nIsShadowReceiver", JNI_FALSE, [&]() -> jboolean {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jboolean) rm->isShadowReceiver((RenderableManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nGetAxisAlignedBoundingBox(JNIEnv* env,
        jclass, jlong nativeRenderableManager, jint i, jfloatArray center_,
        jfloatArray halfExtent_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nGetAxisAlignedBoundingBox", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            jfloat *center = env->GetFloatArrayElements(center_, NULL);
            jfloat *halfExtent = env->GetFloatArrayElements(halfExtent_, NULL);
            Box const &aabb = rm->getAxisAlignedBoundingBox((RenderableManager::Instance) i);
            *reinterpret_cast<filament::math::float3 *>(center) = aabb.center;
            *reinterpret_cast<filament::math::float3 *>(halfExtent) = aabb.halfExtent;
            env->ReleaseFloatArrayElements(center_, center, 0);
            env->ReleaseFloatArrayElements(halfExtent_, halfExtent, 0);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetPrimitiveCount(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nGetPrimitiveCount", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jint) rm->getPrimitiveCount((RenderableManager::Instance) i);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetInstanceCount(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nGetInstanceCount", 0, [&]() -> jint {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (jint) rm->getInstanceCount((RenderableManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetMaterialInstanceAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jlong nativeMaterialInstance) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetMaterialInstanceAt", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            const MaterialInstance *materialInstance = (const MaterialInstance *) nativeMaterialInstance;
            rm->setMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
                    materialInstance);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nClearMaterialInstanceAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nClearMaterialInstanceAt", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->clearMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderableManager_nGetMaterialInstanceAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_RenderableManager_nGetMaterialInstanceAt", 0, [&]() -> jlong {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            return (long) rm->getMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetGeometryAt__JIIIJJII(JNIEnv* env,
        jclass, jlong nativeRenderableManager, jint i, jint primitiveIndex, jint primitiveType,
        jlong nativeVertexBuffer, jlong nativeIndexBuffer, jint offset, jint count) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetGeometryAt__JIIIJJII", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            rm->setGeometryAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
                    (RenderableManager::PrimitiveType) primitiveType, vertexBuffer, indexBuffer,
                    (size_t) offset, (size_t) count);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetBlendOrderAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jint blendOrder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetBlendOrderAt", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setBlendOrderAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
                    (uint16_t) blendOrder);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetGlobalBlendOrderEnabledAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetGlobalBlendOrderEnabledAt", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setGlobalBlendOrderEnabledAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
                    (bool) enabled);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetEnabledAttributesAt(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderableManager_nGetEnabledAttributesAt", 0, [&]() -> jint {
            RenderableManager const *rm = (RenderableManager const *) nativeRenderableManager;
            AttributeBitset enabled = rm->getEnabledAttributesAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
            return enabled.getValue();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetLightChannel(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint channel, jboolean enable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderableManager_nSetLightChannel", [&]() {
            RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
            rm->setLightChannel((RenderableManager::Instance) i, channel, (bool)enable);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nGetLightChannel(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jint channel) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_RenderableManager_nGetLightChannel", JNI_FALSE, [&]() -> jboolean {
            RenderableManager const *rm = (RenderableManager const *) nativeRenderableManager;
            return rm->getLightChannel((RenderableManager::Instance) i, channel);
    });
}
