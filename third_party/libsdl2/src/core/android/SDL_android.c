/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"
#include "SDL_stdinc.h"
#include "SDL_assert.h"
#include "SDL_hints.h"
#include "SDL_log.h"
#include "SDL_main.h"

#ifdef __ANDROID__

#include "SDL_system.h"
#include "SDL_android.h"

#include "keyinfotable.h"

#include "../../events/SDL_events_c.h"
#include "../../video/android/SDL_androidkeyboard.h"
#include "../../video/android/SDL_androidmouse.h"
#include "../../video/android/SDL_androidtouch.h"
#include "../../video/android/SDL_androidvideo.h"
#include "../../video/android/SDL_androidwindow.h"
#include "../../joystick/android/SDL_sysjoystick_c.h"
#include "../../haptic/android/SDL_syshaptic_c.h"

#include <android/log.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
/* #define LOG_TAG "SDL_android" */
/* #define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__) */
/* #define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__) */
#define LOGI(...) do {} while (0)
#define LOGE(...) do {} while (0)


#define SDL_JAVA_PREFIX                                 org_libsdl_app
#define CONCAT1(prefix, class, function)                CONCAT2(prefix, class, function)
#define CONCAT2(prefix, class, function)                Java_ ## prefix ## _ ## class ## _ ## function
#define SDL_JAVA_INTERFACE(function)                    CONCAT1(SDL_JAVA_PREFIX, SDLActivity, function)
#define SDL_JAVA_AUDIO_INTERFACE(function)              CONCAT1(SDL_JAVA_PREFIX, SDLAudioManager, function)
#define SDL_JAVA_CONTROLLER_INTERFACE(function)         CONCAT1(SDL_JAVA_PREFIX, SDLControllerManager, function)
#define SDL_JAVA_INTERFACE_INPUT_CONNECTION(function)   CONCAT1(SDL_JAVA_PREFIX, SDLInputConnection, function)


/* Java class SDLActivity */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeSetupJNI)(
        JNIEnv* mEnv, jclass cls);

JNIEXPORT int JNICALL SDL_JAVA_INTERFACE(nativeRunMain)(
        JNIEnv* env, jclass cls,
        jstring library, jstring function, jobject array);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeDropFile)(
        JNIEnv* env, jclass jcls,
        jstring filename);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeResize)(
        JNIEnv* env, jclass jcls,
        jint width, jint height, jint format, jfloat rate);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeSurfaceChanged)(
        JNIEnv* env, jclass jcls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeSurfaceDestroyed)(
        JNIEnv* env, jclass jcls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyDown)(
        JNIEnv* env, jclass jcls,
        jint keycode);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyUp)(
        JNIEnv* env, jclass jcls,
        jint keycode);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyboardFocusLost)(
        JNIEnv* env, jclass jcls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeTouch)(
        JNIEnv* env, jclass jcls,
        jint touch_device_id_in, jint pointer_finger_id_in,
        jint action, jfloat x, jfloat y, jfloat p);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeMouse)(
        JNIEnv* env, jclass jcls,
        jint button, jint action, jfloat x, jfloat y);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeAccel)(
        JNIEnv* env, jclass jcls,
        jfloat x, jfloat y, jfloat z);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeClipboardChanged)(
        JNIEnv* env, jclass jcls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeLowMemory)(
        JNIEnv* env, jclass cls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeQuit)(
        JNIEnv* env, jclass cls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativePause)(
        JNIEnv* env, jclass cls);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeResume)(
        JNIEnv* env, jclass cls);

JNIEXPORT jstring JNICALL SDL_JAVA_INTERFACE(nativeGetHint)(
        JNIEnv* env, jclass cls,
        jstring name);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeSetenv)(
        JNIEnv* env, jclass cls,
        jstring name, jstring value);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeEnvironmentVariablesSet)(
        JNIEnv* env, jclass cls);

/* Java class SDLInputConnection */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE_INPUT_CONNECTION(nativeCommitText)(
        JNIEnv* env, jclass cls,
        jstring text, jint newCursorPosition);

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE_INPUT_CONNECTION(nativeSetComposingText)(
        JNIEnv* env, jclass cls,
        jstring text, jint newCursorPosition);

/* Java class SDLAudioManager */
JNIEXPORT void JNICALL SDL_JAVA_AUDIO_INTERFACE(nativeSetupJNI)(
        JNIEnv *env, jclass jcls);

/* Java class SDLControllerManager */
JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeSetupJNI)(
        JNIEnv *env, jclass jcls);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativePadDown)(
        JNIEnv* env, jclass jcls,
        jint device_id, jint keycode);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativePadUp)(
        JNIEnv* env, jclass jcls,
        jint device_id, jint keycode);

JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativeJoy)(
        JNIEnv* env, jclass jcls,
        jint device_id, jint axis, jfloat value);

JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativeHat)(
        JNIEnv* env, jclass jcls,
        jint device_id, jint hat_id, jint x, jint y);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeAddJoystick)(
        JNIEnv* env, jclass jcls,
        jint device_id, jstring device_name, jstring device_desc, jint is_accelerometer,
        jint nbuttons, jint naxes, jint nhats, jint nballs);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeRemoveJoystick)(
        JNIEnv* env, jclass jcls,
        jint device_id);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeAddHaptic)(
        JNIEnv* env, jclass jcls,
        jint device_id, jstring device_name);

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeRemoveHaptic)(
        JNIEnv* env, jclass jcls,
        jint device_id);



/* Uncomment this to log messages entering and exiting methods in this file */
/* #define DEBUG_JNI */

static void Android_JNI_ThreadDestroyed(void*);

/*******************************************************************************
 This file links the Java side of Android with libsdl
*******************************************************************************/
#include <jni.h>


/*******************************************************************************
                               Globals
*******************************************************************************/
static pthread_key_t mThreadKey;
static JavaVM* mJavaVM;

/* Main activity */
static jclass mActivityClass;

/* method signatures */
static jmethodID midGetNativeSurface;
static jmethodID midSetActivityTitle;
static jmethodID midSetWindowStyle;
static jmethodID midSetOrientation;
static jmethodID midGetContext;
static jmethodID midIsAndroidTV;
static jmethodID midInputGetInputDeviceIds;
static jmethodID midSendMessage;
static jmethodID midShowTextInput;
static jmethodID midIsScreenKeyboardShown;
static jmethodID midClipboardSetText;
static jmethodID midClipboardGetText;
static jmethodID midClipboardHasText;
static jmethodID midOpenAPKExpansionInputStream;
static jmethodID midGetManifestEnvironmentVariables;
static jmethodID midGetDisplayDPI;

/* audio manager */
static jclass mAudioManagerClass;

/* method signatures */
static jmethodID midAudioOpen;
static jmethodID midAudioWriteShortBuffer;
static jmethodID midAudioWriteByteBuffer;
static jmethodID midAudioClose;
static jmethodID midCaptureOpen;
static jmethodID midCaptureReadShortBuffer;
static jmethodID midCaptureReadByteBuffer;
static jmethodID midCaptureClose;

/* controller manager */
static jclass mControllerManagerClass;

/* method signatures */
static jmethodID midPollInputDevices;
static jmethodID midPollHapticDevices;
static jmethodID midHapticRun;

/* static fields */
static jfieldID fidSeparateMouseAndTouch;

/* Accelerometer data storage */
static float fLastAccelerometer[3];
static SDL_bool bHasNewData;

static SDL_bool bHasEnvironmentVariables = SDL_FALSE;

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/

/* Library init */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    mJavaVM = vm;
    LOGI("JNI_OnLoad called");
    if ((*mJavaVM)->GetEnv(mJavaVM, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }
    /*
     * Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
     * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
     */
    if (pthread_key_create(&mThreadKey, Android_JNI_ThreadDestroyed) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "Error initializing pthread key");
    }
    Android_JNI_SetupThread();

    return JNI_VERSION_1_4;
}

void checkJNIReady()
{
    if (!mActivityClass || !mAudioManagerClass || !mControllerManagerClass) {
        // We aren't fully initialized, let's just return.
        return;
    }

    SDL_SetMainReady();    
}

/* Activity initialization -- called before SDL_main() to initialize JNI bindings */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeSetupJNI)(JNIEnv* mEnv, jclass cls)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "nativeSetupJNI()");

    Android_JNI_SetupThread();

    mActivityClass = (jclass)((*mEnv)->NewGlobalRef(mEnv, cls));

    midGetNativeSurface = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "getNativeSurface","()Landroid/view/Surface;");
    midSetActivityTitle = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "setActivityTitle","(Ljava/lang/String;)Z");
    midSetWindowStyle = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "setWindowStyle","(Z)V");
    midSetOrientation = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "setOrientation","(IIZLjava/lang/String;)V");
    midGetContext = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "getContext","()Landroid/content/Context;");
    midIsAndroidTV = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "isAndroidTV","()Z");
    midInputGetInputDeviceIds = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "inputGetInputDeviceIds", "(I)[I");
    midSendMessage = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "sendMessage", "(II)Z");
    midShowTextInput =  (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "showTextInput", "(IIII)Z");
    midIsScreenKeyboardShown = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "isScreenKeyboardShown","()Z");
    midClipboardSetText = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "clipboardSetText", "(Ljava/lang/String;)V");
    midClipboardGetText = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "clipboardGetText", "()Ljava/lang/String;");
    midClipboardHasText = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "clipboardHasText", "()Z");
    midOpenAPKExpansionInputStream = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "openAPKExpansionInputStream", "(Ljava/lang/String;)Ljava/io/InputStream;");

    midGetManifestEnvironmentVariables = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass,
                                "getManifestEnvironmentVariables", "()Z");

    midGetDisplayDPI = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass, "getDisplayDPI", "()Landroid/util/DisplayMetrics;");
    midGetDisplayDPI = (*mEnv)->GetStaticMethodID(mEnv, mActivityClass, "getDisplayDPI", "()Landroid/util/DisplayMetrics;");

    if (!midGetNativeSurface ||
       !midSetActivityTitle || !midSetWindowStyle || !midSetOrientation || !midGetContext || !midIsAndroidTV || !midInputGetInputDeviceIds ||
       !midSendMessage || !midShowTextInput || !midIsScreenKeyboardShown ||
       !midClipboardSetText || !midClipboardGetText || !midClipboardHasText ||
       !midOpenAPKExpansionInputStream || !midGetManifestEnvironmentVariables|| !midGetDisplayDPI) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "Missing some Java callbacks, do you have the latest version of SDLActivity.java?");
    }

    fidSeparateMouseAndTouch = (*mEnv)->GetStaticFieldID(mEnv, mActivityClass, "mSeparateMouseAndTouch", "Z");

    if (!fidSeparateMouseAndTouch) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "Missing some Java static fields, do you have the latest version of SDLActivity.java?");
    }

    checkJNIReady();
}

