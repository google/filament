//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CaptureReplayTest.cpp:
//   Application that runs replay for testing of capture replay
//

#include "common/debug.h"
#include "common/system_utils.h"
#include "platform/PlatformMethods.h"
#include "traces_export.h"
#include "util/EGLPlatformParameters.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include "util/android/AndroidWindow.h"
#endif

#include <stdint.h>
#include <string.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "frame_capture_test_utils.h"

namespace
{
EGLWindow *gEGLWindow                = nullptr;
constexpr char kResultTag[]          = "*RESULT";
constexpr int kInitializationFailure = -1;
constexpr int kSerializationFailure  = -2;
constexpr int kExitSuccess           = 0;

// This arbitrary value rejects placeholder serialized states. In practice they are many thousands
// of characters long. See frame_capture_utils_mock.cpp for the current placeholder string.
constexpr size_t kTooShortStateLength = 40;

[[maybe_unused]] bool IsGLExtensionEnabled(const std::string &extName)
{
    return angle::CheckExtensionExists(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)),
                                       extName);
}

[[maybe_unused]] void KHRONOS_APIENTRY DebugCallback(GLenum source,
                                                     GLenum type,
                                                     GLuint id,
                                                     GLenum severity,
                                                     GLsizei length,
                                                     const GLchar *message,
                                                     const void *userParam)
{
    printf("%s\n", message);
}

[[maybe_unused]] void LogError(angle::PlatformMethods *platform, const char *errorMessage)
{
    printf("ERR: %s\n", errorMessage);
}

[[maybe_unused]] void LogWarning(angle::PlatformMethods *platform, const char *warningMessage)
{
    printf("WARN: %s\n", warningMessage);
}

[[maybe_unused]] void LogInfo(angle::PlatformMethods *platform, const char *infoMessage)
{
    printf("INFO: %s\n", infoMessage);
}

bool CompareSerializedContexts(const char *capturedSerializedContextState,
                               const char *replaySerializedContextState)
{

    return strcmp(replaySerializedContextState, capturedSerializedContextState) == 0;
}

EGLImage KHRONOS_APIENTRY EGLCreateImage(EGLDisplay display,
                                         EGLContext context,
                                         EGLenum target,
                                         EGLClientBuffer buffer,
                                         const EGLAttrib *attrib_list)
{

    GLWindowContext ctx = reinterpret_cast<GLWindowContext>(context);
    return gEGLWindow->createImage(ctx, target, buffer, attrib_list);
}

EGLImage KHRONOS_APIENTRY EGLCreateImageKHR(EGLDisplay display,
                                            EGLContext context,
                                            EGLenum target,
                                            EGLClientBuffer buffer,
                                            const EGLint *attrib_list)
{

    GLWindowContext ctx = reinterpret_cast<GLWindowContext>(context);
    return gEGLWindow->createImageKHR(ctx, target, buffer, attrib_list);
}

EGLBoolean KHRONOS_APIENTRY EGLDestroyImage(EGLDisplay display, EGLImage image)
{
    return gEGLWindow->destroyImage(image);
}

EGLBoolean KHRONOS_APIENTRY EGLDestroyImageKHR(EGLDisplay display, EGLImage image)
{
    return gEGLWindow->destroyImageKHR(image);
}

EGLSurface KHRONOS_APIENTRY EGLCreatePbufferSurface(EGLDisplay display,
                                                    EGLConfig *config,
                                                    const EGLint *attrib_list)
{
    return gEGLWindow->createPbufferSurface(attrib_list);
}

EGLBoolean KHRONOS_APIENTRY EGLDestroySurface(EGLDisplay display, EGLSurface surface)
{
    return gEGLWindow->destroySurface(surface);
}

EGLBoolean KHRONOS_APIENTRY EGLBindTexImage(EGLDisplay display, EGLSurface surface, EGLint buffer)
{
    return gEGLWindow->bindTexImage(surface, buffer);
}

EGLBoolean KHRONOS_APIENTRY EGLReleaseTexImage(EGLDisplay display,
                                               EGLSurface surface,
                                               EGLint buffer)
{
    return gEGLWindow->releaseTexImage(surface, buffer);
}

EGLBoolean KHRONOS_APIENTRY EGLMakeCurrent(EGLDisplay display,
                                           EGLSurface draw,
                                           EGLSurface read,
                                           EGLContext context)
{
    return gEGLWindow->makeCurrent(draw, read, context);
}
}  // namespace

