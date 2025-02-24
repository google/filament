//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.cpp : Implements functions for querying the thread-local GL and EGL state.

#include "libGLESv2/global_state.h"

#include "common/debug.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Thread.h"
#include "libGLESv2/egl_stubs_autogen.h"
#include "libGLESv2/resource.h"

#include <atomic>
#if defined(ANGLE_PLATFORM_APPLE)
#    include <dispatch/dispatch.h>
#endif
namespace egl
{
namespace
{
ANGLE_REQUIRE_CONSTANT_INIT gl::Context *g_LastContext(nullptr);
static_assert(std::is_trivially_destructible<decltype(g_LastContext)>::value,
              "global last context is not trivially destructible");

bool g_EGLValidationEnabled = true;

// Called only on Android platform
[[maybe_unused]] void ThreadCleanupCallback(void *ptr)
{
    egl::Thread *thread = static_cast<egl::Thread *>(ptr);
    ASSERT(thread);
    ANGLE_SCOPED_GLOBAL_EGL_AND_EGL_SYNC_LOCK();
    // ReleaseThread() and makeCurrent() inside will perform:
    // - destroy Context if it was already marked for destruction;
    // - invalidate Context if Display was already terminated by app;
    // - perform Display termination when no active threads (and current Contexts);
    // - release any invalid objects in case if Display was not terminated.
    (void)ReleaseThread(thread);
}

Thread *AllocateCurrentThread()
{
    Thread *thread;
    {
        // Global thread intentionally leaked.
        // Display TLS data is also intentionally leaked.
        ANGLE_SCOPED_DISABLE_LSAN();
        thread = new Thread();
#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
        SetCurrentThreadTLS(thread);
#else
        gCurrentThread = thread;
#endif

        Display::InitTLS();
    }

    // Initialize current-context TLS slot
    gl::SetCurrentValidContext(nullptr);

#if defined(ANGLE_PLATFORM_ANDROID)
    static pthread_once_t keyOnce                 = PTHREAD_ONCE_INIT;
    static angle::TLSIndex gThreadCleanupTLSIndex = TLS_INVALID_INDEX;

    // Create thread cleanup TLS slot
    auto CreateThreadCleanupTLSIndex = []() {
        gThreadCleanupTLSIndex = angle::CreateTLSIndex(ThreadCleanupCallback);
    };
    pthread_once(&keyOnce, CreateThreadCleanupTLSIndex);
    ASSERT(gThreadCleanupTLSIndex != TLS_INVALID_INDEX);

    // Initialize thread cleanup TLS slot
    angle::SetTLSValue(gThreadCleanupTLSIndex, thread);
#endif  // ANGLE_PLATFORM_ANDROID

    ASSERT(thread);
    return thread;
}

}  // anonymous namespace

#if defined(ANGLE_PLATFORM_APPLE)
// TODO(angleproject:6479): Due to a bug in Apple's dyld loader, `thread_local` will cause
// excessive memory use. Temporarily avoid it by using pthread's thread
// local storage instead.
// https://bugs.webkit.org/show_bug.cgi?id=228240

static angle::TLSIndex GetCurrentThreadTLSIndex()
{
    static angle::TLSIndex CurrentThreadIndex = TLS_INVALID_INDEX;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
      ASSERT(CurrentThreadIndex == TLS_INVALID_INDEX);
      CurrentThreadIndex = angle::CreateTLSIndex(nullptr);
    });
    return CurrentThreadIndex;
}
Thread *GetCurrentThreadTLS()
{
    angle::TLSIndex CurrentThreadIndex = GetCurrentThreadTLSIndex();
    ASSERT(CurrentThreadIndex != TLS_INVALID_INDEX);
    return static_cast<Thread *>(angle::GetTLSValue(CurrentThreadIndex));
}
void SetCurrentThreadTLS(Thread *thread)
{
    angle::TLSIndex CurrentThreadIndex = GetCurrentThreadTLSIndex();
    ASSERT(CurrentThreadIndex != TLS_INVALID_INDEX);
    angle::SetTLSValue(CurrentThreadIndex, thread);
}
#elif defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
static thread_local Thread *gCurrentThread = nullptr;
Thread *GetCurrentThreadTLS()
{
    return gCurrentThread;
}
void SetCurrentThreadTLS(Thread *thread)
{
    gCurrentThread = thread;
}
#else
thread_local Thread *gCurrentThread = nullptr;
#endif

gl::Context *GetGlobalLastContext()
{
    return g_LastContext;
}

void SetGlobalLastContext(gl::Context *context)
{
    g_LastContext = context;
}

// This function causes an MSAN false positive, which is muted. See https://crbug.com/1211047
// It also causes a flaky false positive in TSAN. http://crbug.com/1223970
ANGLE_NO_SANITIZE_MEMORY ANGLE_NO_SANITIZE_THREAD Thread *GetCurrentThread()
{
#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
    Thread *current = GetCurrentThreadTLS();
#else
    Thread *current = gCurrentThread;
#endif
    return (current ? current : AllocateCurrentThread());
}