/* Audio initialization -- called before SDL_main() to initialize JNI bindings */
JNIEXPORT void JNICALL SDL_JAVA_AUDIO_INTERFACE(nativeSetupJNI)(JNIEnv* mEnv, jclass cls)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "AUDIO nativeSetupJNI()");

    Android_JNI_SetupThread();

    mAudioManagerClass = (jclass)((*mEnv)->NewGlobalRef(mEnv, cls));

    midAudioOpen = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "audioOpen", "(IZZI)I");
    midAudioWriteShortBuffer = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "audioWriteShortBuffer", "([S)V");
    midAudioWriteByteBuffer = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "audioWriteByteBuffer", "([B)V");
    midAudioClose = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "audioClose", "()V");
    midCaptureOpen = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "captureOpen", "(IZZI)I");
    midCaptureReadShortBuffer = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "captureReadShortBuffer", "([SZ)I");
    midCaptureReadByteBuffer = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "captureReadByteBuffer", "([BZ)I");
    midCaptureClose = (*mEnv)->GetStaticMethodID(mEnv, mAudioManagerClass,
                                "captureClose", "()V");

    if (!midAudioOpen || !midAudioWriteShortBuffer || !midAudioWriteByteBuffer || !midAudioClose ||
       !midCaptureOpen || !midCaptureReadShortBuffer || !midCaptureReadByteBuffer || !midCaptureClose) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "Missing some Java callbacks, do you have the latest version of SDLAudioManager.java?");
    }

    checkJNIReady();
}

/* Controller initialization -- called before SDL_main() to initialize JNI bindings */
JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeSetupJNI)(JNIEnv* mEnv, jclass cls)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "CONTROLLER nativeSetupJNI()");

    Android_JNI_SetupThread();

    mControllerManagerClass = (jclass)((*mEnv)->NewGlobalRef(mEnv, cls));

    midPollInputDevices = (*mEnv)->GetStaticMethodID(mEnv, mControllerManagerClass,
                                "pollInputDevices", "()V");
    midPollHapticDevices = (*mEnv)->GetStaticMethodID(mEnv, mControllerManagerClass,
                                "pollHapticDevices", "()V");
    midHapticRun = (*mEnv)->GetStaticMethodID(mEnv, mControllerManagerClass,
                                "hapticRun", "(II)V");

    if (!midPollInputDevices || !midPollHapticDevices || !midHapticRun) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "Missing some Java callbacks, do you have the latest version of SDLControllerManager.java?");
    }

    checkJNIReady();
}

/* SDL main function prototype */
typedef int (*SDL_main_func)(int argc, char *argv[]);

/* Start up the SDL app */
JNIEXPORT int JNICALL SDL_JAVA_INTERFACE(nativeRunMain)(JNIEnv* env, jclass cls, jstring library, jstring function, jobject array)
{
    int status = -1;
    const char *library_file;
    void *library_handle;

    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "nativeRunMain()");

    library_file = (*env)->GetStringUTFChars(env, library, NULL);
    library_handle = dlopen(library_file, RTLD_GLOBAL);
    if (library_handle) {
        const char *function_name;
        SDL_main_func SDL_main;

        function_name = (*env)->GetStringUTFChars(env, function, NULL);
        SDL_main = (SDL_main_func)dlsym(library_handle, function_name);
        if (SDL_main) {
            int i;
            int argc;
            int len;
            char **argv;

            /* Prepare the arguments. */
            len = (*env)->GetArrayLength(env, array);
            argv = SDL_stack_alloc(char*, 1 + len + 1);
            argc = 0;
            /* Use the name "app_process" so PHYSFS_platformCalcBaseDir() works.
               https://bitbucket.org/MartinFelis/love-android-sdl2/issue/23/release-build-crash-on-start
             */
            argv[argc++] = SDL_strdup("app_process");
            for (i = 0; i < len; ++i) {
                const char* utf;
                char* arg = NULL;
                jstring string = (*env)->GetObjectArrayElement(env, array, i);
                if (string) {
                    utf = (*env)->GetStringUTFChars(env, string, 0);
                    if (utf) {
                        arg = SDL_strdup(utf);
                        (*env)->ReleaseStringUTFChars(env, string, utf);
                    }
                    (*env)->DeleteLocalRef(env, string);
                }
                if (!arg) {
                    arg = SDL_strdup("");
                }
                argv[argc++] = arg;
            }
            argv[argc] = NULL;


            /* Run the application. */
            status = SDL_main(argc, argv);

            /* Release the arguments. */
            for (i = 0; i < argc; ++i) {
                SDL_free(argv[i]);
            }
            SDL_stack_free(argv);

        } else {
            __android_log_print(ANDROID_LOG_ERROR, "SDL", "nativeRunMain(): Couldn't find function %s in library %s", function_name, library_file);
        }
        (*env)->ReleaseStringUTFChars(env, function, function_name);

        dlclose(library_handle);

    } else {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "nativeRunMain(): Couldn't load library %s", library_file);
    }
    (*env)->ReleaseStringUTFChars(env, library, library_file);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */

    return status;
}

/* Drop file */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeDropFile)(
                                    JNIEnv* env, jclass jcls,
                                    jstring filename)
{
    const char *path = (*env)->GetStringUTFChars(env, filename, NULL);
    SDL_SendDropFile(NULL, path);
    (*env)->ReleaseStringUTFChars(env, filename, path);
    SDL_SendDropComplete(NULL);
}

/* Resize */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeResize)(
                                    JNIEnv* env, jclass jcls,
                                    jint width, jint height, jint format, jfloat rate)
{
    Android_SetScreenResolution(width, height, format, rate);
}

/* Paddown */
JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativePadDown)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id, jint keycode)
{
    return Android_OnPadDown(device_id, keycode);
}

/* Padup */
JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativePadUp)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id, jint keycode)
{
    return Android_OnPadUp(device_id, keycode);
}

/* Joy */
JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativeJoy)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id, jint axis, jfloat value)
{
    Android_OnJoy(device_id, axis, value);
}

/* POV Hat */
JNIEXPORT void JNICALL SDL_JAVA_CONTROLLER_INTERFACE(onNativeHat)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id, jint hat_id, jint x, jint y)
{
    Android_OnHat(device_id, hat_id, x, y);
}


JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeAddJoystick)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id, jstring device_name, jstring device_desc, jint is_accelerometer,
                                    jint nbuttons, jint naxes, jint nhats, jint nballs)
{
    int retval;
    const char *name = (*env)->GetStringUTFChars(env, device_name, NULL);
    const char *desc = (*env)->GetStringUTFChars(env, device_desc, NULL);

    retval = Android_AddJoystick(device_id, name, desc, (SDL_bool) is_accelerometer, nbuttons, naxes, nhats, nballs);

    (*env)->ReleaseStringUTFChars(env, device_name, name);
    (*env)->ReleaseStringUTFChars(env, device_desc, desc);

    return retval;
}

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeRemoveJoystick)(
                                    JNIEnv* env, jclass jcls,
                                    jint device_id)
{
    return Android_RemoveJoystick(device_id);
}

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeAddHaptic)(
    JNIEnv* env, jclass jcls, jint device_id, jstring device_name)
{
    int retval;
    const char *name = (*env)->GetStringUTFChars(env, device_name, NULL);

    retval = Android_AddHaptic(device_id, name);

    (*env)->ReleaseStringUTFChars(env, device_name, name);

    return retval;
}

JNIEXPORT jint JNICALL SDL_JAVA_CONTROLLER_INTERFACE(nativeRemoveHaptic)(
    JNIEnv* env, jclass jcls, jint device_id)
{
    return Android_RemoveHaptic(device_id);
}


/* Surface Created */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeSurfaceChanged)(JNIEnv* env, jclass jcls)
{
    SDL_WindowData *data;
    SDL_VideoDevice *_this;

    if (Android_Window == NULL || Android_Window->driverdata == NULL ) {
        return;
    }

    _this =  SDL_GetVideoDevice();
    data =  (SDL_WindowData *) Android_Window->driverdata;

    /* If the surface has been previously destroyed by onNativeSurfaceDestroyed, recreate it here */
    if (data->egl_surface == EGL_NO_SURFACE) {
        if(data->native_window) {
            ANativeWindow_release(data->native_window);
        }
        data->native_window = Android_JNI_GetNativeWindow();
        data->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) data->native_window);
    }

    /* GL Context handling is done in the event loop because this function is run from the Java thread */

}

