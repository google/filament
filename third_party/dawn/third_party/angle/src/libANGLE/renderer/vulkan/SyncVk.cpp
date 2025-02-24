//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncVk.cpp:
//    Implements the class methods for SyncVk.
//

#include "libANGLE/renderer/vulkan/SyncVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"

#if !defined(ANGLE_PLATFORM_WINDOWS)
#    include <poll.h>
#    include <unistd.h>
#else
#    include <io.h>
#endif

namespace
{
// Wait for file descriptor to be signaled
VkResult SyncWaitFd(int fd, uint64_t timeoutNs, VkResult timeoutResult = VK_TIMEOUT)
{
#if !defined(ANGLE_PLATFORM_WINDOWS)
    struct pollfd fds;
    int ret;

    // Convert nanoseconds to milliseconds
    int timeoutMs = static_cast<int>(timeoutNs / 1000000);
    // If timeoutNs was non-zero but less than one millisecond, make it a millisecond.
    if (timeoutNs > 0 && timeoutNs < 1000000)
    {
        timeoutMs = 1;
    }

    ASSERT(fd >= 0);

    fds.fd     = fd;
    fds.events = POLLIN;

    do
    {
        ret = poll(&fds, 1, timeoutMs);
        if (ret > 0)
        {
            if (fds.revents & (POLLERR | POLLNVAL))
            {
                return VK_ERROR_UNKNOWN;
            }
            return VK_SUCCESS;
        }
        else if (ret == 0)
        {
            return timeoutResult;
        }
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    return VK_ERROR_UNKNOWN;
#else
    UNREACHABLE();
    return VK_ERROR_UNKNOWN;
#endif
}

// Map VkResult to GLenum
void MapVkResultToGlenum(VkResult vkResult, angle::Result angleResult, void *outResult)
{
    GLenum *glEnumOut = static_cast<GLenum *>(outResult);
    ASSERT(glEnumOut);

    if (angleResult != angle::Result::Continue)
    {
        *glEnumOut = GL_WAIT_FAILED;
        return;
    }

    switch (vkResult)
    {
        case VK_EVENT_SET:
            *glEnumOut = GL_ALREADY_SIGNALED;
            break;
        case VK_SUCCESS:
            *glEnumOut = GL_CONDITION_SATISFIED;
            break;
        case VK_TIMEOUT:
            *glEnumOut = GL_TIMEOUT_EXPIRED;
            break;
        default:
            *glEnumOut = GL_WAIT_FAILED;
            break;
    }
}

// Map VkResult to EGLint
void MapVkResultToEglint(VkResult result, angle::Result angleResult, void *outResult)
{
    EGLint *eglIntOut = static_cast<EGLint *>(outResult);
    ASSERT(eglIntOut);

    if (angleResult != angle::Result::Continue)
    {
        *eglIntOut = EGL_FALSE;
        return;
    }

    switch (result)
    {
        case VK_EVENT_SET:
            // fall through.  EGL doesn't differentiate between event being already set, or set
            // before timeout.
        case VK_SUCCESS:
            *eglIntOut = EGL_CONDITION_SATISFIED_KHR;
            break;
        case VK_TIMEOUT:
            *eglIntOut = EGL_TIMEOUT_EXPIRED_KHR;
            break;
        default:
            *eglIntOut = EGL_FALSE;
            break;
    }
}

}  // anonymous namespace

namespace rx
{
namespace vk
{
SyncHelper::SyncHelper() {}

SyncHelper::~SyncHelper() {}

void SyncHelper::releaseToRenderer(Renderer *renderer) {}

angle::Result SyncHelper::initialize(ContextVk *contextVk, SyncFenceScope scope)
{
    ASSERT(!mUse.valid());
    return contextVk->onSyncObjectInit(this, scope);
}

angle::Result SyncHelper::prepareForClientWait(ErrorContext *context,
                                               ContextVk *contextVk,
                                               bool flushCommands,
                                               uint64_t timeout,
                                               VkResult *resultOut)
{
    // If the event is already set, don't wait
    bool alreadySignaled = false;
    ANGLE_TRY(getStatus(context, contextVk, &alreadySignaled));
    if (alreadySignaled)
    {
        *resultOut = VK_EVENT_SET;
        return angle::Result::Continue;
    }

    // If timeout is zero, there's no need to wait, so return timeout already.
    if (timeout == 0)
    {
        *resultOut = VK_TIMEOUT;
        return angle::Result::Continue;
    }

    // Submit commands if requested
    if (flushCommands && contextVk)
    {
        ANGLE_TRY(contextVk->flushCommandsAndEndRenderPassIfDeferredSyncInit(
            RenderPassClosureReason::SyncObjectClientWait));
    }

    *resultOut = VK_INCOMPLETE;
    return angle::Result::Continue;
}

angle::Result SyncHelper::clientWait(ErrorContext *context,
                                     ContextVk *contextVk,
                                     bool flushCommands,
                                     uint64_t timeout,
                                     MapVkResultToApiType mappingFunction,
                                     void *resultOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SyncHelper::clientWait");

    VkResult status = VK_INCOMPLETE;
    ANGLE_TRY(prepareForClientWait(context, contextVk, flushCommands, timeout, &status));

    if (status != VK_INCOMPLETE)
    {
        mappingFunction(status, angle::Result::Continue, resultOut);
        return angle::Result::Continue;
    }

    Renderer *renderer = context->getRenderer();

    // If we need to perform a CPU wait don't set the resultOut parameter passed into the
    // method, instead set the parameter passed into the unlocked tail call.
    auto clientWaitUnlocked = [renderer, context, mappingFunction, use = mUse,
                               timeout](void *resultOut) {
        ANGLE_TRACE_EVENT0("gpu.angle", "SyncHelper::clientWait block (unlocked)");

        VkResult status = VK_INCOMPLETE;
        angle::Result angleResult =
            renderer->waitForResourceUseToFinishWithUserTimeout(context, use, timeout, &status);
        // Note: resultOut may be nullptr through the glFinishFenceNV path, which does not have a
        // return value.
        if (resultOut != nullptr)
        {
            mappingFunction(status, angleResult, resultOut);
        }
    };

    // Schedule the wait to be run at the tail of the current call.
    egl::Display::GetCurrentThreadUnlockedTailCall()->add(clientWaitUnlocked);
    return angle::Result::Continue;
}

angle::Result SyncHelper::finish(ContextVk *contextVk)
{
    GLenum result;
    return clientWait(contextVk, contextVk, true, UINT64_MAX, MapVkResultToGlenum, &result);
}

angle::Result SyncHelper::serverWait(ContextVk *contextVk)
{
    // If already signaled, no need to wait
    bool alreadySignaled = false;
    ANGLE_TRY(getStatus(contextVk, contextVk, &alreadySignaled));
    if (alreadySignaled)
    {
        return angle::Result::Continue;
    }

    // Every resource already tracks its usage and issues the appropriate barriers, so there's
    // really nothing to do here.  An execution barrier is issued to strictly satisfy what the
    // application asked for.
    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer({}, &commandBuffer));
    commandBuffer->pipelineBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0,
                                   nullptr);
    return angle::Result::Continue;
}

angle::Result SyncHelper::getStatus(ErrorContext *context, ContextVk *contextVk, bool *signaledOut)
{
    // Submit commands if it was deferred on the context that issued the sync object
    ANGLE_TRY(submitSyncIfDeferred(contextVk, RenderPassClosureReason::SyncObjectClientWait));
    ASSERT(mUse.valid());
    Renderer *renderer = context->getRenderer();
    if (renderer->hasResourceUseFinished(mUse))
    {
        *signaledOut = true;
    }
    else
    {
        // Check completed commands once before returning, perhaps the serial is actually already
        // finished.
        // We don't call checkCompletedCommandsAndCleanup() to cleanup finished commands immediately
        // if isAsyncCommandBufferResetAndGarbageCleanupEnabled feature is turned off.
        // Because when that feature is turned off, vkResetCommandBuffer() is called in cleanup
        // step, and it must take the CommandPoolAccess::mCmdPoolMutex lock, see details in
        // CommandPoolAccess::collectPrimaryCommandBuffer. This means the cleanup step can
        // be blocked by command buffer recording if another thread calls
        // CommandPoolAccess::flushRenderPassCommands(), which is against EGL spec where
        // eglClientWaitSync() should return immediately with timeout == 0.
        if (renderer->isAsyncCommandBufferResetAndGarbageCleanupEnabled())
        {
            ANGLE_TRY(renderer->checkCompletedCommandsAndCleanup(context));
        }
        else
        {
            ANGLE_TRY(renderer->checkCompletedCommands(context));
        }

        *signaledOut = renderer->hasResourceUseFinished(mUse);
    }
    return angle::Result::Continue;
}

angle::Result SyncHelper::submitSyncIfDeferred(ContextVk *contextVk, RenderPassClosureReason reason)
{
    if (contextVk == nullptr)
    {
        return angle::Result::Continue;
    }

    if (contextVk->getRenderer()->hasResourceUseSubmitted(mUse))
    {
        return angle::Result::Continue;
    }

    // The submission of a sync object may be deferred to allow further optimizations to an open
    // render pass before a submission happens for another reason.  If the sync object is being
    // waited on by the current context, the application must have used GL_SYNC_FLUSH_COMMANDS_BIT.
    // However, when waited on by other contexts, the application must have ensured the original
    // context is flushed.  Due to deferred flushes, a glFlush is not sufficient to guarantee this.
    //
    // Deferring the submission is restricted to non-EGL sync objects, so it's sufficient to ensure
    // that the contexts in the share group issue their deferred flushes.
    for (auto context : contextVk->getShareGroup()->getContexts())
    {
        ContextVk *sharedContextVk = vk::GetImpl(context.second);
        if (sharedContextVk->hasUnsubmittedUse(mUse))
        {
            ANGLE_TRY(sharedContextVk->flushCommandsAndEndRenderPassIfDeferredSyncInit(reason));
            break;
        }
    }
    // Note mUse could still be invalid here if it is inserted on a fresh created context, i.e.,
    // fence is tracking nothing and is finished when inserted..
    ASSERT(contextVk->getRenderer()->hasResourceUseSubmitted(mUse));

    return angle::Result::Continue;
}

ExternalFence::ExternalFence()
    : mDevice(VK_NULL_HANDLE), mFenceFdStatus(VK_INCOMPLETE), mFenceFd(kInvalidFenceFd)
{}

ExternalFence::~ExternalFence()
{
    if (mDevice != VK_NULL_HANDLE)
    {
        mFence.destroy(mDevice);
    }

    if (mFenceFd != kInvalidFenceFd)
    {
        close(mFenceFd);
    }
}

VkResult ExternalFence::init(VkDevice device, const VkFenceCreateInfo &createInfo)
{
    ASSERT(device != VK_NULL_HANDLE);
    ASSERT(mFenceFdStatus == VK_INCOMPLETE && mFenceFd == kInvalidFenceFd);
    ASSERT(mDevice == VK_NULL_HANDLE);
    mDevice = device;
    return mFence.init(device, createInfo);
}

void ExternalFence::init(int fenceFd)
{
    ASSERT(fenceFd != kInvalidFenceFd);
    ASSERT(mFenceFdStatus == VK_INCOMPLETE && mFenceFd == kInvalidFenceFd);
    mFenceFdStatus = VK_SUCCESS;
    mFenceFd       = fenceFd;
}

VkResult ExternalFence::getStatus(VkDevice device) const
{
    if (mFenceFdStatus == VK_SUCCESS)
    {
        return SyncWaitFd(mFenceFd, 0, VK_NOT_READY);
    }
    return mFence.getStatus(device);
}

VkResult ExternalFence::wait(VkDevice device, uint64_t timeout) const
{
    if (mFenceFdStatus == VK_SUCCESS)
    {
        return SyncWaitFd(mFenceFd, timeout);
    }
    return mFence.wait(device, timeout);
}

void ExternalFence::exportFd(VkDevice device, const VkFenceGetFdInfoKHR &fenceGetFdInfo)
{
    ASSERT(mFenceFdStatus == VK_INCOMPLETE && mFenceFd == kInvalidFenceFd);
    mFenceFdStatus = mFence.exportFd(device, fenceGetFdInfo, &mFenceFd);
    ASSERT(mFenceFdStatus != VK_INCOMPLETE);
}

SyncHelperNativeFence::SyncHelperNativeFence()
{
    mExternalFence = std::make_shared<ExternalFence>();
}

SyncHelperNativeFence::~SyncHelperNativeFence() {}

void SyncHelperNativeFence::releaseToRenderer(Renderer *renderer)
{
    mExternalFence.reset();
}

angle::Result SyncHelperNativeFence::initializeWithFd(ContextVk *contextVk, int inFd)
{
    ASSERT(inFd >= kInvalidFenceFd);

    // If valid FD provided by application - import it to fence.
    if (inFd > kInvalidFenceFd)
    {
        // File descriptor ownership: EGL_ANDROID_native_fence_sync
        // Whenever a file descriptor is passed into or returned from an
        // EGL call in this extension, ownership of that file descriptor is
        // transferred. The recipient of the file descriptor must close it when it is
        // no longer needed, and the provider of the file descriptor must dup it
        // before providing it if they require continued use of the native fence.
        mExternalFence->init(inFd);
        return angle::Result::Continue;
    }

    Renderer *renderer = contextVk->getRenderer();
    VkDevice device    = renderer->getDevice();

    VkExportFenceCreateInfo exportCreateInfo = {};
    exportCreateInfo.sType                   = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
    exportCreateInfo.pNext                   = nullptr;
    exportCreateInfo.handleTypes             = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;

    // Create fenceInfo base.
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags             = 0;
    fenceCreateInfo.pNext             = &exportCreateInfo;

    // Initialize/create a VkFence handle
    ANGLE_VK_TRY(contextVk, mExternalFence->init(device, fenceCreateInfo));

    // invalid FD provided by application - create one with fence.
    /*
      Spec: "When a fence sync object is created or when an EGL native fence sync
      object is created with the EGL_SYNC_NATIVE_FENCE_FD_ANDROID attribute set to
      EGL_NO_NATIVE_FENCE_FD_ANDROID, eglCreateSyncKHR also inserts a fence command
      into the command stream of the bound client API's current context and associates it
      with the newly created sync object.
    */
    // Flush current pending set of commands providing the fence...
    ANGLE_TRY(contextVk->flushAndSubmitCommands(nullptr, &mExternalFence,
                                                RenderPassClosureReason::SyncObjectWithFdInit));

    ANGLE_VK_TRY(contextVk, mExternalFence->getFenceFdStatus());

    return angle::Result::Continue;
}

angle::Result SyncHelperNativeFence::prepareForClientWait(ErrorContext *context,
                                                          ContextVk *contextVk,
                                                          bool flushCommands,
                                                          uint64_t timeout,
                                                          VkResult *resultOut)
{
    // If already signaled, don't wait
    bool alreadySignaled = false;
    ANGLE_TRY(getStatus(context, contextVk, &alreadySignaled));
    if (alreadySignaled)
    {
        *resultOut = VK_SUCCESS;
        return angle::Result::Continue;
    }

    // If timeout is zero, there's no need to wait, so return timeout already.
    if (timeout == 0)
    {
        *resultOut = VK_TIMEOUT;
        return angle::Result::Continue;
    }

    if (flushCommands && contextVk)
    {
        ANGLE_TRY(contextVk->flushAndSubmitCommands(nullptr, nullptr,
                                                    RenderPassClosureReason::SyncObjectClientWait));
    }

    *resultOut = VK_INCOMPLETE;
    return angle::Result::Continue;
}

angle::Result SyncHelperNativeFence::clientWait(ErrorContext *context,
                                                ContextVk *contextVk,
                                                bool flushCommands,
                                                uint64_t timeout,
                                                MapVkResultToApiType mappingFunction,
                                                void *resultOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SyncHelperNativeFence::clientWait");

    VkResult status = VK_INCOMPLETE;
    ANGLE_TRY(prepareForClientWait(context, contextVk, flushCommands, timeout, &status));

    if (status != VK_INCOMPLETE)
    {
        mappingFunction(status, angle::Result::Continue, resultOut);
        return angle::Result::Continue;
    }

    Renderer *renderer = context->getRenderer();

    auto clientWaitUnlocked = [device = renderer->getDevice(), fence = mExternalFence,
                               mappingFunction, timeout](void *resultOut) {
        ANGLE_TRACE_EVENT0("gpu.angle", "SyncHelperNativeFence::clientWait block (unlocked)");
        ASSERT(resultOut);

        VkResult status = fence->wait(device, timeout);
        mappingFunction(status, angle::Result::Continue, resultOut);
    };

    egl::Display::GetCurrentThreadUnlockedTailCall()->add(clientWaitUnlocked);
    return angle::Result::Continue;
}

angle::Result SyncHelperNativeFence::serverWait(ContextVk *contextVk)
{
    Renderer *renderer = contextVk->getRenderer();

    // If already signaled, no need to wait
    bool alreadySignaled = false;
    ANGLE_TRY(getStatus(contextVk, contextVk, &alreadySignaled));
    if (alreadySignaled)
    {
        return angle::Result::Continue;
    }

    VkDevice device = renderer->getDevice();
    DeviceScoped<Semaphore> waitSemaphore(device);
    // Wait semaphore for next vkQueueSubmit().
    // Create a Semaphore with imported fenceFd.
    ANGLE_VK_TRY(contextVk, waitSemaphore.get().init(device));

    VkImportSemaphoreFdInfoKHR importFdInfo = {};
    importFdInfo.sType                      = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    importFdInfo.semaphore                  = waitSemaphore.get().getHandle();
    importFdInfo.flags                      = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR;
    importFdInfo.handleType                 = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    importFdInfo.fd                         = dup(mExternalFence->getFenceFd());
    ANGLE_VK_TRY(contextVk, waitSemaphore.get().importFd(device, importFdInfo));

    // Add semaphore to next submit job.
    contextVk->addWaitSemaphore(waitSemaphore.get().getHandle(),
                                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    contextVk->addGarbage(&waitSemaphore.get());  // This releases the handle.
    return angle::Result::Continue;
}

angle::Result SyncHelperNativeFence::getStatus(ErrorContext *context,
                                               ContextVk *contextVk,
                                               bool *signaledOut)
{
    VkResult result = mExternalFence->getStatus(context->getDevice());
    if (result != VK_NOT_READY)
    {
        ANGLE_VK_TRY(context, result);
    }
    *signaledOut = (result == VK_SUCCESS);
    return angle::Result::Continue;
}

angle::Result SyncHelperNativeFence::dupNativeFenceFD(ErrorContext *context, int *fdOut) const
{
    if (mExternalFence->getFenceFd() == kInvalidFenceFd)
    {
        return angle::Result::Stop;
    }

    *fdOut = dup(mExternalFence->getFenceFd());

    return angle::Result::Continue;
}

}  // namespace vk

SyncVk::SyncVk() : SyncImpl() {}

SyncVk::~SyncVk() {}

void SyncVk::onDestroy(const gl::Context *context)
{
    mSyncHelper.releaseToRenderer(vk::GetImpl(context)->getRenderer());
}

angle::Result SyncVk::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE);
    ASSERT(flags == 0);

    return mSyncHelper.initialize(vk::GetImpl(context), SyncFenceScope::CurrentContextToShareGroup);
}

