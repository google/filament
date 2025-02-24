//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AndroidWindow.cpp: Implementation of OSWindow for Android

#include "util/android/AndroidWindow.h"

#include <pthread.h>
#include <filesystem>
#include <iostream>

#include "common/debug.h"
#include "util/android/third_party/android_native_app_glue.h"

namespace
{
struct android_app *sApp = nullptr;
pthread_mutex_t sInitWindowMutex;
pthread_cond_t sInitWindowCond;
bool sInitWindowDone = false;
JNIEnv *gJni         = nullptr;

// SCREEN_ORIENTATION_LANDSCAPE and SCREEN_ORIENTATION_PORTRAIT are
// available from Android API level 1
// https://developer.android.com/reference/android/app/Activity#setRequestedOrientation(int)
const int kScreenOrientationLandscape = 0;
const int kScreenOrientationPortrait  = 1;

JNIEnv *GetJniEnv()
{
    if (gJni)
        return gJni;

    sApp->activity->vm->AttachCurrentThread(&gJni, NULL);
    return gJni;
}

int SetScreenOrientation(struct android_app *app, int orientation)
{
    // Use reverse JNI to call the Java entry point that rotates the
    // display to respect width and height
    JNIEnv *jni = GetJniEnv();
    if (!jni)
    {
        std::cerr << "Failed to get JNI env for screen rotation";
        return JNI_ERR;
    }

    jclass clazz       = jni->GetObjectClass(app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "setRequestedOrientation", "(I)V");
    jni->CallVoidMethod(app->activity->clazz, methodID, orientation);

    return 0;
}
}  // namespace

AndroidWindow::AndroidWindow() {}

AndroidWindow::~AndroidWindow() {}

bool AndroidWindow::initializeImpl(const std::string &name, int width, int height)
{
    return resize(width, height);
}
void AndroidWindow::destroy() {}

void AndroidWindow::disableErrorMessageDialog() {}

void AndroidWindow::resetNativeWindow() {}

EGLNativeWindowType AndroidWindow::getNativeWindow() const
{
    // Return the entire Activity Surface for now
    // sApp->window is valid only after sInitWindowDone, which is true after initializeImpl()
    return sApp->window;
}

EGLNativeDisplayType AndroidWindow::getNativeDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

void AndroidWindow::messageLoop()
{
    // TODO: accumulate events in the real message loop of android_main,
    // and process them here
}

void AndroidWindow::setMousePosition(int x, int y)
{
    UNIMPLEMENTED();
}

bool AndroidWindow::setOrientation(int width, int height)
{
    // Set tests to run in correct orientation
    int32_t err = SetScreenOrientation(
        sApp, (width > height) ? kScreenOrientationLandscape : kScreenOrientationPortrait);

    return err == 0;
}
bool AndroidWindow::setPosition(int x, int y)
{
    UNIMPLEMENTED();
    return false;
}

bool AndroidWindow::resize(int width, int height)
{
    mWidth  = width;
    mHeight = height;

    // sApp->window used below is valid only after Activity Surface is created
    pthread_mutex_lock(&sInitWindowMutex);
    while (!sInitWindowDone)
    {
        pthread_cond_wait(&sInitWindowCond, &sInitWindowMutex);
    }
    pthread_mutex_unlock(&sInitWindowMutex);

    if (sApp->window == nullptr)
    {
        // Note: logging isn't initalized yet but this message shows up in logcat.
        FATAL() << "Window is NULL (is screen locked? e.g. SplashScreen in logcat)";
    }

    // TODO: figure out a way to set the format as well,
    // which is available only after EGLWindow initialization
    int32_t err = ANativeWindow_setBuffersGeometry(sApp->window, mWidth, mHeight, 0);
    return err == 0;
}

void AndroidWindow::setVisible(bool isVisible) {}

void AndroidWindow::signalTestEvent()
{
    UNIMPLEMENTED();
}

static void onAppCmd(struct android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            pthread_mutex_lock(&sInitWindowMutex);
            sInitWindowDone = true;
            pthread_cond_broadcast(&sInitWindowCond);
            pthread_mutex_unlock(&sInitWindowMutex);
            break;
        case APP_CMD_DESTROY:
            if (gJni)
            {
                sApp->activity->vm->DetachCurrentThread();
            }
            gJni = nullptr;
            break;

            // TODO: process other commands and pass them to AndroidWindow for handling
            // TODO: figure out how to handle APP_CMD_PAUSE,
            // which should immediately halt all the rendering,
            // since Activity Surface is no longer available.
            // Currently tests crash when paused, for example, due to device changing orientation
    }
}