/* Surface Destroyed */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeSurfaceDestroyed)(JNIEnv* env, jclass jcls)
{
    /* We have to clear the current context and destroy the egl surface here
     * Otherwise there's BAD_NATIVE_WINDOW errors coming from eglCreateWindowSurface on resume
     * Ref: http://stackoverflow.com/questions/8762589/eglcreatewindowsurface-on-ics-and-switching-from-2d-to-3d
     */
    SDL_WindowData *data;
    SDL_VideoDevice *_this;

    if (Android_Window == NULL || Android_Window->driverdata == NULL ) {
        return;
    }

    _this =  SDL_GetVideoDevice();
    data = (SDL_WindowData *) Android_Window->driverdata;

    if (data->egl_surface != EGL_NO_SURFACE) {
        SDL_EGL_MakeCurrent(_this, NULL, NULL);
        SDL_EGL_DestroySurface(_this, data->egl_surface);
        data->egl_surface = EGL_NO_SURFACE;
    }

    /* GL Context handling is done in the event loop because this function is run from the Java thread */

}

/* Keydown */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyDown)(
                                    JNIEnv* env, jclass jcls,
                                    jint keycode)
{
    Android_OnKeyDown(keycode);
}

/* Keyup */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyUp)(
                                    JNIEnv* env, jclass jcls,
                                    jint keycode)
{
    Android_OnKeyUp(keycode);
}

/* Keyboard Focus Lost */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeKeyboardFocusLost)(
                                    JNIEnv* env, jclass jcls)
{
    /* Calling SDL_StopTextInput will take care of hiding the keyboard and cleaning up the DummyText widget */
    SDL_StopTextInput();
}


/* Touch */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeTouch)(
                                    JNIEnv* env, jclass jcls,
                                    jint touch_device_id_in, jint pointer_finger_id_in,
                                    jint action, jfloat x, jfloat y, jfloat p)
{
    Android_OnTouch(touch_device_id_in, pointer_finger_id_in, action, x, y, p);
}

/* Mouse */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeMouse)(
                                    JNIEnv* env, jclass jcls,
                                    jint button, jint action, jfloat x, jfloat y)
{
    Android_OnMouse(button, action, x, y);
}

/* Accelerometer */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeAccel)(
                                    JNIEnv* env, jclass jcls,
                                    jfloat x, jfloat y, jfloat z)
{
    fLastAccelerometer[0] = x;
    fLastAccelerometer[1] = y;
    fLastAccelerometer[2] = z;
    bHasNewData = SDL_TRUE;
}

/* Clipboard */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(onNativeClipboardChanged)(
                                    JNIEnv* env, jclass jcls)
{
    SDL_SendClipboardUpdate();
}

/* Low memory */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeLowMemory)(
                                    JNIEnv* env, jclass cls)
{
    SDL_SendAppEvent(SDL_APP_LOWMEMORY);
}

/* Quit */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeQuit)(
                                    JNIEnv* env, jclass cls)
{
    /* Discard previous events. The user should have handled state storage
     * in SDL_APP_WILLENTERBACKGROUND. After nativeQuit() is called, no
     * events other than SDL_QUIT and SDL_APP_TERMINATING should fire */
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    /* Inject a SDL_QUIT event */
    SDL_SendQuit();
    SDL_SendAppEvent(SDL_APP_TERMINATING);
    /* Resume the event loop so that the app can catch SDL_QUIT which
     * should now be the top event in the event queue. */
    if (!SDL_SemValue(Android_ResumeSem)) SDL_SemPost(Android_ResumeSem);
}

/* Pause */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativePause)(
                                    JNIEnv* env, jclass cls)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "nativePause()");

    if (Android_Window) {
        SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_FOCUS_LOST, 0, 0);
        SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
        SDL_SendAppEvent(SDL_APP_WILLENTERBACKGROUND);
        SDL_SendAppEvent(SDL_APP_DIDENTERBACKGROUND);

        /* *After* sending the relevant events, signal the pause semaphore
         * so the event loop knows to pause and (optionally) block itself */
        if (!SDL_SemValue(Android_PauseSem)) SDL_SemPost(Android_PauseSem);
    }
}

/* Resume */
JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeResume)(
                                    JNIEnv* env, jclass cls)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "nativeResume()");

    if (Android_Window) {
        SDL_SendAppEvent(SDL_APP_WILLENTERFOREGROUND);
        SDL_SendAppEvent(SDL_APP_DIDENTERFOREGROUND);
        SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_FOCUS_GAINED, 0, 0);
        SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_RESTORED, 0, 0);
        /* Signal the resume semaphore so the event loop knows to resume and restore the GL Context
         * We can't restore the GL Context here because it needs to be done on the SDL main thread
         * and this function will be called from the Java thread instead.
         */
        if (!SDL_SemValue(Android_ResumeSem)) SDL_SemPost(Android_ResumeSem);
    }
}

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE_INPUT_CONNECTION(nativeCommitText)(
                                    JNIEnv* env, jclass cls,
                                    jstring text, jint newCursorPosition)
{
    const char *utftext = (*env)->GetStringUTFChars(env, text, NULL);

    SDL_SendKeyboardText(utftext);

    (*env)->ReleaseStringUTFChars(env, text, utftext);
}

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE_INPUT_CONNECTION(nativeGenerateScancodeForUnichar)(
                                    JNIEnv* env, jclass cls,
                                    jchar chUnicode)
{
    SDL_Scancode code = SDL_SCANCODE_UNKNOWN;
    uint16_t mod = 0;

    // We do not care about bigger than 127.
    if (chUnicode < 127) {
        AndroidKeyInfo info = unicharToAndroidKeyInfoTable[chUnicode];
        code = info.code;
        mod = info.mod;
    }

    if (mod & KMOD_SHIFT) {
        /* If character uses shift, press shift down */
        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LSHIFT);
    }

    /* send a keydown and keyup even for the character */
    SDL_SendKeyboardKey(SDL_PRESSED, code);
    SDL_SendKeyboardKey(SDL_RELEASED, code);

    if (mod & KMOD_SHIFT) {
        /* If character uses shift, press shift back up */
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LSHIFT);
    }
}


JNIEXPORT void JNICALL SDL_JAVA_INTERFACE_INPUT_CONNECTION(nativeSetComposingText)(
                                    JNIEnv* env, jclass cls,
                                    jstring text, jint newCursorPosition)
{
    const char *utftext = (*env)->GetStringUTFChars(env, text, NULL);

    SDL_SendEditingText(utftext, 0, 0);

    (*env)->ReleaseStringUTFChars(env, text, utftext);
}

JNIEXPORT jstring JNICALL SDL_JAVA_INTERFACE(nativeGetHint)(
                                    JNIEnv* env, jclass cls,
                                    jstring name)
{
    const char *utfname = (*env)->GetStringUTFChars(env, name, NULL);
    const char *hint = SDL_GetHint(utfname);

    jstring result = (*env)->NewStringUTF(env, hint);
    (*env)->ReleaseStringUTFChars(env, name, utfname);

    return result;
}

JNIEXPORT void JNICALL SDL_JAVA_INTERFACE(nativeSetenv)(
                                    JNIEnv* env, jclass cls,
                                    jstring name, jstring value)
{
    const char *utfname = (*env)->GetStringUTFChars(env, name, NULL);
    const char *utfvalue = (*env)->GetStringUTFChars(env, value, NULL);

    SDL_setenv(utfname, utfvalue, 1);

    (*env)->ReleaseStringUTFChars(env, name, utfname);
    (*env)->ReleaseStringUTFChars(env, value, utfvalue);

}

/*******************************************************************************
             Functions called by SDL into Java
*******************************************************************************/

static int s_active = 0;
struct LocalReferenceHolder
{
    JNIEnv *m_env;
    const char *m_func;
};

static struct LocalReferenceHolder LocalReferenceHolder_Setup(const char *func)
{
    struct LocalReferenceHolder refholder;
    refholder.m_env = NULL;
    refholder.m_func = func;
#ifdef DEBUG_JNI
    SDL_Log("Entering function %s", func);
#endif
    return refholder;
}

static SDL_bool LocalReferenceHolder_Init(struct LocalReferenceHolder *refholder, JNIEnv *env)
{
    const int capacity = 16;
    if ((*env)->PushLocalFrame(env, capacity) < 0) {
        SDL_SetError("Failed to allocate enough JVM local references");
        return SDL_FALSE;
    }
    ++s_active;
    refholder->m_env = env;
    return SDL_TRUE;
}

static void LocalReferenceHolder_Cleanup(struct LocalReferenceHolder *refholder)
{
#ifdef DEBUG_JNI
    SDL_Log("Leaving function %s", refholder->m_func);
#endif
    if (refholder->m_env) {
        JNIEnv* env = refholder->m_env;
        (*env)->PopLocalFrame(env, NULL);
        --s_active;
    }
}

static SDL_bool LocalReferenceHolder_IsActive(void)
{
    return s_active > 0;
}

ANativeWindow* Android_JNI_GetNativeWindow(void)
{
    ANativeWindow* anw;
    jobject s;
    JNIEnv *env = Android_JNI_GetEnv();

    s = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetNativeSurface);
    anw = ANativeWindow_fromSurface(env, s);
    (*env)->DeleteLocalRef(env, s);

    return anw;
}