angle::Result SyncVk::clientWait(const gl::Context *context,
                                 GLbitfield flags,
                                 GLuint64 timeout,
                                 GLenum *outResult)
{
    ContextVk *contextVk = vk::GetImpl(context);

    ASSERT((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) == 0);

    bool flush = (flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0;

    return mSyncHelper.clientWait(contextVk, contextVk, flush, static_cast<uint64_t>(timeout),
                                  MapVkResultToGlenum, outResult);
}

angle::Result SyncVk::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    ASSERT(flags == 0);
    ASSERT(timeout == GL_TIMEOUT_IGNORED);

    ContextVk *contextVk = vk::GetImpl(context);
    return mSyncHelper.serverWait(contextVk);
}

angle::Result SyncVk::getStatus(const gl::Context *context, GLint *outResult)
{
    ContextVk *contextVk = vk::GetImpl(context);
    bool signaled        = false;
    ANGLE_TRY(mSyncHelper.getStatus(contextVk, contextVk, &signaled));

    *outResult = signaled ? GL_SIGNALED : GL_UNSIGNALED;
    return angle::Result::Continue;
}

EGLSyncVk::EGLSyncVk() : EGLSyncImpl(), mSyncHelper(nullptr) {}

EGLSyncVk::~EGLSyncVk() {}