static int32_t onInputEvent(struct android_app *app, AInputEvent *event)
{
    // TODO: Handle input events
    return 0;  // 0 == not handled
}

static bool validPollResult(int result)
{
    return result >= 0 || result == ALOOPER_POLL_CALLBACK;
}

void android_main(struct android_app *app)
{
    int events;
    struct android_poll_source *source;

    sApp = app;
    pthread_mutex_init(&sInitWindowMutex, nullptr);
    pthread_cond_init(&sInitWindowCond, nullptr);

    // Event handlers, invoked from source->process()
    app->onAppCmd     = onAppCmd;
    app->onInputEvent = onInputEvent;

    // Message loop, polling for events indefinitely (due to -1 timeout)
    // Must be here in order to handle APP_CMD_INIT_WINDOW event,
    // which occurs after AndroidWindow::initializeImpl(), but before AndroidWindow::messageLoop
    while (
        validPollResult(ALooper_pollOnce(-1, nullptr, &events, reinterpret_cast<void **>(&source))))
    {
        if (source != nullptr)
        {
            source->process(app, source);
        }
    }
}

std::string AndroidWindow::GetApplicationDirectory()
{
    // Use reverse JNI.
    JNIEnv *jni = GetJniEnv();
    if (!jni)
    {
        std::cerr << "GetApplicationDirectory:: Failed to get JNI env";
        return "";
    }

    // Get the ANativeActivity class
    jclass nativeActivityClass = jni->GetObjectClass(sApp->activity->clazz);
    if (!nativeActivityClass)
    {
        std::cerr << "GetApplicationDirectory: Failed to get ANativeActivity class";
        return "";
    }

    // Get the getApplicationContext() method ID
    jmethodID getApplicationContextMethod = jni->GetMethodID(
        nativeActivityClass, "getApplicationContext", "()Landroid/content/Context;");
    if (!getApplicationContextMethod)
    {
        std::cerr << "GetApplicationDirectory: Failed to find getApplicationContext method";
        return "";
    }

    // Call getApplicationContext() to get the Context object
    jobject context = jni->CallObjectMethod(sApp->activity->clazz, getApplicationContextMethod);
    if (!context)
    {
        std::cerr << "GetApplicationDirectory: Failed to get Context object";
        return "";
    }

    // Get the Context class
    jclass contextClass = jni->GetObjectClass(context);
    if (!contextClass)
    {
        std::cerr << "GetApplicationDirectory: Failed to get Context class";
        return "";
    }

    // Get the getFilesDir() method ID
    jmethodID getFilesDirMethod = jni->GetMethodID(contextClass, "getFilesDir", "()Ljava/io/File;");
    if (!getFilesDirMethod)
    {
        std::cerr << "GetApplicationDirectory: Failed to find getFilesDir method";
        return "";
    }

    // Call getFilesDir() to get the File object
    jobject fileObject = jni->CallObjectMethod(context, getFilesDirMethod);
    if (!fileObject)
    {
        std::cerr << "GetApplicationDirectory: Failed to get File object";
        return "";
    }

    // Get the File class
    jclass fileClass = jni->GetObjectClass(fileObject);
    if (!fileClass)
    {
        std::cerr << "GetApplicationDirectory: Failed to get File class";
        return "";
    }

    // Get the getAbsolutePath() method ID
    jmethodID getAbsolutePathMethod =
        jni->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    if (!getAbsolutePathMethod)
    {
        std::cerr << "GetApplicationDirectory: Failed to find getAbsolutePath method";
        return "";
    }

    // Call getAbsolutePath() to get the path as a jstring
    jstring pathString = (jstring)jni->CallObjectMethod(fileObject, getAbsolutePathMethod);
    if (!pathString)
    {
        std::cerr << "GetApplicationDirectory: Failed to get path string";
        return "";
    }

    // Convert the jstring to a std::string
    const char *pathChars = jni->GetStringUTFChars(pathString, nullptr);
    std::string filesDirPath(pathChars);
    jni->ReleaseStringUTFChars(pathString, pathChars);

    // Return the base directory, stripping "files" essentially
    std::filesystem::path fullPath(filesDirPath);
    return fullPath.parent_path();
}