void SetContextCurrent(Thread *thread, gl::Context *context)
{
#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
    Thread *currentThread = GetCurrentThreadTLS();
#else
    Thread *currentThread = gCurrentThread;
#endif
    ASSERT(currentThread);
    currentThread->setCurrent(context);

    gl::SetCurrentValidContext(context);

#if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
    DirtyContextIfNeeded(context);
#endif
}

ScopedSyncCurrentContextFromThread::ScopedSyncCurrentContextFromThread(egl::Thread *thread)
    : mThread(thread)
{
    ASSERT(mThread);
}

ScopedSyncCurrentContextFromThread::~ScopedSyncCurrentContextFromThread()
{
    SetContextCurrent(mThread, mThread->getContext());
}

void SetEGLValidationEnabled(bool enabled)
{
    g_EGLValidationEnabled = enabled;
}

bool IsEGLValidationEnabled()
{
    return g_EGLValidationEnabled;
}

}  // namespace egl

namespace gl
{
void GenerateContextLostErrorOnContext(Context *context)
{
    if (context && context->isContextLost())
    {
        context->getMutableErrorSetForValidation()->validationError(
            angle::EntryPoint::Invalid, GL_CONTEXT_LOST, err::kContextLost);
    }
}

void GenerateContextLostErrorOnCurrentGlobalContext()
{
    // If the client starts issuing GL calls before ANGLE has had a chance to initialize,
    // GenerateContextLostErrorOnCurrentGlobalContext can be called before AllocateCurrentThread has
    // had a chance to run. Calling GetCurrentThread() ensures that TLS thread state is set up.
    egl::GetCurrentThread();

    GenerateContextLostErrorOnContext(GetGlobalContext());
}
}  // namespace gl

#if defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
namespace egl
{

namespace
{

void DeallocateCurrentThread()
{
    SafeDelete(gCurrentThread);
}

bool InitializeProcess()
{
    EnsureDebugAllocated();
    AllocateGlobalMutex();
    return AllocateCurrentThread() != nullptr;
}

void TerminateProcess()
{
    DeallocateDebug();
    DeallocateGlobalMutex();
    DeallocateCurrentThread();
}

}  // anonymous namespace

}  // namespace egl

namespace
{
// The following WaitForDebugger code is based on SwiftShader. See:
// https://cs.chromium.org/chromium/src/third_party/swiftshader/src/Vulkan/main.cpp
#    if defined(ANGLE_ENABLE_ASSERTS) && !defined(ANGLE_ENABLE_WINDOWS_UWP)
INT_PTR CALLBACK DebuggerWaitDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            ::GetWindowRect(GetDesktopWindow(), &rect);
            ::SetWindowPos(hwnd, HWND_TOP, rect.right / 2, rect.bottom / 2, 0, 0, SWP_NOSIZE);
            ::SetTimer(hwnd, 1, 100, NULL);
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDCANCEL)
            {
                ::EndDialog(hwnd, 0);
            }
            break;
        case WM_TIMER:
            if (angle::IsDebuggerAttached())
            {
                ::EndDialog(hwnd, 0);
            }
    }

    return FALSE;
}

void WaitForDebugger(HINSTANCE instance)
{
    if (angle::IsDebuggerAttached())
        return;

    HRSRC dialog = ::FindResourceA(instance, MAKEINTRESOURCEA(IDD_DIALOG1), MAKEINTRESOURCEA(5));
    if (!dialog)
    {
        printf("Error finding wait for debugger dialog. Error %lu.\n", ::GetLastError());
        return;
    }

    DLGTEMPLATE *dialogTemplate = reinterpret_cast<DLGTEMPLATE *>(::LoadResource(instance, dialog));
    ::DialogBoxIndirectA(instance, dialogTemplate, NULL, DebuggerWaitDialogProc);
}
#    else
void WaitForDebugger(HINSTANCE instance) {}
#    endif  // defined(ANGLE_ENABLE_ASSERTS) && !defined(ANGLE_ENABLE_WINDOWS_UWP)
}  // namespace

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            if (angle::GetBoolEnvironmentVar("ANGLE_WAIT_FOR_DEBUGGER"))
            {
                WaitForDebugger(instance);
            }
            return static_cast<BOOL>(egl::InitializeProcess());

        case DLL_THREAD_ATTACH:
            return static_cast<BOOL>(egl::AllocateCurrentThread() != nullptr);

        case DLL_THREAD_DETACH:
            egl::DeallocateCurrentThread();
            break;

        case DLL_PROCESS_DETACH:
            egl::TerminateProcess();
            break;
    }

    return TRUE;
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