void Android_JNI_SetActivityTitle(const char *title)
{
    JNIEnv *mEnv = Android_JNI_GetEnv();

    jstring jtitle = (jstring)((*mEnv)->NewStringUTF(mEnv, title));
    (*mEnv)->CallStaticBooleanMethod(mEnv, mActivityClass, midSetActivityTitle, jtitle);
    (*mEnv)->DeleteLocalRef(mEnv, jtitle);
}

void Android_JNI_SetWindowStyle(SDL_bool fullscreen)
{
    JNIEnv *mEnv = Android_JNI_GetEnv();
    (*mEnv)->CallStaticVoidMethod(mEnv, mActivityClass, midSetWindowStyle, fullscreen ? 1 : 0);
}

void Android_JNI_SetOrientation(int w, int h, int resizable, const char *hint)
{
    JNIEnv *mEnv = Android_JNI_GetEnv();

    jstring jhint = (jstring)((*mEnv)->NewStringUTF(mEnv, (hint ? hint : "")));
    (*mEnv)->CallStaticVoidMethod(mEnv, mActivityClass, midSetOrientation, w, h, (resizable? 1 : 0), jhint);
    (*mEnv)->DeleteLocalRef(mEnv, jhint);
}

SDL_bool Android_JNI_GetAccelerometerValues(float values[3])
{
    int i;
    SDL_bool retval = SDL_FALSE;

    if (bHasNewData) {
        for (i = 0; i < 3; ++i) {
            values[i] = fLastAccelerometer[i];
        }
        bHasNewData = SDL_FALSE;
        retval = SDL_TRUE;
    }

    return retval;
}

static void Android_JNI_ThreadDestroyed(void* value)
{
    /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey value to NULL as required */
    JNIEnv *env = (JNIEnv*) value;
    if (env != NULL) {
        (*mJavaVM)->DetachCurrentThread(mJavaVM);
        pthread_setspecific(mThreadKey, NULL);
    }
}

JNIEnv* Android_JNI_GetEnv(void)
{
    /* From http://developer.android.com/guide/practices/jni.html
     * All threads are Linux threads, scheduled by the kernel.
     * They're usually started from managed code (using Thread.start), but they can also be created elsewhere and then
     * attached to the JavaVM. For example, a thread started with pthread_create can be attached with the
     * JNI AttachCurrentThread or AttachCurrentThreadAsDaemon functions. Until a thread is attached, it has no JNIEnv,
     * and cannot make JNI calls.
     * Attaching a natively-created thread causes a java.lang.Thread object to be constructed and added to the "main"
     * ThreadGroup, making it visible to the debugger. Calling AttachCurrentThread on an already-attached thread
     * is a no-op.
     * Note: You can call this function any number of times for the same thread, there's no harm in it
     */

    JNIEnv *env;
    int status = (*mJavaVM)->AttachCurrentThread(mJavaVM, &env, NULL);
    if(status < 0) {
        LOGE("failed to attach current thread");
        return 0;
    }

    /* From http://developer.android.com/guide/practices/jni.html
     * Threads attached through JNI must call DetachCurrentThread before they exit. If coding this directly is awkward,
     * in Android 2.0 (Eclair) and higher you can use pthread_key_create to define a destructor function that will be
     * called before the thread exits, and call DetachCurrentThread from there. (Use that key with pthread_setspecific
     * to store the JNIEnv in thread-local-storage; that way it'll be passed into your destructor as the argument.)
     * Note: The destructor is not called unless the stored value is != NULL
     * Note: You can call this function any number of times for the same thread, there's no harm in it
     *       (except for some lost CPU cycles)
     */
    pthread_setspecific(mThreadKey, (void*) env);

    return env;
}

int Android_JNI_SetupThread(void)
{
    Android_JNI_GetEnv();
    return 1;
}

/*
 * Audio support
 */
static jboolean audioBuffer16Bit = JNI_FALSE;
static jobject audioBuffer = NULL;
static void* audioBufferPinned = NULL;
static jboolean captureBuffer16Bit = JNI_FALSE;
static jobject captureBuffer = NULL;

int Android_JNI_OpenAudioDevice(int iscapture, int sampleRate, int is16Bit, int channelCount, int desiredBufferFrames)
{
    jboolean audioBufferStereo;
    int audioBufferFrames;
    jobject jbufobj = NULL;
    jboolean isCopy;

    JNIEnv *env = Android_JNI_GetEnv();

    if (!env) {
        LOGE("callback_handler: failed to attach current thread");
    }
    Android_JNI_SetupThread();

    audioBufferStereo = channelCount > 1;

    if (iscapture) {
        __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "SDL audio: opening device for capture");
        captureBuffer16Bit = is16Bit;
        if ((*env)->CallStaticIntMethod(env, mAudioManagerClass, midCaptureOpen, sampleRate, audioBuffer16Bit, audioBufferStereo, desiredBufferFrames) != 0) {
            /* Error during audio initialization */
            __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL audio: error on AudioRecord initialization!");
            return 0;
        }
    } else {
        __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "SDL audio: opening device for output");
        audioBuffer16Bit = is16Bit;
        if ((*env)->CallStaticIntMethod(env, mAudioManagerClass, midAudioOpen, sampleRate, audioBuffer16Bit, audioBufferStereo, desiredBufferFrames) != 0) {
            /* Error during audio initialization */
            __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL audio: error on AudioTrack initialization!");
            return 0;
        }
    }

    /* Allocating the audio buffer from the Java side and passing it as the return value for audioInit no longer works on
     * Android >= 4.2 due to a "stale global reference" error. So now we allocate this buffer directly from this side. */

    if (is16Bit) {
        jshortArray audioBufferLocal = (*env)->NewShortArray(env, desiredBufferFrames * (audioBufferStereo ? 2 : 1));
        if (audioBufferLocal) {
            jbufobj = (*env)->NewGlobalRef(env, audioBufferLocal);
            (*env)->DeleteLocalRef(env, audioBufferLocal);
        }
    }
    else {
        jbyteArray audioBufferLocal = (*env)->NewByteArray(env, desiredBufferFrames * (audioBufferStereo ? 2 : 1));
        if (audioBufferLocal) {
            jbufobj = (*env)->NewGlobalRef(env, audioBufferLocal);
            (*env)->DeleteLocalRef(env, audioBufferLocal);
        }
    }

    if (jbufobj == NULL) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL audio: could not allocate an audio buffer!");
        return 0;
    }

    if (iscapture) {
        captureBuffer = jbufobj;
    } else {
        audioBuffer = jbufobj;
    }

    isCopy = JNI_FALSE;

    if (is16Bit) {
        if (!iscapture) {
            audioBufferPinned = (*env)->GetShortArrayElements(env, (jshortArray)audioBuffer, &isCopy);
        }
        audioBufferFrames = (*env)->GetArrayLength(env, (jshortArray)audioBuffer);
    } else {
        if (!iscapture) {
            audioBufferPinned = (*env)->GetByteArrayElements(env, (jbyteArray)audioBuffer, &isCopy);
        }
        audioBufferFrames = (*env)->GetArrayLength(env, (jbyteArray)audioBuffer);
    }

    if (audioBufferStereo) {
        audioBufferFrames /= 2;
    }

    return audioBufferFrames;
}

int Android_JNI_GetDisplayDPI(float *ddpi, float *xdpi, float *ydpi)
{
    JNIEnv *env = Android_JNI_GetEnv();

    jobject jDisplayObj = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetDisplayDPI);
    jclass jDisplayClass = (*env)->GetObjectClass(env, jDisplayObj);

    jfieldID fidXdpi = (*env)->GetFieldID(env, jDisplayClass, "xdpi", "F");
    jfieldID fidYdpi = (*env)->GetFieldID(env, jDisplayClass, "ydpi", "F");
    jfieldID fidDdpi = (*env)->GetFieldID(env, jDisplayClass, "densityDpi", "I");

    float nativeXdpi = (*env)->GetFloatField(env, jDisplayObj, fidXdpi);
    float nativeYdpi = (*env)->GetFloatField(env, jDisplayObj, fidYdpi);
    int nativeDdpi = (*env)->GetIntField(env, jDisplayObj, fidDdpi);


    (*env)->DeleteLocalRef(env, jDisplayObj);
    (*env)->DeleteLocalRef(env, jDisplayClass);

    if (ddpi) {
        *ddpi = (float)nativeDdpi;
    }
    if (xdpi) {
        *xdpi = nativeXdpi;
    }
    if (ydpi) {
        *ydpi = nativeYdpi;
    }

    return 0;
}

void * Android_JNI_GetAudioBuffer(void)
{
    return audioBufferPinned;
}

void Android_JNI_WriteAudioBuffer(void)
{
    JNIEnv *mAudioEnv = Android_JNI_GetEnv();

    if (audioBuffer16Bit) {
        (*mAudioEnv)->ReleaseShortArrayElements(mAudioEnv, (jshortArray)audioBuffer, (jshort *)audioBufferPinned, JNI_COMMIT);
        (*mAudioEnv)->CallStaticVoidMethod(mAudioEnv, mAudioManagerClass, midAudioWriteShortBuffer, (jshortArray)audioBuffer);
    } else {
        (*mAudioEnv)->ReleaseByteArrayElements(mAudioEnv, (jbyteArray)audioBuffer, (jbyte *)audioBufferPinned, JNI_COMMIT);
        (*mAudioEnv)->CallStaticVoidMethod(mAudioEnv, mAudioManagerClass, midAudioWriteByteBuffer, (jbyteArray)audioBuffer);
    }

    /* JNI_COMMIT means the changes are committed to the VM but the buffer remains pinned */
}