// static
std::string AndroidWindow::GetExternalStorageDirectory()
{
    // Use reverse JNI.
    JNIEnv *jni = GetJniEnv();
    if (!jni)
    {
        std::cerr << "GetExternalStorageDirectory:: Failed to get JNI env";
        return "";
    }

    jclass classEnvironment = jni->FindClass("android/os/Environment");
    if (classEnvironment == 0)
    {
        std::cerr << "GetExternalStorageDirectory: Failed to find Environment";
        return "";
    }

    // public static File getExternalStorageDirectory ()
    jmethodID methodIDgetExternalStorageDirectory =
        jni->GetStaticMethodID(classEnvironment, "getExternalStorageDirectory", "()Ljava/io/File;");
    if (methodIDgetExternalStorageDirectory == 0)
    {
        std::cerr << "GetExternalStorageDirectory: Failed to get static method";
        return "";
    }

    jobject objectFile =
        jni->CallStaticObjectMethod(classEnvironment, methodIDgetExternalStorageDirectory);
    jthrowable exception = jni->ExceptionOccurred();
    if (exception != 0)
    {
        jni->ExceptionDescribe();
        jni->ExceptionClear();
        std::cerr << "GetExternalStorageDirectory: Failed because of exception";
        return "";
    }

    // Call method on File object to retrieve String object.
    jclass classFile = jni->GetObjectClass(objectFile);
    if (classEnvironment == 0)
    {
        std::cerr << "GetExternalStorageDirectory: Failed to find object class";
        return "";
    }

    jmethodID methodIDgetAbsolutePath =
        jni->GetMethodID(classFile, "getAbsolutePath", "()Ljava/lang/String;");
    if (methodIDgetAbsolutePath == 0)
    {
        std::cerr << "GetExternalStorageDirectory: Failed to get method ID";
        return "";
    }

    jstring stringPath =
        static_cast<jstring>(jni->CallObjectMethod(objectFile, methodIDgetAbsolutePath));

    // TODO(jmadill): Find how to pass the root test directory to ANGLE. http://crbug.com/1097957

    // // https://stackoverflow.com/questions/12841240/android-pass-parameter-to-native-activity
    // jclass clazz = jni->GetObjectClass(sApp->activity->clazz);
    // if (clazz == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Bad activity";
    //     return "";
    // }

    // jmethodID giid = jni->GetMethodID(clazz, "getIntent", "()Landroid/content/Intent;");
    // if (giid == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Could not find getIntent";
    //     return "";
    // }

    // jobject intent = jni->CallObjectMethod(sApp->activity->clazz, giid);
    // if (intent == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Error calling getIntent";
    //     return "";
    // }

    // jclass icl = jni->GetObjectClass(intent);
    // if (icl == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Error getting getIntent class";
    //     return "";
    // }

    // jmethodID gseid =
    //     jni->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    // if (gseid == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Could not find getStringExtra";
    //     return "";
    // }

    // jstring stringPath = static_cast<jstring>(jni->CallObjectMethod(
    //     intent, gseid, jni->NewStringUTF("org.chromium.base.test.util.UrlUtils.RootDirectory")));
    // if (stringPath != 0)
    // {
    //     const char *path = jni->GetStringUTFChars(stringPath, nullptr);
    //     return std::string(path) + "/chromium_tests_root";
    // }

    // jclass environment = jni->FindClass("org/chromium/base/test/util/UrlUtils");
    // if (environment == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Failed to find Environment";
    //     return "";
    // }

    // jmethodID getDir =
    //     jni->GetStaticMethodID(environment, "getIsolatedTestRoot", "()Ljava/lang/String;");
    // if (getDir == 0)
    // {
    //     std::cerr << "GetExternalStorageDirectory: Failed to get static method";
    //     return "";
    // }

    // stringPath = static_cast<jstring>(jni->CallStaticObjectMethod(environment, getDir));

    exception = jni->ExceptionOccurred();
    if (exception != 0)
    {
        jni->ExceptionDescribe();
        jni->ExceptionClear();
        std::cerr << "GetExternalStorageDirectory: Failed because of exception";
        return "";
    }

    const char *path = jni->GetStringUTFChars(stringPath, nullptr);
    return std::string(path) + "/chromium_tests_root";
}

// static
OSWindow *OSWindow::New()
{
    // There should be only one live instance of AndroidWindow at a time,
    // as there is only one Activity Surface behind it.
    // Creating a new AndroidWindow each time works for ANGLETest,
    // as it destroys an old window before creating a new one.
    // TODO: use GLSurfaceView to support multiple windows
    return new AndroidWindow();
}
