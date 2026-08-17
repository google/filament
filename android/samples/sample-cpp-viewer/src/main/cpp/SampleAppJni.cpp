#include <private/backend/VirtualMachineEnv.h>
#include <samples/SampleDispatcher.h>

#include <jni.h>

// This is called automatically when the shared library is loaded by the JVM.
// It is necessary because Filament's Android OpenGL/GLES platform (PlatformEGLAndroid)
// and driver threads rely on VirtualMachineEnv to obtain JNIEnv/JavaVM references
// for EGL surface and lifecycle management. Without this call, VirtualMachineEnv::get()
// will fail an assertion ('sVirtualMachine') on startup.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_cppviewer_SampleAppDispatcher_nCreateSampleApp(JNIEnv* env, jclass clazz,
        jstring sampleName, jlong nativeDm, jlong nativeLoader) {
    
    const char* nameStr = env->GetStringUTFChars(sampleName, nullptr);
    utils::CString name(nameStr);
    env->ReleaseStringUTFChars(sampleName, nameStr);
    
    SampleConfig config;
    config.title = utils::CString(name.c_str());
    
    auto* dm = reinterpret_cast<filament::app::DisplayManager*>(nativeDm);
    auto* loader = reinterpret_cast<filament::app::AssetLoader*>(nativeLoader);
    
    auto app = dispatchSample(name, config, dm, loader);
    if (app) {
        // Let's init here before handing the app over to the app.
        app->init();
        
        return reinterpret_cast<jlong>(app.release());
    }
    return 0;
}

#include "AndroidAssetLoader.h"

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_cppviewer_SampleAppDispatcher_createAssetLoader(JNIEnv* env, jclass clazz,
        jobject assetManager) {
    AAssetManager* nativeAssetManager = AAssetManager_fromJava(env, assetManager);
    auto* loader = new AndroidAssetLoader(nativeAssetManager);
    return reinterpret_cast<jlong>(loader);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleAppDispatcher_destroyAssetLoader(JNIEnv* env, jclass clazz,
        jlong nativeLoader) {
    auto* loader = reinterpret_cast<AndroidAssetLoader*>(nativeLoader);
    delete loader;
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_google_android_filament_cppviewer_SampleAppDispatcher_getSampleNames(JNIEnv* env, jclass clazz) {
    utils::FixedCapacityVector<utils::CString> names = getSampleNames();
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(names.size(), stringClass, nullptr);
    for (size_t i = 0; i < names.size(); i++) {
        jstring str = env->NewStringUTF(names[i].c_str());
        env->SetObjectArrayElement(array, i, str);
        env->DeleteLocalRef(str);
    }
    return array;
}
