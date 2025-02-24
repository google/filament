//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// egl_context_lock_autogen.h:
//   Context Lock functions for the EGL entry points.

#ifndef LIBGLESV2_EGL_CONTEXT_LOCK_IMPL_H_
#define LIBGLESV2_EGL_CONTEXT_LOCK_IMPL_H_

#include "libGLESv2/egl_context_lock_autogen.h"

namespace egl
{
namespace priv
{
ANGLE_INLINE bool ClientWaitSyncHasFlush(EGLint flags)
{
    return (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) != 0;
}
}  // namespace priv

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ChooseConfig(Thread *thread,
                                                                egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CopyBuffers(Thread *thread,
                                                               egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateContext(Thread *thread,
                                                                 egl::Display *dpyPacked,
                                                                 gl::ContextID share_contextPacked)
{
    return TryLockContext(dpyPacked, share_contextPacked);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreatePbufferSurface(Thread *thread,
                                                                        egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreatePixmapSurface(Thread *thread,
                                                                       egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateWindowSurface(Thread *thread,
                                                                       egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroyContext(Thread *thread,
                                                                  egl::Display *dpyPacked,
                                                                  gl::ContextID ctxPacked)
{
    // Explicit lock in egl::Display::destroyContext()/makeCurrent()
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroySurface(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetConfigAttrib(Thread *thread,
                                                                   egl::Display *dpyPacked,
                                                                   EGLint attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetConfigs(Thread *thread,
                                                              egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetCurrentDisplay(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetCurrentSurface(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetDisplay(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetError(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetProcAddress(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_Initialize(Thread *thread,
                                                              egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_MakeCurrent(Thread *thread,
                                                               egl::Display *dpyPacked,
                                                               gl::ContextID ctxPacked)
{
    // Explicit lock in egl::Display::makeCurrent()
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryContext(Thread *thread,
                                                                egl::Display *dpyPacked,
                                                                gl::ContextID ctxPacked,
                                                                EGLint attribute)
{
    return TryLockContext(dpyPacked, ctxPacked);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryString(Thread *thread,
                                                               egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QuerySurface(Thread *thread,
                                                                egl::Display *dpyPacked,
                                                                EGLint attribute)
{
    switch (attribute)
    {
        // EGL_BUFFER_AGE_EXT uses current Context and therefore requires the lock.
        case EGL_BUFFER_AGE_EXT:
            return TryLockCurrentContext(thread);
        // Other attributes are not using Context, therefore lock is not required.
        default:
            return {};
    }
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_SwapBuffers(Thread *thread,
                                                               egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_Terminate(Thread *thread,
                                                             egl::Display *dpyPacked)
{
    // Accesses only not curent Contexts
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_WaitGL(Thread *thread)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_WaitNative(Thread *thread)
{
    return TryLockCurrentContext(thread);
}

// EGL 1.1
ANGLE_INLINE ScopedContextMutexLock GetContextLock_BindTexImage(Thread *thread,
                                                                egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ReleaseTexImage(Thread *thread,
                                                                   egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_SurfaceAttrib(Thread *thread,
                                                                 egl::Display *dpyPacked,
                                                                 EGLint attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_SwapInterval(Thread *thread,
                                                                egl::Display *dpyPacked)
{
    // Only checked in Validation that we have current Context
    return {};
}

// EGL 1.2
ANGLE_INLINE ScopedContextMutexLock GetContextLock_BindAPI(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreatePbufferFromClientBuffer(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryAPI(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ReleaseThread(Thread *thread)
{
    // Explicit lock in egl::Display::makeCurrent()
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_WaitClient(Thread *thread)
{
    return TryLockCurrentContext(thread);
}

// EGL 1.4
ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetCurrentContext(Thread *thread)
{
    return {};
}

// EGL 1.5
ANGLE_INLINE ScopedContextMutexLock GetContextLock_ClientWaitSync(Thread *thread,
                                                                  egl::Display *dpyPacked,
                                                                  EGLint flags)
{
    if (priv::ClientWaitSyncHasFlush(flags))
    {
        return TryLockCurrentContext(thread);
    }
    else
    {
        return {};
    }
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateImage(Thread *thread,
                                                               egl::Display *dpyPacked,
                                                               gl::ContextID ctxPacked)
{
    return TryLockContext(dpyPacked, ctxPacked);
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreatePlatformPixmapSurface(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreatePlatformWindowSurface(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateSync(Thread *thread,
                                                              egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroyImage(Thread *thread,
                                                                egl::Display *dpyPacked)
{
    // Explicit lock in egl::Display::destroyImageImpl()
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroySync(Thread *thread,
                                                               egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetPlatformDisplay(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetSyncAttrib(Thread *thread,
                                                                 egl::Display *dpyPacked,
                                                                 EGLint attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_WaitSync(Thread *thread,
                                                            egl::Display *dpyPacked,
                                                            EGLint flags)

{
    return TryLockCurrentContext(thread);
}

// EGL_ANDROID_blob_cache
ANGLE_INLINE ScopedContextMutexLock GetContextLock_SetBlobCacheFuncsANDROID(Thread *thread,
                                                                            egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANDROID_create_native_client_buffer
ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateNativeClientBufferANDROID(Thread *thread)
{
    return {};
}

// EGL_ANDROID_get_frame_timestamps
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_GetCompositorTimingSupportedANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_GetCompositorTimingANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetNextFrameIdANDROID(Thread *thread,
                                                                         egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_GetFrameTimestampSupportedANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_GetFrameTimestampsANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANDROID_get_native_client_buffer
ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetNativeClientBufferANDROID(Thread *thread)
{
    return {};
}

// EGL_ANDROID_native_fence_sync
ANGLE_INLINE ScopedContextMutexLock GetContextLock_DupNativeFenceFDANDROID(Thread *thread,
                                                                           egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANDROID_presentation_time
ANGLE_INLINE ScopedContextMutexLock GetContextLock_PresentationTimeANDROID(Thread *thread,
                                                                           egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_device_creation
ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateDeviceANGLE(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ReleaseDeviceANGLE(Thread *thread)
{
    return {};
}

// EGL_ANGLE_device_vulkan
ANGLE_INLINE ScopedContextMutexLock GetContextLock_LockVulkanQueueANGLE(Thread *thread,
                                                                        egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_UnlockVulkanQueueANGLE(Thread *thread,
                                                                          egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_external_context_and_surface
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_AcquireExternalContextANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_ReleaseExternalContextANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_ANGLE_feature_control
ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryStringiANGLE(Thread *thread,
                                                                     egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDisplayAttribANGLE(Thread *thread,
                                                                           egl::Display *dpyPacked,
                                                                           EGLint attribute)
{
    return {};
}

// EGL_ANGLE_metal_shared_event_sync
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CopyMetalSharedEventANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_power_preference
ANGLE_INLINE ScopedContextMutexLock GetContextLock_ReleaseHighPowerGPUANGLE(Thread *thread,
                                                                            egl::Display *dpyPacked,
                                                                            gl::ContextID ctxPacked)
{
    return TryLockContext(dpyPacked, ctxPacked);
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_ReacquireHighPowerGPUANGLE(Thread *thread,
                                          egl::Display *dpyPacked,
                                          gl::ContextID ctxPacked)
{
    return TryLockContext(dpyPacked, ctxPacked);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_HandleGPUSwitchANGLE(Thread *thread,
                                                                        egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ForceGPUSwitchANGLE(Thread *thread,
                                                                       egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_prepare_swap_buffers
ANGLE_INLINE ScopedContextMutexLock GetContextLock_PrepareSwapBuffersANGLE(Thread *thread,
                                                                           egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_ANGLE_program_cache_control
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_ProgramCacheGetAttribANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ProgramCacheQueryANGLE(Thread *thread,
                                                                          egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_ProgramCachePopulateANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_ProgramCacheResizeANGLE(Thread *thread,
                                                                           egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_query_surface_pointer
ANGLE_INLINE ScopedContextMutexLock GetContextLock_QuerySurfacePointerANGLE(Thread *thread,
                                                                            egl::Display *dpyPacked,
                                                                            EGLint attribute)
{
    return {};
}

// EGL_ANGLE_stream_producer_d3d_texture
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreateStreamProducerD3DTextureANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_StreamPostD3DTextureANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_swap_with_frame_token
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_SwapBuffersWithFrameTokenANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_ANGLE_sync_control_rate
ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetMscRateANGLE(Thread *thread,
                                                                   egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_vulkan_image
ANGLE_INLINE ScopedContextMutexLock GetContextLock_ExportVkImageANGLE(Thread *thread,
                                                                      egl::Display *dpyPacked)
{
    return {};
}

// EGL_ANGLE_wait_until_work_scheduled
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_WaitUntilWorkScheduledANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

// EGL_CHROMIUM_sync_control
ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetSyncValuesCHROMIUM(Thread *thread,
                                                                         egl::Display *dpyPacked)
{
    return {};
}

// EGL_EXT_device_query
ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDeviceAttribEXT(Thread *thread,
                                                                        EGLint attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDeviceStringEXT(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDisplayAttribEXT(Thread *thread,
                                                                         egl::Display *dpyPacked,
                                                                         EGLint attribute)
{
    return {};
}

// EGL_EXT_image_dma_buf_import_modifiers
ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDmaBufFormatsEXT(Thread *thread,
                                                                         egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDmaBufModifiersEXT(Thread *thread,
                                                                           egl::Display *dpyPacked)
{
    return {};
}

// EGL_EXT_platform_base
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreatePlatformPixmapSurfaceEXT(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_CreatePlatformWindowSurfaceEXT(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetPlatformDisplayEXT(Thread *thread)
{
    return {};
}

// EGL_EXT_surface_compression
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_QuerySupportedCompressionRatesEXT(Thread *thread, egl::Display *dpyPacked)
{
    return {};
}

// EGL_KHR_debug
ANGLE_INLINE ScopedContextMutexLock GetContextLock_DebugMessageControlKHR(Thread *thread)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_LabelObjectKHR(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryDebugKHR(Thread *thread, EGLint attribute)
{
    return {};
}

// EGL_KHR_fence_sync
ANGLE_INLINE ScopedContextMutexLock GetContextLock_ClientWaitSyncKHR(Thread *thread,
                                                                     egl::Display *dpyPacked,
                                                                     EGLint flags)
{
    if (priv::ClientWaitSyncHasFlush(flags))
    {
        return TryLockCurrentContext(thread);
    }
    else
    {
        return {};
    }
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateSyncKHR(Thread *thread,
                                                                 egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroySyncKHR(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_GetSyncAttribKHR(Thread *thread,
                                                                    egl::Display *dpyPacked,
                                                                    EGLint attribute)
{
    return {};
}

// EGL_KHR_image
ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateImageKHR(Thread *thread,
                                                                  egl::Display *dpyPacked,
                                                                  gl::ContextID ctxPacked)
{
    return TryLockContext(dpyPacked, ctxPacked);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroyImageKHR(Thread *thread,
                                                                   egl::Display *dpyPacked)
{
    // Explicit lock in egl::Display::destroyImageImpl()
    return {};
}

// EGL_KHR_lock_surface3
ANGLE_INLINE ScopedContextMutexLock GetContextLock_LockSurfaceKHR(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QuerySurface64KHR(Thread *thread,
                                                                     egl::Display *dpyPacked,
                                                                     EGLint attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_UnlockSurfaceKHR(Thread *thread,
                                                                    egl::Display *dpyPacked)
{
    return {};
}

// EGL_KHR_partial_update
ANGLE_INLINE ScopedContextMutexLock GetContextLock_SetDamageRegionKHR(Thread *thread,
                                                                      egl::Display *dpyPacked)
{
    return {};
}

// EGL_KHR_reusable_sync
ANGLE_INLINE ScopedContextMutexLock GetContextLock_SignalSyncKHR(Thread *thread,
                                                                 egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_KHR_stream
ANGLE_INLINE ScopedContextMutexLock GetContextLock_CreateStreamKHR(Thread *thread,
                                                                   egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_DestroyStreamKHR(Thread *thread,
                                                                    egl::Display *dpyPacked)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryStreamKHR(Thread *thread,
                                                                  egl::Display *dpyPacked,
                                                                  EGLenum attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_QueryStreamu64KHR(Thread *thread,
                                                                     egl::Display *dpyPacked,
                                                                     EGLenum attribute)
{
    return {};
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_StreamAttribKHR(Thread *thread,
                                                                   egl::Display *dpyPacked,
                                                                   EGLenum attribute)
{
    return {};
}

// EGL_KHR_stream_consumer_gltexture
ANGLE_INLINE ScopedContextMutexLock GetContextLock_StreamConsumerAcquireKHR(Thread *thread,
                                                                            egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock
GetContextLock_StreamConsumerGLTextureExternalKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

ANGLE_INLINE ScopedContextMutexLock GetContextLock_StreamConsumerReleaseKHR(Thread *thread,
                                                                            egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_KHR_swap_buffers_with_damage
ANGLE_INLINE ScopedContextMutexLock GetContextLock_SwapBuffersWithDamageKHR(Thread *thread,
                                                                            egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_KHR_wait_sync
ANGLE_INLINE ScopedContextMutexLock GetContextLock_WaitSyncKHR(Thread *thread,
                                                               egl::Display *dpyPacked,
                                                               EGLint flags)
{
    return TryLockCurrentContext(thread);
}

// EGL_NV_post_sub_buffer
ANGLE_INLINE ScopedContextMutexLock GetContextLock_PostSubBufferNV(Thread *thread,
                                                                   egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_NV_stream_consumer_gltexture_yuv
ANGLE_INLINE ScopedContextMutexLock
GetContextLock_StreamConsumerGLTextureExternalAttribsNV(Thread *thread, egl::Display *dpyPacked)
{
    return TryLockCurrentContext(thread);
}

// EGL_ANGLE_no_error
ANGLE_INLINE ScopedContextMutexLock GetContextLock_SetValidationEnabledANGLE(Thread *thread)
{
    return {};
}

}  // namespace egl

#endif  // LIBGLESV2_EGL_CONTEXT_LOCK_IMPL_H_