void EGLSyncVk::onDestroy(const egl::Display *display)
{
    mSyncHelper->releaseToRenderer(vk::GetImpl(display)->getRenderer());
}

egl::Error EGLSyncVk::initialize(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLenum type,
                                 const egl::AttributeMap &attribs)
{
    ASSERT(context != nullptr);

    switch (type)
    {
        case EGL_SYNC_FENCE_KHR:
        case EGL_SYNC_GLOBAL_FENCE_ANGLE:
        {
            vk::SyncHelper *syncHelper = new vk::SyncHelper();
            mSyncHelper.reset(syncHelper);
            const SyncFenceScope scope = type == EGL_SYNC_GLOBAL_FENCE_ANGLE
                                             ? SyncFenceScope::AllContextsToAllContexts
                                             : SyncFenceScope::CurrentContextToAllContexts;
            if (syncHelper->initialize(vk::GetImpl(context), scope) == angle::Result::Stop)
            {
                return egl::Error(EGL_BAD_ALLOC, "eglCreateSyncKHR failed to create sync object");
            }
            return egl::NoError();
        }
        case EGL_SYNC_NATIVE_FENCE_ANDROID:
        {
            vk::SyncHelperNativeFence *syncHelper = new vk::SyncHelperNativeFence();
            mSyncHelper.reset(syncHelper);
            EGLint nativeFenceFd =
                attribs.getAsInt(EGL_SYNC_NATIVE_FENCE_FD_ANDROID, EGL_NO_NATIVE_FENCE_FD_ANDROID);
            return angle::ToEGL(syncHelper->initializeWithFd(vk::GetImpl(context), nativeFenceFd),
                                EGL_BAD_ALLOC);
        }
        default:
            UNREACHABLE();
            return egl::Error(EGL_BAD_ALLOC);
    }
}