int Android_JNI_CaptureAudioBuffer(void *buffer, int buflen)
{
    JNIEnv *env = Android_JNI_GetEnv();
    jboolean isCopy = JNI_FALSE;
    jint br;

    if (captureBuffer16Bit) {
        SDL_assert((*env)->GetArrayLength(env, (jshortArray)captureBuffer) == (buflen / 2));
        br = (*env)->CallStaticIntMethod(env, mAudioManagerClass, midCaptureReadShortBuffer, (jshortArray)captureBuffer, JNI_TRUE);
        if (br > 0) {
            jshort *ptr = (*env)->GetShortArrayElements(env, (jshortArray)captureBuffer, &isCopy);
            br *= 2;
            SDL_memcpy(buffer, ptr, br);
            (*env)->ReleaseShortArrayElements(env, (jshortArray)captureBuffer, (jshort *)ptr, JNI_ABORT);
        }
    } else {
        SDL_assert((*env)->GetArrayLength(env, (jshortArray)captureBuffer) == buflen);
        br = (*env)->CallStaticIntMethod(env, mAudioManagerClass, midCaptureReadByteBuffer, (jbyteArray)captureBuffer, JNI_TRUE);
        if (br > 0) {
            jbyte *ptr = (*env)->GetByteArrayElements(env, (jbyteArray)captureBuffer, &isCopy);
            SDL_memcpy(buffer, ptr, br);
            (*env)->ReleaseByteArrayElements(env, (jbyteArray)captureBuffer, (jbyte *)ptr, JNI_ABORT);
        }
    }

    return (int) br;
}

void Android_JNI_FlushCapturedAudio(void)
{
    JNIEnv *env = Android_JNI_GetEnv();
#if 0  /* !!! FIXME: this needs API 23, or it'll do blocking reads and never end. */
    if (captureBuffer16Bit) {
        const jint len = (*env)->GetArrayLength(env, (jshortArray)captureBuffer);
        while ((*env)->CallStaticIntMethod(env, mActivityClass, midCaptureReadShortBuffer, (jshortArray)captureBuffer, JNI_FALSE) == len) { /* spin */ }
    } else {
        const jint len = (*env)->GetArrayLength(env, (jbyteArray)captureBuffer);
        while ((*env)->CallStaticIntMethod(env, mActivityClass, midCaptureReadByteBuffer, (jbyteArray)captureBuffer, JNI_FALSE) == len) { /* spin */ }
    }
#else
    if (captureBuffer16Bit) {
        (*env)->CallStaticIntMethod(env, mAudioManagerClass, midCaptureReadShortBuffer, (jshortArray)captureBuffer, JNI_FALSE);
    } else {
        (*env)->CallStaticIntMethod(env, mAudioManagerClass, midCaptureReadByteBuffer, (jbyteArray)captureBuffer, JNI_FALSE);
    }
#endif
}

void Android_JNI_CloseAudioDevice(const int iscapture)
{
    JNIEnv *env = Android_JNI_GetEnv();

    if (iscapture) {
        (*env)->CallStaticVoidMethod(env, mAudioManagerClass, midCaptureClose);
        if (captureBuffer) {
            (*env)->DeleteGlobalRef(env, captureBuffer);
            captureBuffer = NULL;
        }
    } else {
        (*env)->CallStaticVoidMethod(env, mAudioManagerClass, midAudioClose);
        if (audioBuffer) {
            (*env)->DeleteGlobalRef(env, audioBuffer);
            audioBuffer = NULL;
            audioBufferPinned = NULL;
        }
    }
}

/* Test for an exception and call SDL_SetError with its detail if one occurs */
/* If the parameter silent is truthy then SDL_SetError() will not be called. */
static SDL_bool Android_JNI_ExceptionOccurred(SDL_bool silent)
{
    JNIEnv *mEnv = Android_JNI_GetEnv();
    jthrowable exception;

    SDL_assert(LocalReferenceHolder_IsActive());

    exception = (*mEnv)->ExceptionOccurred(mEnv);
    if (exception != NULL) {
        jmethodID mid;

        /* Until this happens most JNI operations have undefined behaviour */
        (*mEnv)->ExceptionClear(mEnv);

        if (!silent) {
            jclass exceptionClass = (*mEnv)->GetObjectClass(mEnv, exception);
            jclass classClass = (*mEnv)->FindClass(mEnv, "java/lang/Class");
            jstring exceptionName;
            const char* exceptionNameUTF8;
            jstring exceptionMessage;

            mid = (*mEnv)->GetMethodID(mEnv, classClass, "getName", "()Ljava/lang/String;");
            exceptionName = (jstring)(*mEnv)->CallObjectMethod(mEnv, exceptionClass, mid);
            exceptionNameUTF8 = (*mEnv)->GetStringUTFChars(mEnv, exceptionName, 0);

            mid = (*mEnv)->GetMethodID(mEnv, exceptionClass, "getMessage", "()Ljava/lang/String;");
            exceptionMessage = (jstring)(*mEnv)->CallObjectMethod(mEnv, exception, mid);

            if (exceptionMessage != NULL) {
                const char* exceptionMessageUTF8 = (*mEnv)->GetStringUTFChars(mEnv, exceptionMessage, 0);
                SDL_SetError("%s: %s", exceptionNameUTF8, exceptionMessageUTF8);
                (*mEnv)->ReleaseStringUTFChars(mEnv, exceptionMessage, exceptionMessageUTF8);
            } else {
                SDL_SetError("%s", exceptionNameUTF8);
            }

            (*mEnv)->ReleaseStringUTFChars(mEnv, exceptionName, exceptionNameUTF8);
        }

        return SDL_TRUE;
    }

    return SDL_FALSE;
}

static int Internal_Android_JNI_FileOpen(SDL_RWops* ctx)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);

    int result = 0;

    jmethodID mid;
    jobject context;
    jobject assetManager;
    jobject inputStream;
    jclass channels;
    jobject readableByteChannel;
    jstring fileNameJString;
    jobject fd;
    jclass fdCls;
    jfieldID descriptor;

    JNIEnv *mEnv = Android_JNI_GetEnv();
    if (!LocalReferenceHolder_Init(&refs, mEnv)) {
        goto failure;
    }

    fileNameJString = (jstring)ctx->hidden.androidio.fileNameRef;
    ctx->hidden.androidio.position = 0;

    /* context = SDLActivity.getContext(); */
    context = (*mEnv)->CallStaticObjectMethod(mEnv, mActivityClass, midGetContext);

    /* assetManager = context.getAssets(); */
    mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, context),
            "getAssets", "()Landroid/content/res/AssetManager;");
    assetManager = (*mEnv)->CallObjectMethod(mEnv, context, mid);

    /* First let's try opening the file to obtain an AssetFileDescriptor.
    * This method reads the files directly from the APKs using standard *nix calls
    */
    mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, assetManager), "openFd", "(Ljava/lang/String;)Landroid/content/res/AssetFileDescriptor;");
    inputStream = (*mEnv)->CallObjectMethod(mEnv, assetManager, mid, fileNameJString);
    if (Android_JNI_ExceptionOccurred(SDL_TRUE)) {
        goto fallback;
    }

    mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream), "getStartOffset", "()J");
    ctx->hidden.androidio.offset = (*mEnv)->CallLongMethod(mEnv, inputStream, mid);
    if (Android_JNI_ExceptionOccurred(SDL_TRUE)) {
        goto fallback;
    }

    mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream), "getDeclaredLength", "()J");
    ctx->hidden.androidio.size = (*mEnv)->CallLongMethod(mEnv, inputStream, mid);
    if (Android_JNI_ExceptionOccurred(SDL_TRUE)) {
        goto fallback;
    }

    mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream), "getFileDescriptor", "()Ljava/io/FileDescriptor;");
    fd = (*mEnv)->CallObjectMethod(mEnv, inputStream, mid);
    fdCls = (*mEnv)->GetObjectClass(mEnv, fd);
    descriptor = (*mEnv)->GetFieldID(mEnv, fdCls, "descriptor", "I");
    ctx->hidden.androidio.fd = (*mEnv)->GetIntField(mEnv, fd, descriptor);
    ctx->hidden.androidio.assetFileDescriptorRef = (*mEnv)->NewGlobalRef(mEnv, inputStream);

    /* Seek to the correct offset in the file. */
    lseek(ctx->hidden.androidio.fd, (off_t)ctx->hidden.androidio.offset, SEEK_SET);

    if (0) {
fallback:
        /* Disabled log message because of spam on the Nexus 7 */
        /* __android_log_print(ANDROID_LOG_DEBUG, "SDL", "Falling back to legacy InputStream method for opening file"); */

        /* Try the old method using InputStream */
        ctx->hidden.androidio.assetFileDescriptorRef = NULL;

        /* inputStream = assetManager.open(<filename>); */
        mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, assetManager),
                "open", "(Ljava/lang/String;I)Ljava/io/InputStream;");
        inputStream = (*mEnv)->CallObjectMethod(mEnv, assetManager, mid, fileNameJString, 1 /* ACCESS_RANDOM */);
        if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
            /* Try fallback to APK expansion files */
            inputStream = (*mEnv)->CallStaticObjectMethod(mEnv, mActivityClass, midOpenAPKExpansionInputStream, fileNameJString);

            /* Exception is checked first because it always needs to be cleared.
             * If no exception occurred then the last SDL error message is kept.
             */
            if (Android_JNI_ExceptionOccurred(SDL_FALSE) || !inputStream) {
                goto failure;
            }
        }

        ctx->hidden.androidio.inputStreamRef = (*mEnv)->NewGlobalRef(mEnv, inputStream);

        /* Despite all the visible documentation on [Asset]InputStream claiming
         * that the .available() method is not guaranteed to return the entire file
         * size, comments in <sdk>/samples/<ver>/ApiDemos/src/com/example/ ...
         * android/apis/content/ReadAsset.java imply that Android's
         * AssetInputStream.available() /will/ always return the total file size
        */

        /* size = inputStream.available(); */
        mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream),
                "available", "()I");
        ctx->hidden.androidio.size = (long)(*mEnv)->CallIntMethod(mEnv, inputStream, mid);
        if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
            goto failure;
        }

        /* readableByteChannel = Channels.newChannel(inputStream); */
        channels = (*mEnv)->FindClass(mEnv, "java/nio/channels/Channels");
        mid = (*mEnv)->GetStaticMethodID(mEnv, channels,
                "newChannel",
                "(Ljava/io/InputStream;)Ljava/nio/channels/ReadableByteChannel;");
        readableByteChannel = (*mEnv)->CallStaticObjectMethod(
                mEnv, channels, mid, inputStream);
        if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
            goto failure;
        }

        ctx->hidden.androidio.readableByteChannelRef =
            (*mEnv)->NewGlobalRef(mEnv, readableByteChannel);

        /* Store .read id for reading purposes */
        mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, readableByteChannel),
                "read", "(Ljava/nio/ByteBuffer;)I");
        ctx->hidden.androidio.readMethod = mid;
    }

    if (0) {
failure:
        result = -1;

        (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.fileNameRef);

        if(ctx->hidden.androidio.inputStreamRef != NULL) {
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.inputStreamRef);
        }

        if(ctx->hidden.androidio.readableByteChannelRef != NULL) {
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.readableByteChannelRef);
        }

        if(ctx->hidden.androidio.assetFileDescriptorRef != NULL) {
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.assetFileDescriptorRef);
        }

    }

    LocalReferenceHolder_Cleanup(&refs);
    return result;
}