GenericProc KHRONOS_APIENTRY TraceLoadProc(const char *procName)
{
    if (!gEGLWindow)
    {
        std::cout << "No Window pointer in TraceLoadProc.\n";
        return nullptr;
    }
    else
    {
        if (strcmp(procName, "eglCreateImage") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLCreateImage);
        }
        if (strcmp(procName, "eglCreateImageKHR") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLCreateImageKHR);
        }
        if (strcmp(procName, "eglDestroyImage") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLDestroyImage);
        }
        if (strcmp(procName, "eglDestroyImageKHR") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLDestroyImageKHR);
        }
        if (strcmp(procName, "eglCreatePbufferSurface") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLCreatePbufferSurface);
        }
        if (strcmp(procName, "eglDestroySurface") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLDestroySurface);
        }
        if (strcmp(procName, "eglBindTexImage") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLBindTexImage);
        }
        if (strcmp(procName, "eglReleaseTexImage") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLReleaseTexImage);
        }
        if (strcmp(procName, "eglMakeCurrent") == 0)
        {
            return reinterpret_cast<GenericProc>(EGLMakeCurrent);
        }
        return gEGLWindow->getProcAddress(procName);
    }
}

class CaptureReplayTests
{
  public:
    CaptureReplayTests()
    {
        // Load EGL library so we can initialize the display.
        mEntryPointsLib.reset(
            angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ModuleDir));