egl::Error EGLSyncVk::clientWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags,
                                 EGLTime timeout,
                                 EGLint *outResult)
{
    ASSERT((flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) == 0);

    bool flush = (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) != 0;

    ContextVk *contextVk = context != nullptr && flush ? vk::GetImpl(context) : nullptr;
    if (mSyncHelper->clientWait(vk::GetImpl(display), contextVk, flush,
                                static_cast<uint64_t>(timeout), MapVkResultToEglint,
                                outResult) == angle::Result::Stop)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    return egl::NoError();
}

egl::Error EGLSyncVk::serverWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags)
{
    // Server wait requires a valid bound context.
    ASSERT(context);

    // No flags are currently implemented.
    ASSERT(flags == 0);

    ContextVk *contextVk = vk::GetImpl(context);
    return angle::ToEGL(mSyncHelper->serverWait(contextVk), EGL_BAD_ALLOC);
}

egl::Error EGLSyncVk::getStatus(const egl::Display *display, EGLint *outStatus)
{
    bool signaled = false;
    if (mSyncHelper->getStatus(vk::GetImpl(display), nullptr, &signaled) == angle::Result::Stop)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    *outStatus = signaled ? EGL_SIGNALED_KHR : EGL_UNSIGNALED_KHR;
    return egl::NoError();
}

egl::Error EGLSyncVk::dupNativeFenceFD(const egl::Display *display, EGLint *fdOut) const
{
    return angle::ToEGL(mSyncHelper->dupNativeFenceFD(vk::GetImpl(display), fdOut),
                        EGL_BAD_PARAMETER);
}

}  // namespace rx