int Android_JNI_FileOpen(SDL_RWops* ctx,
        const char* fileName, const char* mode)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);
    JNIEnv *mEnv = Android_JNI_GetEnv();
    int retval;
    jstring fileNameJString;

    if (!LocalReferenceHolder_Init(&refs, mEnv)) {
        LocalReferenceHolder_Cleanup(&refs);
        return -1;
    }

    if (!ctx) {
        LocalReferenceHolder_Cleanup(&refs);
        return -1;
    }

    fileNameJString = (*mEnv)->NewStringUTF(mEnv, fileName);
    ctx->hidden.androidio.fileNameRef = (*mEnv)->NewGlobalRef(mEnv, fileNameJString);
    ctx->hidden.androidio.inputStreamRef = NULL;
    ctx->hidden.androidio.readableByteChannelRef = NULL;
    ctx->hidden.androidio.readMethod = NULL;
    ctx->hidden.androidio.assetFileDescriptorRef = NULL;

    retval = Internal_Android_JNI_FileOpen(ctx);
    LocalReferenceHolder_Cleanup(&refs);
    return retval;
}

size_t Android_JNI_FileRead(SDL_RWops* ctx, void* buffer,
        size_t size, size_t maxnum)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);

    if (ctx->hidden.androidio.assetFileDescriptorRef) {
        size_t bytesMax = size * maxnum;
        size_t result;
        if (ctx->hidden.androidio.size != -1 /* UNKNOWN_LENGTH */ && ctx->hidden.androidio.position + bytesMax > ctx->hidden.androidio.size) {
            bytesMax = ctx->hidden.androidio.size - ctx->hidden.androidio.position;
        }
        result = read(ctx->hidden.androidio.fd, buffer, bytesMax );
        if (result > 0) {
            ctx->hidden.androidio.position += result;
            LocalReferenceHolder_Cleanup(&refs);
            return result / size;
        }
        LocalReferenceHolder_Cleanup(&refs);
        return 0;
    } else {
        jlong bytesRemaining = (jlong) (size * maxnum);
        jlong bytesMax = (jlong) (ctx->hidden.androidio.size -  ctx->hidden.androidio.position);
        int bytesRead = 0;
        JNIEnv *mEnv;
        jobject readableByteChannel;
        jmethodID readMethod;
        jobject byteBuffer;

        /* Don't read more bytes than those that remain in the file, otherwise we get an exception */
        if (bytesRemaining >  bytesMax) bytesRemaining = bytesMax;

        mEnv = Android_JNI_GetEnv();
        if (!LocalReferenceHolder_Init(&refs, mEnv)) {
            LocalReferenceHolder_Cleanup(&refs);
            return 0;
        }

        readableByteChannel = (jobject)ctx->hidden.androidio.readableByteChannelRef;
        readMethod = (jmethodID)ctx->hidden.androidio.readMethod;
        byteBuffer = (*mEnv)->NewDirectByteBuffer(mEnv, buffer, bytesRemaining);

        while (bytesRemaining > 0) {
            /* result = readableByteChannel.read(...); */
            int result = (*mEnv)->CallIntMethod(mEnv, readableByteChannel, readMethod, byteBuffer);

            if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
                LocalReferenceHolder_Cleanup(&refs);
                return 0;
            }

            if (result < 0) {
                break;
            }

            bytesRemaining -= result;
            bytesRead += result;
            ctx->hidden.androidio.position += result;
        }
        LocalReferenceHolder_Cleanup(&refs);
        return bytesRead / size;
    }
}

size_t Android_JNI_FileWrite(SDL_RWops* ctx, const void* buffer,
        size_t size, size_t num)
{
    SDL_SetError("Cannot write to Android package filesystem");
    return 0;
}

static int Internal_Android_JNI_FileClose(SDL_RWops* ctx, SDL_bool release)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);

    int result = 0;
    JNIEnv *mEnv = Android_JNI_GetEnv();

    if (!LocalReferenceHolder_Init(&refs, mEnv)) {
        LocalReferenceHolder_Cleanup(&refs);
        return SDL_SetError("Failed to allocate enough JVM local references");
    }

    if (ctx) {
        if (release) {
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.fileNameRef);
        }

        if (ctx->hidden.androidio.assetFileDescriptorRef) {
            jobject inputStream = (jobject)ctx->hidden.androidio.assetFileDescriptorRef;
            jmethodID mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream),
                    "close", "()V");
            (*mEnv)->CallVoidMethod(mEnv, inputStream, mid);
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.assetFileDescriptorRef);
            if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
                result = -1;
            }
        }
        else {
            jobject inputStream = (jobject)ctx->hidden.androidio.inputStreamRef;

            /* inputStream.close(); */
            jmethodID mid = (*mEnv)->GetMethodID(mEnv, (*mEnv)->GetObjectClass(mEnv, inputStream),
                    "close", "()V");
            (*mEnv)->CallVoidMethod(mEnv, inputStream, mid);
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.inputStreamRef);
            (*mEnv)->DeleteGlobalRef(mEnv, (jobject)ctx->hidden.androidio.readableByteChannelRef);
            if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
                result = -1;
            }
        }

        if (release) {
            SDL_FreeRW(ctx);
        }
    }

    LocalReferenceHolder_Cleanup(&refs);
    return result;
}


Sint64 Android_JNI_FileSize(SDL_RWops* ctx)
{
    return ctx->hidden.androidio.size;
}

Sint64 Android_JNI_FileSeek(SDL_RWops* ctx, Sint64 offset, int whence)
{
    if (ctx->hidden.androidio.assetFileDescriptorRef) {
        off_t ret;
        switch (whence) {
            case RW_SEEK_SET:
                if (ctx->hidden.androidio.size != -1 /* UNKNOWN_LENGTH */ && offset > ctx->hidden.androidio.size) offset = ctx->hidden.androidio.size;
                offset += ctx->hidden.androidio.offset;
                break;
            case RW_SEEK_CUR:
                offset += ctx->hidden.androidio.position;
                if (ctx->hidden.androidio.size != -1 /* UNKNOWN_LENGTH */ && offset > ctx->hidden.androidio.size) offset = ctx->hidden.androidio.size;
                offset += ctx->hidden.androidio.offset;
                break;
            case RW_SEEK_END:
                offset = ctx->hidden.androidio.offset + ctx->hidden.androidio.size + offset;
                break;
            default:
                return SDL_SetError("Unknown value for 'whence'");
        }
        whence = SEEK_SET;

        ret = lseek(ctx->hidden.androidio.fd, (off_t)offset, SEEK_SET);
        if (ret == -1) return -1;
        ctx->hidden.androidio.position = ret - ctx->hidden.androidio.offset;
    } else {
        Sint64 newPosition;
        Sint64 movement;

        switch (whence) {
            case RW_SEEK_SET:
                newPosition = offset;
                break;
            case RW_SEEK_CUR:
                newPosition = ctx->hidden.androidio.position + offset;
                break;
            case RW_SEEK_END:
                newPosition = ctx->hidden.androidio.size + offset;
                break;
            default:
                return SDL_SetError("Unknown value for 'whence'");
        }

        /* Validate the new position */
        if (newPosition < 0) {
            return SDL_Error(SDL_EFSEEK);
        }
        if (newPosition > ctx->hidden.androidio.size) {
            newPosition = ctx->hidden.androidio.size;
        }

        movement = newPosition - ctx->hidden.androidio.position;
        if (movement > 0) {
            unsigned char buffer[4096];

            /* The easy case where we're seeking forwards */
            while (movement > 0) {
                Sint64 amount = sizeof (buffer);
                size_t result;
                if (amount > movement) {
                    amount = movement;
                }
                result = Android_JNI_FileRead(ctx, buffer, 1, amount);
                if (result <= 0) {
                    /* Failed to read/skip the required amount, so fail */
                    return -1;
                }

                movement -= result;
            }

        } else if (movement < 0) {
            /* We can't seek backwards so we have to reopen the file and seek */
            /* forwards which obviously isn't very efficient */
            Internal_Android_JNI_FileClose(ctx, SDL_FALSE);
            Internal_Android_JNI_FileOpen(ctx);
            Android_JNI_FileSeek(ctx, newPosition, RW_SEEK_SET);
        }
    }

    return ctx->hidden.androidio.position;

}

