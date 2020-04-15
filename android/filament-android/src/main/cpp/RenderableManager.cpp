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

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nHasComponent(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint entity) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return (jboolean) rm->hasComponent((Entity &) entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetInstance(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint entity) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return rm->getInstance((Entity &) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nDestroy(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint entity) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->destroy((Entity &) entity);
}



extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderableManager_nCreateBuilder(JNIEnv*, jclass,
        jint count) {
    return (jlong) new RenderableManager::Builder((size_t) count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine, jint entity) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return jboolean(builder->build(*engine, (Entity &) entity) == RenderableManager::Builder::Success);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJ(JNIEnv*, jclass,
        jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
    builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
            vertexBuffer, indexBuffer);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJII(JNIEnv*,
        jclass, jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer, jint offset, jint count) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
    builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
            vertexBuffer, indexBuffer, (size_t) offset, (size_t) count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderGeometry__JIIJJIIII(JNIEnv*,
        jclass, jlong nativeBuilder, jint index, jint primitiveType, jlong nativeVertexBuffer,
        jlong nativeIndexBuffer, jint offset, jint minIndex, jint maxIndex, jint count) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
    builder->geometry((size_t) index, (RenderableManager::PrimitiveType) primitiveType,
            vertexBuffer, indexBuffer, (size_t) offset, (size_t) minIndex, (size_t) maxIndex,
            (size_t) count);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderMaterial(JNIEnv*, jclass,
        jlong nativeBuilder, jint index, jlong nativeMaterialInstance) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->material((size_t) index, (const MaterialInstance*) nativeMaterialInstance);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBlendOrder(JNIEnv*, jclass,
        jlong nativeBuilder, jint index, jint blendOrder) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->blendOrder((size_t) index, (uint16_t) blendOrder);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderBoundingBox(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat cx, jfloat cy, jfloat cz, jfloat ex, jfloat ey, jfloat ez) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->boundingBox({{cx, cy, cz},
                          {ex, ey, ez}});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderLayerMask(JNIEnv*, jclass,
        jlong nativeBuilder, jint select, jint value) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->layerMask((uint8_t) select, (uint8_t) value);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderPriority(JNIEnv*, jclass,
        jlong nativeBuilder, jint priority) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->priority((uint8_t) priority);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderCulling(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->culling(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderCastShadows(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->castShadows(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderReceiveShadows(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->receiveShadows(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderScreenSpaceContactShadows(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->screenSpaceContactShadows(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSkinning(JNIEnv*, jclass,
        jlong nativeBuilder, jint boneCount) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->skinning((size_t)boneCount);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderSkinningBones(JNIEnv* env, jclass,
        jlong nativeBuilder, jint boneCount, jobject bones, jint remaining) {
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
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nBuilderMorphing(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    RenderableManager::Builder *builder = (RenderableManager::Builder *) nativeBuilder;
    builder->morphing(enabled);
}


extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nSetBonesAsMatrices(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jobject matrices, jint remaining,
        jint boneCount, jint offset) {
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
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nSetBonesAsQuaternions(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint i, jobject quaternions, jint remaining,
        jint boneCount, jint offset) {
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
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetMorphWeights(JNIEnv* env, jclass,
        jlong nativeRenderableManager, jint instance, jfloatArray weights) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    jfloat* vec = env->GetFloatArrayElements(weights, NULL);
    math::float4 floatvec(vec[0], vec[1], vec[2], vec[3]);
    env->ReleaseFloatArrayElements(weights, vec, JNI_ABORT);
    rm->setMorphWeights((RenderableManager::Instance)instance, floatvec);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetAxisAlignedBoundingBox(JNIEnv*,
        jclass, jlong nativeRenderableManager, jint i, jfloat cx, jfloat cy, jfloat cz,
        jfloat ex, jfloat ey, jfloat ez) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setAxisAlignedBoundingBox((RenderableManager::Instance) i, {{cx, cy, cz},
                                                                    {ex, ey, ez}});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetLayerMask(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint select, jint value) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setLayerMask((RenderableManager::Instance) i, (uint8_t) select, (uint8_t) value);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetPriority(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint priority) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setPriority((RenderableManager::Instance) i, (uint8_t) priority);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetCulling(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setCulling((RenderableManager::Instance) i, enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetCastShadows(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setCastShadows((RenderableManager::Instance) i, enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetReceiveShadows(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setReceiveShadows((RenderableManager::Instance) i, enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetScreenSpaceContactShadows(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jboolean enabled) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setScreenSpaceContactShadows((RenderableManager::Instance) i, enabled);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nIsShadowCaster(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return (jboolean) rm->isShadowCaster((RenderableManager::Instance) i);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_RenderableManager_nIsShadowReceiver(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return (jboolean) rm->isShadowReceiver((RenderableManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nGetAxisAlignedBoundingBox(JNIEnv* env,
        jclass, jlong nativeRenderableManager, jint i, jfloatArray center_,
        jfloatArray halfExtent_) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    jfloat *center = env->GetFloatArrayElements(center_, NULL);
    jfloat *halfExtent = env->GetFloatArrayElements(halfExtent_, NULL);
    Box const &aabb = rm->getAxisAlignedBoundingBox((RenderableManager::Instance) i);
    *reinterpret_cast<filament::math::float3 *>(center) = aabb.center;
    *reinterpret_cast<filament::math::float3 *>(halfExtent) = aabb.halfExtent;
    env->ReleaseFloatArrayElements(center_, center, 0);
    env->ReleaseFloatArrayElements(halfExtent_, halfExtent, 0);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetPrimitiveCount(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return (jint) rm->getPrimitiveCount((RenderableManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetMaterialInstanceAt(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jlong nativeMaterialInstance) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    const MaterialInstance *materialInstance = (const MaterialInstance *) nativeMaterialInstance;
    rm->setMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
            materialInstance);
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_RenderableManager_nGetMaterialInstanceAt(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    return (long) rm->getMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
}

extern "C" JNIEXPORT long JNICALL
Java_com_google_android_filament_RenderableManager_nGetMaterialAt(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    MaterialInstance *mi = rm->getMaterialInstanceAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
    return (long) mi->getMaterial();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetGeometryAt__JIIIJJII(JNIEnv*,
        jclass, jlong nativeRenderableManager, jint i, jint primitiveIndex, jint primitiveType,
        jlong nativeVertexBuffer, jlong nativeIndexBuffer, jint offset, jint count) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
    rm->setGeometryAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
            (RenderableManager::PrimitiveType) primitiveType, vertexBuffer, indexBuffer,
            (size_t) offset, (size_t) count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetGeometryAt__JIIIII(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jint primitiveType, jint offset,
        jint count) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setGeometryAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
            (RenderableManager::PrimitiveType) primitiveType, (size_t) offset, (size_t) count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderableManager_nSetBlendOrderAt(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex, jint blendOrder) {
    RenderableManager *rm = (RenderableManager *) nativeRenderableManager;
    rm->setBlendOrderAt((RenderableManager::Instance) i, (size_t) primitiveIndex,
            (uint16_t) blendOrder);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderableManager_nGetEnabledAttributesAt(JNIEnv*, jclass,
        jlong nativeRenderableManager, jint i, jint primitiveIndex) {
    RenderableManager const *rm = (RenderableManager const *) nativeRenderableManager;
    AttributeBitset enabled = rm->getEnabledAttributesAt((RenderableManager::Instance) i, (size_t) primitiveIndex);
    return enabled.getValue();
}
