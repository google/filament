/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <utils/debug.h>

#include "common/NioUtils.h"

#include "MaterialKey.h"

using namespace filament;
using namespace filament::gltfio;
using namespace utils;

class JavaMaterialProvider : public MaterialProvider {
    JNIEnv* const mEnv;
    const jobject mJavaProvider;
    mutable Material** mPreviousMaterials = nullptr;

    jclass mMaterialKeyClass;

    jmethodID mMaterialKeyConstructor;
    jmethodID mCreateMaterialInstance;
    jmethodID mGetMaterial;
    jmethodID mGetMaterials;
    jmethodID mNeedsDummyData;
    jmethodID mDestroyMaterials;
    jmethodID mMaterialInstanceGetNativeObject;
    jmethodID mMaterialGetNativeObject;

public:
    JavaMaterialProvider(JNIEnv* env, jobject provider) : mEnv(env),
            mJavaProvider(env->NewGlobalRef(provider)) {
        mMaterialKeyClass = env->FindClass(JAVA_MATERIAL_KEY);
        mMaterialKeyClass = (jclass) env->NewGlobalRef(mMaterialKeyClass);
        assert_invariant(mMaterialKeyClass);

        mMaterialKeyConstructor = env->GetMethodID(mMaterialKeyClass, "<init>", "()V");
        assert_invariant(mMaterialKeyConstructor);

        jclass materialInstanceClass = env->FindClass("com/google/android/filament/MaterialInstance");
        mMaterialInstanceGetNativeObject = env->GetMethodID(materialInstanceClass, "getNativeObject", "()J");
        assert_invariant(mMaterialInstanceGetNativeObject);

        jclass materialClass = env->FindClass("com/google/android/filament/Material");
        mMaterialGetNativeObject = env->GetMethodID(materialClass, "getNativeObject", "()J");
        assert_invariant(mMaterialGetNativeObject);

        jclass providerClass = env->GetObjectClass(provider);

        mCreateMaterialInstance = env->GetMethodID(providerClass, "createMaterialInstance",
                "(L" JAVA_MATERIAL_KEY ";[ILjava/lang/String;Ljava/lang/String;)Lcom/google/android/filament/MaterialInstance;");
        assert_invariant(mCreateMaterialInstance);

        mGetMaterial = env->GetMethodID(providerClass, "getMaterial",
                "(L" JAVA_MATERIAL_KEY ";[ILjava/lang/String;)Lcom/google/android/filament/Material;");
        assert_invariant(mGetMaterial);

        mGetMaterials = env->GetMethodID(providerClass, "getMaterials",
                "()[Lcom/google/android/filament/Material;");
        assert_invariant(mGetMaterials);

        mNeedsDummyData = env->GetMethodID(providerClass, "needsDummyData","(I)Z");
        assert_invariant(mNeedsDummyData);

        mDestroyMaterials = env->GetMethodID(providerClass, "destroyMaterials", "()V");
        assert_invariant(mDestroyMaterials);
    }

    ~JavaMaterialProvider() override {
        mEnv->DeleteGlobalRef(mMaterialKeyClass);
        mEnv->DeleteGlobalRef(mJavaProvider);
        delete mPreviousMaterials;
    }

    MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap, const char* label, const char* extras) override {
        // Create a Java object for the material key and copy the native fields into it.
        jobject javaKey = mEnv->NewObject(mMaterialKeyClass, mMaterialKeyConstructor);

        auto& helper = MaterialKeyHelper::get();
        helper.copy(mEnv, javaKey, *config);

        // Convert the optional label into a Java string.
        jstring stringLabel = label ? mEnv->NewStringUTF(label) : nullptr;

        // Convert the optional extras into a Java string.
        jstring stringExtras = extras ? mEnv->NewStringUTF(extras) : nullptr;

        // Allocate space for the output argument.
        jintArray uvMapArray = mEnv->NewIntArray(uvmap->size());

        // Call the Java-based material provider.
        jobject materialInstance = mEnv->CallObjectMethod(mJavaProvider, mCreateMaterialInstance,
                javaKey, uvMapArray, stringLabel, stringExtras);

        // Copy the UvMap results from the JVM array into the native array.
        if (uvmap) {
            jint* elements = mEnv->GetIntArrayElements(uvMapArray, nullptr);
            for (size_t i = 0; i < uvmap->size(); i++) {
                (*uvmap)[i] = (UvSet) elements[i];
            }
            mEnv->ReleaseIntArrayElements(uvMapArray, elements, JNI_ABORT);
        }

        // The config parameter is an in-out parameter so we need to copy the results from Java.
        helper.copy(mEnv, *config, javaKey);

        mEnv->DeleteLocalRef(javaKey);
        mEnv->DeleteLocalRef(uvMapArray);

        if (stringLabel) {
            mEnv->DeleteLocalRef(stringLabel);
        }

        if (stringExtras) {
            mEnv->DeleteLocalRef(stringExtras);
        }

        if (materialInstance == nullptr) {
            return nullptr;
        }

        return (MaterialInstance*) mEnv->CallLongMethod(materialInstance, mMaterialInstanceGetNativeObject);
    }

    Material* getMaterial(MaterialKey* config, UvMap* uvmap, const char* label) override {
        // Create a Java object for the material key and copy the native fields into it.
        jobject javaKey = mEnv->NewObject(mMaterialKeyClass, mMaterialKeyConstructor);

        auto& helper = MaterialKeyHelper::get();
        helper.copy(mEnv, javaKey, *config);

        // Convert the optional label into a Java string.
        jstring stringLabel = label ? mEnv->NewStringUTF(label) : nullptr;

        // Allocate space for the output argument.
        jintArray uvMapArray = mEnv->NewIntArray(uvmap->size());

        // Call the Java-based material provider.
        jobject material = mEnv->CallObjectMethod(mJavaProvider, mGetMaterial,
                javaKey, uvMapArray, stringLabel);

        // Copy the UvMap results from the JVM array into the native array.
        if (uvmap) {
            jint* elements = mEnv->GetIntArrayElements(uvMapArray, nullptr);
            for (size_t i = 0; i < uvmap->size(); i++) {
                (*uvmap)[i] = (UvSet) elements[i];
            }
            mEnv->ReleaseIntArrayElements(uvMapArray, elements, JNI_ABORT);
        }

        // The config parameter is an in-out parameter so we need to copy the results from Java.
        helper.copy(mEnv, *config, javaKey);

        mEnv->DeleteLocalRef(javaKey);
        mEnv->DeleteLocalRef(uvMapArray);

        if (stringLabel) {
            mEnv->DeleteLocalRef(stringLabel);
        }

        if (material == nullptr) {
            return nullptr;
        }

        return (Material*) mEnv->CallLongMethod(material, mMaterialGetNativeObject);
    }

    const Material* const* getMaterials() const noexcept override {
        jobjectArray javaMaterials = (jobjectArray) mEnv->CallObjectMethod(mJavaProvider, mGetMaterials);

        const size_t count = mEnv->GetArrayLength(javaMaterials);

        delete mPreviousMaterials;
        using MaterialPointer = Material*;
        mPreviousMaterials = new MaterialPointer[count];

        // Call "getNativeObject" on each material in order to pass them up into the native layer.
        for (size_t i = 0; i < count; ++i) {
            jobject javaMaterial = mEnv->GetObjectArrayElement(javaMaterials, i);
            jlong matPointer = mEnv->CallLongMethod(javaMaterial, mMaterialGetNativeObject);
            mPreviousMaterials[i] = (Material*) matPointer;
        }

        return mPreviousMaterials;
    }

    size_t getMaterialsCount() const noexcept override {
        jobjectArray javaMaterials = (jobjectArray) mEnv->CallObjectMethod(mJavaProvider, mGetMaterials);
        return mEnv->GetArrayLength(javaMaterials);
    }

    void destroyMaterials() override {
        mEnv->CallObjectMethod(mJavaProvider, mDestroyMaterials);
    }

    bool needsDummyData(VertexAttribute attrib) const noexcept override {
        return mEnv->CallBooleanMethod(mJavaProvider, mNeedsDummyData, (int) attrib);
    }
};

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAssetLoader(JNIEnv* env, jclass,
        jlong nativeEngine, jobject provider, jlong nativeEntities) {
    Engine* engine = (Engine*) nativeEngine;
    MaterialProvider* materialProvider = nullptr;

    // First check for a fast path that passes a native MaterialProvider into the loader.
    // This drastically reduces the number of JNI calls while the asset is being loaded.
    jclass klass = env->GetObjectClass(provider);
    jmethodID getNativeObject = env->GetMethodID(klass, "getNativeObject", "()J");
    if (getNativeObject) {
        materialProvider = (MaterialProvider*) env->CallLongMethod(provider, getNativeObject);
    } else {
        env->ExceptionClear();
    }

    if (materialProvider == nullptr) {
        materialProvider = new JavaMaterialProvider(env, provider);
    }

    EntityManager* entities = (EntityManager*) nativeEntities;
    NameComponentManager* names = new NameComponentManager(*entities);
    return (jlong) AssetLoader::create({engine, materialProvider, names, entities});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAssetLoader(JNIEnv*, jclass,
        jlong nativeLoader) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    NameComponentManager* names = loader->getNames();
    AssetLoader::destroy(&loader);
    delete names;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateAsset(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    return (jlong) loader->createAsset((const uint8_t *) buffer.getData(),
            buffer.getSize());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateInstancedAsset(JNIEnv* env, jclass,
        jlong nativeLoader, jobject javaBuffer, jint remaining, jlongArray instances) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    AutoBuffer buffer(env, javaBuffer, remaining);
    jsize numInstances = env->GetArrayLength(instances);
    using Handle = FilamentInstance*;
    Handle* ptrInstances = new Handle[numInstances];
    jlong asset = (jlong) loader->createInstancedAsset((const uint8_t *) buffer.getData(),
            buffer.getSize(), ptrInstances, numInstances);
    if (asset) {
        jlong* longInstances = env->GetLongArrayElements(instances, nullptr);
        for (jsize i = 0; i < numInstances; i++) {
            longInstances[i] = (jlong) ptrInstances[i];
        }
        env->ReleaseLongArrayElements(instances, longInstances, 0);
    }
    delete[] ptrInstances;
    return asset;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nCreateInstance(JNIEnv* env, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    FilamentAsset* primary = (FilamentAsset*) nativeAsset;
    return (jlong) loader->createInstance(primary);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nEnableDiagnostics(JNIEnv*, jclass,
        jlong nativeLoader, jboolean enable) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    loader->enableDiagnostics(enable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_gltfio_AssetLoader_nDestroyAsset(JNIEnv*, jclass,
        jlong nativeLoader, jlong nativeAsset) {
    AssetLoader* loader = (AssetLoader*) nativeLoader;
    FilamentAsset* asset = (FilamentAsset*) nativeAsset;
    loader->destroyAsset(asset);
}