int Android_JNI_FileClose(SDL_RWops* ctx)
{
    return Internal_Android_JNI_FileClose(ctx, SDL_TRUE);
}

int Android_JNI_SetClipboardText(const char* text)
{
    JNIEnv* env = Android_JNI_GetEnv();
    jstring string = (*env)->NewStringUTF(env, text);
    (*env)->CallStaticVoidMethod(env, mActivityClass, midClipboardSetText, string);
    (*env)->DeleteLocalRef(env, string);
    return 0;
}

char* Android_JNI_GetClipboardText(void)
{
    JNIEnv* env = Android_JNI_GetEnv();
    char* text = NULL;
    jstring string;
    
    string = (*env)->CallStaticObjectMethod(env, mActivityClass, midClipboardGetText);
    if (string) {
        const char* utf = (*env)->GetStringUTFChars(env, string, 0);
        if (utf) {
            text = SDL_strdup(utf);
            (*env)->ReleaseStringUTFChars(env, string, utf);
        }
        (*env)->DeleteLocalRef(env, string);
    }
    
    return (text == NULL) ? SDL_strdup("") : text;
}

SDL_bool Android_JNI_HasClipboardText(void)
{
    JNIEnv* env = Android_JNI_GetEnv();
    jboolean retval = (*env)->CallStaticBooleanMethod(env, mActivityClass, midClipboardHasText);
    return (retval == JNI_TRUE) ? SDL_TRUE : SDL_FALSE;
}

/* returns 0 on success or -1 on error (others undefined then)
 * returns truthy or falsy value in plugged, charged and battery
 * returns the value in seconds and percent or -1 if not available
 */
int Android_JNI_GetPowerInfo(int* plugged, int* charged, int* battery, int* seconds, int* percent)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);
    JNIEnv* env = Android_JNI_GetEnv();
    jmethodID mid;
    jobject context;
    jstring action;
    jclass cls;
    jobject filter;
    jobject intent;
    jstring iname;
    jmethodID imid;
    jstring bname;
    jmethodID bmid;
    if (!LocalReferenceHolder_Init(&refs, env)) {
        LocalReferenceHolder_Cleanup(&refs);
        return -1;
    }


    /* context = SDLActivity.getContext(); */
    context = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetContext);

    action = (*env)->NewStringUTF(env, "android.intent.action.BATTERY_CHANGED");

    cls = (*env)->FindClass(env, "android/content/IntentFilter");

    mid = (*env)->GetMethodID(env, cls, "<init>", "(Ljava/lang/String;)V");
    filter = (*env)->NewObject(env, cls, mid, action);

    (*env)->DeleteLocalRef(env, action);

    mid = (*env)->GetMethodID(env, mActivityClass, "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;");
    intent = (*env)->CallObjectMethod(env, context, mid, NULL, filter);

    (*env)->DeleteLocalRef(env, filter);

    cls = (*env)->GetObjectClass(env, intent);

    imid = (*env)->GetMethodID(env, cls, "getIntExtra", "(Ljava/lang/String;I)I");

    /* Watch out for C89 scoping rules because of the macro */
#define GET_INT_EXTRA(var, key) \
    int var; \
    iname = (*env)->NewStringUTF(env, key); \
    var = (*env)->CallIntMethod(env, intent, imid, iname, -1); \
    (*env)->DeleteLocalRef(env, iname);

    bmid = (*env)->GetMethodID(env, cls, "getBooleanExtra", "(Ljava/lang/String;Z)Z");

    /* Watch out for C89 scoping rules because of the macro */
#define GET_BOOL_EXTRA(var, key) \
    int var; \
    bname = (*env)->NewStringUTF(env, key); \
    var = (*env)->CallBooleanMethod(env, intent, bmid, bname, JNI_FALSE); \
    (*env)->DeleteLocalRef(env, bname);

    if (plugged) {
        /* Watch out for C89 scoping rules because of the macro */
        GET_INT_EXTRA(plug, "plugged") /* == BatteryManager.EXTRA_PLUGGED (API 5) */
        if (plug == -1) {
            LocalReferenceHolder_Cleanup(&refs);
            return -1;
        }
        /* 1 == BatteryManager.BATTERY_PLUGGED_AC */
        /* 2 == BatteryManager.BATTERY_PLUGGED_USB */
        *plugged = (0 < plug) ? 1 : 0;
    }

    if (charged) {
        /* Watch out for C89 scoping rules because of the macro */
        GET_INT_EXTRA(status, "status") /* == BatteryManager.EXTRA_STATUS (API 5) */
        if (status == -1) {
            LocalReferenceHolder_Cleanup(&refs);
            return -1;
        }
        /* 5 == BatteryManager.BATTERY_STATUS_FULL */
        *charged = (status == 5) ? 1 : 0;
    }

    if (battery) {
        GET_BOOL_EXTRA(present, "present") /* == BatteryManager.EXTRA_PRESENT (API 5) */
        *battery = present ? 1 : 0;
    }

    if (seconds) {
        *seconds = -1; /* not possible */
    }

    if (percent) {
        int level;
        int scale;

        /* Watch out for C89 scoping rules because of the macro */
        {
            GET_INT_EXTRA(level_temp, "level") /* == BatteryManager.EXTRA_LEVEL (API 5) */
            level = level_temp;
        }
        /* Watch out for C89 scoping rules because of the macro */
        {
            GET_INT_EXTRA(scale_temp, "scale") /* == BatteryManager.EXTRA_SCALE (API 5) */
            scale = scale_temp;
        }

        if ((level == -1) || (scale == -1)) {
            LocalReferenceHolder_Cleanup(&refs);
            return -1;
        }
        *percent = level * 100 / scale;
    }

    (*env)->DeleteLocalRef(env, intent);

    LocalReferenceHolder_Cleanup(&refs);
    return 0;
}

/* returns number of found touch devices as return value and ids in parameter ids */
int Android_JNI_GetTouchDeviceIds(int **ids) {
    JNIEnv *env = Android_JNI_GetEnv();
    jint sources = 4098; /* == InputDevice.SOURCE_TOUCHSCREEN */
    jintArray array = (jintArray) (*env)->CallStaticObjectMethod(env, mActivityClass, midInputGetInputDeviceIds, sources);
    int number = 0;
    *ids = NULL;
    if (array) {
        number = (int) (*env)->GetArrayLength(env, array);
        if (0 < number) {
            jint* elements = (*env)->GetIntArrayElements(env, array, NULL);
            if (elements) {
                int i;
                *ids = SDL_malloc(number * sizeof (**ids));
                for (i = 0; i < number; ++i) { /* not assuming sizeof (jint) == sizeof (int) */
                    (*ids)[i] = elements[i];
                }
                (*env)->ReleaseIntArrayElements(env, array, elements, JNI_ABORT);
            }
        }
        (*env)->DeleteLocalRef(env, array);
    }
    return number;
}

/* sets the mSeparateMouseAndTouch field */
void Android_JNI_SetSeparateMouseAndTouch(SDL_bool new_value)
{
    JNIEnv *env = Android_JNI_GetEnv();
    (*env)->SetStaticBooleanField(env, mActivityClass, fidSeparateMouseAndTouch, new_value ? JNI_TRUE : JNI_FALSE);
}

void Android_JNI_PollInputDevices(void)
{
    JNIEnv *env = Android_JNI_GetEnv();
    (*env)->CallStaticVoidMethod(env, mControllerManagerClass, midPollInputDevices);
}

void Android_JNI_PollHapticDevices(void)
{
    JNIEnv *env = Android_JNI_GetEnv();
    (*env)->CallStaticVoidMethod(env, mControllerManagerClass, midPollHapticDevices);
}

void Android_JNI_HapticRun(int device_id, int length)
{
    JNIEnv *env = Android_JNI_GetEnv();
    (*env)->CallStaticVoidMethod(env, mControllerManagerClass, midHapticRun, device_id, length);
}


/* See SDLActivity.java for constants. */
#define COMMAND_SET_KEEP_SCREEN_ON    5

/* sends message to be handled on the UI event dispatch thread */
int Android_JNI_SendMessage(int command, int param)
{
    JNIEnv *env = Android_JNI_GetEnv();
    jboolean success;
    success = (*env)->CallStaticBooleanMethod(env, mActivityClass, midSendMessage, command, param);
    return success ? 0 : -1;
}

void Android_JNI_SuspendScreenSaver(SDL_bool suspend)
{
    Android_JNI_SendMessage(COMMAND_SET_KEEP_SCREEN_ON, (suspend == SDL_FALSE) ? 0 : 1);
}

void Android_JNI_ShowTextInput(SDL_Rect *inputRect)
{
    JNIEnv *env = Android_JNI_GetEnv();
    (*env)->CallStaticBooleanMethod(env, mActivityClass, midShowTextInput,
                               inputRect->x,
                               inputRect->y,
                               inputRect->w,
                               inputRect->h );
}