        mOSWindow = OSWindow::New();
        mOSWindow->disableErrorMessageDialog();
    }

    ~CaptureReplayTests()
    {
        EGLWindow::Delete(&mEGLWindow);
        OSWindow::Delete(&mOSWindow);
    }

    bool initializeTest(const std::string &execDir, const angle::TraceInfo &traceInfo)
    {
        if (!mOSWindow->initialize(traceInfo.name, traceInfo.drawSurfaceWidth,
                                   traceInfo.drawSurfaceHeight))
        {
            return false;
        }

        mOSWindow->disableErrorMessageDialog();
        mOSWindow->setVisible(true);

        if (mEGLWindow && !mEGLWindow->isContextVersion(traceInfo.contextClientMajorVersion,
                                                        traceInfo.contextClientMinorVersion))
        {
            EGLWindow::Delete(&mEGLWindow);
            mEGLWindow = nullptr;
        }

        if (!mEGLWindow)
        {
            mEGLWindow = EGLWindow::New(traceInfo.contextClientMajorVersion,
                                        traceInfo.contextClientMinorVersion);
        }

        ConfigParameters configParams;
        configParams.redBits     = traceInfo.configRedBits;
        configParams.greenBits   = traceInfo.configGreenBits;
        configParams.blueBits    = traceInfo.configBlueBits;
        configParams.alphaBits   = traceInfo.configAlphaBits;
        configParams.depthBits   = traceInfo.configDepthBits;
        configParams.stencilBits = traceInfo.configStencilBits;

        configParams.clientArraysEnabled   = traceInfo.areClientArraysEnabled;
        configParams.bindGeneratesResource = traceInfo.isBindGeneratesResourcesEnabled;
        configParams.webGLCompatibility    = traceInfo.isWebGLCompatibilityEnabled;
        configParams.robustResourceInit    = traceInfo.isRobustResourceInitEnabled;

        mPlatformParams.renderer   = traceInfo.displayPlatformType;
        mPlatformParams.deviceType = traceInfo.displayDeviceType;
        mPlatformParams.enable(angle::Feature::ForceInitShaderVariables);
        mPlatformParams.enable(angle::Feature::EnableCaptureLimits);

#if defined(ANGLE_ENABLE_ASSERTS)
        mPlatformMethods.logError       = LogError;
        mPlatformMethods.logWarning     = LogWarning;
        mPlatformMethods.logInfo        = LogInfo;
        mPlatformParams.platformMethods = &mPlatformMethods;
#endif  // defined(ANGLE_ENABLE_ASSERTS)

        if (!mEGLWindow->initializeGL(mOSWindow, mEntryPointsLib.get(),
                                      angle::GLESDriverType::AngleEGL, mPlatformParams,
                                      configParams))
        {
            mOSWindow->destroy();
            return false;
        }

        gEGLWindow = mEGLWindow;
        LoadTraceEGL(TraceLoadProc);
        LoadTraceGLES(TraceLoadProc);

        // Disable vsync
        if (!mEGLWindow->setSwapInterval(0))
        {
            cleanupTest();
            return false;
        }

#if defined(ANGLE_ENABLE_ASSERTS)
        if (IsGLExtensionEnabled("GL_KHR_debug"))
        {
            EnableDebugCallback(DebugCallback, nullptr);
        }
#endif

        std::string baseDir = "";
#if defined(ANGLE_TRACE_EXTERNAL_BINARIES)
        baseDir += AndroidWindow::GetApplicationDirectory() + "/angle_traces/";
#endif
        // Load trace
        mTraceLibrary.reset(new angle::TraceLibrary(traceInfo.name, traceInfo, baseDir));
        if (!mTraceLibrary->valid())
        {
            std::cout << "Failed to load trace library: " << traceInfo.name << "\n";
            return false;
        }

        std::stringstream binaryPathStream;
        binaryPathStream << execDir << angle::GetPathSeparator()
                         << ANGLE_CAPTURE_REPLAY_TEST_DATA_DIR;

        mTraceLibrary->setBinaryDataDir(binaryPathStream.str().c_str());

        mTraceLibrary->setupReplay();
        return true;
    }

    void cleanupTest()
    {
        mTraceLibrary->finishReplay();
        mTraceLibrary.reset(nullptr);
        mEGLWindow->destroyGL();
        mOSWindow->destroy();
    }

    void swap() { mEGLWindow->swap(); }

    int runTest(const std::string &exeDir, const angle::TraceInfo &traceInfo)
    {
        if (!initializeTest(exeDir, traceInfo))
        {
            return kInitializationFailure;
        }

        for (uint32_t frame = traceInfo.frameStart; frame <= traceInfo.frameEnd; frame++)
        {
            mTraceLibrary->replayFrame(frame);

            const char *replayedSerializedState =
                reinterpret_cast<const char *>(glGetString(GL_SERIALIZED_CONTEXT_STRING_ANGLE));
            const char *capturedSerializedState = mTraceLibrary->getSerializedContextState(frame);

            if (replayedSerializedState == nullptr ||
                strlen(replayedSerializedState) <= kTooShortStateLength)
            {
                printf("Could not retrieve replay serialized state string.\n");
                return kSerializationFailure;
            }

            if (capturedSerializedState == nullptr ||
                strlen(capturedSerializedState) <= kTooShortStateLength)
            {
                printf("Could not retrieve captured serialized state string.\n");
                return kSerializationFailure;
            }

            // Swap always to allow RenderDoc/other tools to capture frames.
            swap();
            if (!CompareSerializedContexts(replayedSerializedState, capturedSerializedState))
            {
                printf("Serialized contexts differ, saving files.\n");

                std::ostringstream replayName;
                replayName << exeDir << angle::GetPathSeparator() << traceInfo.name
                           << "_ContextReplayed" << frame << ".json";

                std::ofstream debugReplay(replayName.str());
                if (!debugReplay)
                {
                    printf("Error opening debug replay stream.\n");
                }
                else
                {
                    debugReplay << (replayedSerializedState ? replayedSerializedState : "") << "\n";
                    printf("Wrote %s.\n", replayName.str().c_str());
                }

                std::ostringstream captureName;
                captureName << exeDir << angle::GetPathSeparator() << traceInfo.name
                            << "_ContextCaptured" << frame << ".json";

                std::ofstream debugCapture(captureName.str());
                if (!debugCapture)
                {
                    printf("Error opening debug capture stream.\n");
                }
                else
                {
                    debugCapture << (capturedSerializedState ? capturedSerializedState : "")
                                 << "\n";
                    printf("Wrote %s.\n", captureName.str().c_str());
                }

                cleanupTest();
                return kSerializationFailure;
            }
        }
        cleanupTest();
        return kExitSuccess;
    }

    int run(const char *trace)
    {
        std::string startingDirectory = angle::GetCWD().value();

        // Set CWD to executable directory.
        std::string exeDir = angle::GetExecutableDirectory();

        std::stringstream traceJsonPathStream;
        traceJsonPathStream << exeDir << angle::GetPathSeparator()
                            << ANGLE_CAPTURE_REPLAY_TEST_DATA_DIR << angle::GetPathSeparator()
                            << trace << ".json";
        std::string traceJsonPath = traceJsonPathStream.str();

        int result                 = kInitializationFailure;
        angle::TraceInfo traceInfo = {};
        if (!angle::LoadTraceInfoFromJSON(trace, traceJsonPath, &traceInfo))
        {
            std::cout << "Unable to load trace data: " << traceJsonPath << "\n";
        }
        else
        {
            result = runTest(exeDir, traceInfo);
        }
        std::cout << kResultTag << " " << trace << " " << result << "\n";

        angle::SetCWD(startingDirectory.c_str());
        return kExitSuccess;
    }

  private:
    OSWindow *mOSWindow   = nullptr;
    EGLWindow *mEGLWindow = nullptr;
    EGLPlatformParameters mPlatformParams;
    angle::PlatformMethods mPlatformMethods;
    // Handle to the entry point binding library.
    std::unique_ptr<angle::Library> mEntryPointsLib;
    std::unique_ptr<angle::TraceLibrary> mTraceLibrary;
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: capture_replay_tests {trace_label}\n");
        return -1;
    }
    angle::CrashCallback crashCallback = []() {};
    angle::InitCrashHandler(&crashCallback);

    CaptureReplayTests app;
    return app.run(argv[1]);
}
