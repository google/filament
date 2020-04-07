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

#include <filamat/MaterialBuilder.h>

using namespace filament;
using namespace filamat;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderInit(JNIEnv*, jclass) {
    MaterialBuilder::init();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderShutdown(JNIEnv*, jclass) {
    MaterialBuilder::shutdown();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nCreateMaterialBuilder(JNIEnv*, jclass) {
    return (jlong) new MaterialBuilder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nDestroyMaterialBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    return (jlong) new Package(builder->build());
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nGetPackageBytes(JNIEnv* env, jclass,
        jlong nativePackage) {
    auto package = (Package*) nativePackage;
    auto size = jsize(package->getSize());
    jbyteArray ret = env->NewByteArray(size);
    auto data = (jbyte*) package->getData();
    env->SetByteArrayRegion(ret, 0, size, data);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nGetPackageIsValid(JNIEnv*, jclass,
        jlong nativePackage) {
    auto* package = (Package*) nativePackage;
    return jboolean(package->isValid());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nDestroyPackage(JNIEnv*, jclass,
        jlong nativePackage) {
    Package* package = (Package*) nativePackage;
    delete package;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderName(JNIEnv* env,
        jclass, jlong nativeBuilder, jstring name_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* name = env->GetStringUTFChars(name_, nullptr);
    builder->name(name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderMaterialDomain(JNIEnv* env,
        jclass, jlong nativeBuilder, jint domain) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->materialDomain((MaterialBuilder::MaterialDomain) domain);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderShading(JNIEnv*,
        jclass, jlong nativeBuilder, jint shading) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->shading((MaterialBuilder::Shading) shading);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderInterpolation(JNIEnv*,
        jclass, jlong nativeBuilder, jint interpolation) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->interpolation((MaterialBuilder::Interpolation) interpolation);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderUniformParameter(
        JNIEnv* env, jclass, jlong nativeBuilder, jint uniformType, jstring name_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* name = env->GetStringUTFChars(name_, nullptr);
    builder->parameter((MaterialBuilder::UniformType) uniformType, name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderUniformParameterArray(
        JNIEnv* env, jclass, jlong nativeBuilder, jint uniformType, jint size, jstring name_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* name = env->GetStringUTFChars(name_, nullptr);
    builder->parameter((MaterialBuilder::UniformType) uniformType, (size_t) size, name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderSamplerParameter(
        JNIEnv* env, jclass, jlong nativeBuilder, jint samplerType, jint format,
        jint precision, jstring name_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* name = env->GetStringUTFChars(name_, nullptr);
    builder->parameter((MaterialBuilder::SamplerType) samplerType,
            (MaterialBuilder::SamplerFormat) format, (MaterialBuilder::SamplerPrecision) precision,
            name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderVariable(
        JNIEnv* env, jclass, jlong nativeBuilder, jint variable, jstring name_) {
    const char* name = env->GetStringUTFChars(name_, nullptr);
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->variable((MaterialBuilder::Variable) variable, name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderRequire(JNIEnv*,
        jclass, jlong nativeBuilder, jint attribute) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->require((VertexAttribute) attribute);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderMaterial(JNIEnv* env,
        jclass, jlong nativeBuilder, jstring code_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* code = env->GetStringUTFChars(code_, nullptr);
    builder->material(code);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderMaterialVertex(JNIEnv* env,
        jclass, jlong nativeBuilder, jstring code_) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    const char* code = env->GetStringUTFChars(code_, nullptr);
    builder->materialVertex(code);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderBlending(JNIEnv*,
        jclass, jlong nativeBuilder, jint mode) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->blending((MaterialBuilder::BlendingMode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderPostLightingBlending(
        JNIEnv*, jclass, jlong nativeBuilder, jint mode) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->postLightingBlending((MaterialBuilder::BlendingMode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderVertexDomain(JNIEnv*,
        jclass, jlong nativeBuilder, jint vertexDomain) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->vertexDomain((MaterialBuilder::VertexDomain) vertexDomain);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderCulling(JNIEnv*,
        jclass, jlong nativeBuilder, jint mode) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->culling((MaterialBuilder::CullingMode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderColorWrite(JNIEnv*,
        jclass, jlong nativeBuilder, jboolean enable) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->colorWrite(enable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderDepthWrite(JNIEnv*,
        jclass, jlong nativeBuilder, jboolean depthWrite) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->depthWrite(depthWrite);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderDepthCulling(JNIEnv*,
        jclass, jlong nativeBuilder, jboolean depthCulling) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->depthCulling(depthCulling);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderDoubleSided(JNIEnv*,
        jclass, jlong nativeBuilder, jboolean doubleSided) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->doubleSided(doubleSided);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderMaskThreshold(JNIEnv*,
        jclass, jlong nativeBuilder, jfloat maskThreshold) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->maskThreshold(maskThreshold);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderShadowMultiplier(
        JNIEnv*, jclass, jlong nativeBuilder, jboolean shadowMultiplier) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->shadowMultiplier(shadowMultiplier);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderSpecularAntiAliasing(
        JNIEnv*, jclass, jlong nativeBuilder, jboolean specularAntiAliasing) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->specularAntiAliasing(specularAntiAliasing);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderSpecularAntiAliasingVariance(
        JNIEnv*, jclass, jlong nativeBuilder, jfloat variance) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->specularAntiAliasingVariance(variance);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderSpecularAntiAliasingThreshold(
        JNIEnv*, jclass, jlong nativeBuilder, jfloat threshold) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->specularAntiAliasingThreshold(threshold);
}


extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderClearCoatIorChange(
        JNIEnv*, jclass, jlong nativeBuilder, jboolean clearCoatIorChange) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->clearCoatIorChange(clearCoatIorChange);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderFlipUV(JNIEnv*,
        jclass, jlong nativeBuilder, jboolean flipUV) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->flipUV(flipUV);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderMultiBounceAmbientOcclusion(
        JNIEnv*, jclass, jlong nativeBuilder, jboolean multiBounceAO) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->multiBounceAmbientOcclusion(multiBounceAO);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderSpecularAmbientOcclusion(
        JNIEnv*, jclass, jlong nativeBuilder, jint specularAO) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->specularAmbientOcclusion((MaterialBuilder::SpecularAmbientOcclusion) specularAO);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderRefractionMode(JNIEnv* env,
        jclass, jlong nativeBuilder, jint mode) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->refractionMode((MaterialBuilder::RefractionMode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderRefractionType(JNIEnv* env,
        jclass, jlong nativeBuilder, jint type) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->refractionType((MaterialBuilder::RefractionType) type);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderTransparencyMode(
        JNIEnv* env, jclass, jlong nativeBuilder, jint mode) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->transparencyMode((MaterialBuilder::TransparencyMode) mode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderPlatform(JNIEnv*,
        jclass, jlong nativeBuilder, jint platform) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->platform((MaterialBuilder::Platform) platform);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderTargetApi(JNIEnv*,
        jclass, jlong nativeBuilder, jint targetApi) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->targetApi((MaterialBuilder::TargetApi) targetApi);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderOptimization(JNIEnv*,
        jclass, jlong nativeBuilder, jint optimization) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->optimization((MaterialBuilder::Optimization) optimization);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_filamat_MaterialBuilder_nMaterialBuilderVariantFilter(JNIEnv*,
        jclass, jlong nativeBuilder, jbyte variantFilter) {
    auto builder = (MaterialBuilder*) nativeBuilder;
    builder->variantFilter((uint8_t) variantFilter);
}