void Android_JNI_HideTextInput(void)
{
    /* has to match Activity constant */
    const int COMMAND_TEXTEDIT_HIDE = 3;
    Android_JNI_SendMessage(COMMAND_TEXTEDIT_HIDE, 0);
}

SDL_bool Android_JNI_IsScreenKeyboardShown()
{
    JNIEnv *mEnv = Android_JNI_GetEnv();
    jboolean is_shown = 0;
    is_shown = (*mEnv)->CallStaticBooleanMethod(mEnv, mActivityClass, midIsScreenKeyboardShown);
    return is_shown;
}


int Android_JNI_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    JNIEnv *env;
    jclass clazz;
    jmethodID mid;
    jobject context;
    jstring title;
    jstring message;
    jintArray button_flags;
    jintArray button_ids;
    jobjectArray button_texts;
    jintArray colors;
    jobject text;
    jint temp;
    int i;

    env = Android_JNI_GetEnv();

    /* convert parameters */

    clazz = (*env)->FindClass(env, "java/lang/String");

    title = (*env)->NewStringUTF(env, messageboxdata->title);
    message = (*env)->NewStringUTF(env, messageboxdata->message);

    button_flags = (*env)->NewIntArray(env, messageboxdata->numbuttons);
    button_ids = (*env)->NewIntArray(env, messageboxdata->numbuttons);
    button_texts = (*env)->NewObjectArray(env, messageboxdata->numbuttons,
        clazz, NULL);
    for (i = 0; i < messageboxdata->numbuttons; ++i) {
        temp = messageboxdata->buttons[i].flags;
        (*env)->SetIntArrayRegion(env, button_flags, i, 1, &temp);
        temp = messageboxdata->buttons[i].buttonid;
        (*env)->SetIntArrayRegion(env, button_ids, i, 1, &temp);
        text = (*env)->NewStringUTF(env, messageboxdata->buttons[i].text);
        (*env)->SetObjectArrayElement(env, button_texts, i, text);
        (*env)->DeleteLocalRef(env, text);
    }

    if (messageboxdata->colorScheme) {
        colors = (*env)->NewIntArray(env, SDL_MESSAGEBOX_COLOR_MAX);
        for (i = 0; i < SDL_MESSAGEBOX_COLOR_MAX; ++i) {
            temp = (0xFF << 24) |
                   (messageboxdata->colorScheme->colors[i].r << 16) |
                   (messageboxdata->colorScheme->colors[i].g << 8) |
                   (messageboxdata->colorScheme->colors[i].b << 0);
            (*env)->SetIntArrayRegion(env, colors, i, 1, &temp);
        }
    } else {
        colors = NULL;
    }

    (*env)->DeleteLocalRef(env, clazz);

    /* context = SDLActivity.getContext(); */
    context = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetContext);

    clazz = (*env)->GetObjectClass(env, context);

    mid = (*env)->GetMethodID(env, clazz,
        "messageboxShowMessageBox", "(ILjava/lang/String;Ljava/lang/String;[I[I[Ljava/lang/String;[I)I");
    *buttonid = (*env)->CallIntMethod(env, context, mid,
        messageboxdata->flags,
        title,
        message,
        button_flags,
        button_ids,
        button_texts,
        colors);

    (*env)->DeleteLocalRef(env, context);
    (*env)->DeleteLocalRef(env, clazz);

    /* delete parameters */

    (*env)->DeleteLocalRef(env, title);
    (*env)->DeleteLocalRef(env, message);
    (*env)->DeleteLocalRef(env, button_flags);
    (*env)->DeleteLocalRef(env, button_ids);
    (*env)->DeleteLocalRef(env, button_texts);
    (*env)->DeleteLocalRef(env, colors);

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////
//
// Functions exposed to SDL applications in SDL_system.h
//////////////////////////////////////////////////////////////////////////////
*/

void *SDL_AndroidGetJNIEnv(void)
{
    return Android_JNI_GetEnv();
}

void *SDL_AndroidGetActivity(void)
{
    /* See SDL_system.h for caveats on using this function. */

    JNIEnv *env = Android_JNI_GetEnv();
    if (!env) {
        return NULL;
    }

    /* return SDLActivity.getContext(); */
    return (*env)->CallStaticObjectMethod(env, mActivityClass, midGetContext);
}

SDL_bool SDL_IsAndroidTV(void)
{
    JNIEnv *env = Android_JNI_GetEnv();
    return (*env)->CallStaticBooleanMethod(env, mActivityClass, midIsAndroidTV);
}

const char * SDL_AndroidGetInternalStoragePath(void)
{
    static char *s_AndroidInternalFilesPath = NULL;

    if (!s_AndroidInternalFilesPath) {
        struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);
        jmethodID mid;
        jobject context;
        jobject fileObject;
        jstring pathString;
        const char *path;

        JNIEnv *env = Android_JNI_GetEnv();
        if (!LocalReferenceHolder_Init(&refs, env)) {
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        /* context = SDLActivity.getContext(); */
        context = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetContext);
        if (!context) {
            SDL_SetError("Couldn't get Android context!");
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        /* fileObj = context.getFilesDir(); */
        mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
                "getFilesDir", "()Ljava/io/File;");
        fileObject = (*env)->CallObjectMethod(env, context, mid);
        if (!fileObject) {
            SDL_SetError("Couldn't get internal directory");
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        /* path = fileObject.getCanonicalPath(); */
        mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, fileObject),
                "getCanonicalPath", "()Ljava/lang/String;");
        pathString = (jstring)(*env)->CallObjectMethod(env, fileObject, mid);
        if (Android_JNI_ExceptionOccurred(SDL_FALSE)) {
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        path = (*env)->GetStringUTFChars(env, pathString, NULL);
        s_AndroidInternalFilesPath = SDL_strdup(path);
        (*env)->ReleaseStringUTFChars(env, pathString, path);

        LocalReferenceHolder_Cleanup(&refs);
    }
    return s_AndroidInternalFilesPath;
}

int SDL_AndroidGetExternalStorageState(void)
{
    struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);
    jmethodID mid;
    jclass cls;
    jstring stateString;
    const char *state;
    int stateFlags;

    JNIEnv *env = Android_JNI_GetEnv();
    if (!LocalReferenceHolder_Init(&refs, env)) {
        LocalReferenceHolder_Cleanup(&refs);
        return 0;
    }

    cls = (*env)->FindClass(env, "android/os/Environment");
    mid = (*env)->GetStaticMethodID(env, cls,
            "getExternalStorageState", "()Ljava/lang/String;");
    stateString = (jstring)(*env)->CallStaticObjectMethod(env, cls, mid);

    state = (*env)->GetStringUTFChars(env, stateString, NULL);

    /* Print an info message so people debugging know the storage state */
    __android_log_print(ANDROID_LOG_INFO, "SDL", "external storage state: %s", state);

    if (SDL_strcmp(state, "mounted") == 0) {
        stateFlags = SDL_ANDROID_EXTERNAL_STORAGE_READ |
                     SDL_ANDROID_EXTERNAL_STORAGE_WRITE;
    } else if (SDL_strcmp(state, "mounted_ro") == 0) {
        stateFlags = SDL_ANDROID_EXTERNAL_STORAGE_READ;
    } else {
        stateFlags = 0;
    }
    (*env)->ReleaseStringUTFChars(env, stateString, state);

    LocalReferenceHolder_Cleanup(&refs);
    return stateFlags;
}

const char * SDL_AndroidGetExternalStoragePath(void)
{
    static char *s_AndroidExternalFilesPath = NULL;

    if (!s_AndroidExternalFilesPath) {
        struct LocalReferenceHolder refs = LocalReferenceHolder_Setup(__FUNCTION__);
        jmethodID mid;
        jobject context;
        jobject fileObject;
        jstring pathString;
        const char *path;

        JNIEnv *env = Android_JNI_GetEnv();
        if (!LocalReferenceHolder_Init(&refs, env)) {
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        /* context = SDLActivity.getContext(); */
        context = (*env)->CallStaticObjectMethod(env, mActivityClass, midGetContext);

        /* fileObj = context.getExternalFilesDir(); */
        mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
                "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
        fileObject = (*env)->CallObjectMethod(env, context, mid, NULL);
        if (!fileObject) {
            SDL_SetError("Couldn't get external directory");
            LocalReferenceHolder_Cleanup(&refs);
            return NULL;
        }

        /* path = fileObject.getAbsolutePath(); */
        mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, fileObject),
                "getAbsolutePath", "()Ljava/lang/String;");
        pathString = (jstring)(*env)->CallObjectMethod(env, fileObject, mid);

        path = (*env)->GetStringUTFChars(env, pathString, NULL);
        s_AndroidExternalFilesPath = SDL_strdup(path);
        (*env)->ReleaseStringUTFChars(env, pathString, path);

        LocalReferenceHolder_Cleanup(&refs);
    }
    return s_AndroidExternalFilesPath;
}

void Android_JNI_GetManifestEnvironmentVariables(void)
{
    if (!mActivityClass || !midGetManifestEnvironmentVariables) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "Request to get environment variables before JNI is ready");
        return;
    }

    if (!bHasEnvironmentVariables) {
        JNIEnv *env = Android_JNI_GetEnv();
        SDL_bool ret = (*env)->CallStaticBooleanMethod(env, mActivityClass, midGetManifestEnvironmentVariables);
        if (ret) {
            bHasEnvironmentVariables = SDL_TRUE;
        }
    }
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
