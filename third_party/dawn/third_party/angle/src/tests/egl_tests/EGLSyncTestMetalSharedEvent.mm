//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSyncTestMetalSharedEvent:
//   Tests pertaining to EGL_ANGLE_sync_mtl_shared_event extension.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"

#include <Metal/Metal.h>

using namespace angle;

static inline EGLAttrib Uint64HighPart(uint64_t value)
{
    return value >> 32;
}

static inline EGLAttrib Uint64LowPart(uint64_t value)
{
    return value & 0xFFFFFFFF;
}

class EGLSyncTestMetalSharedEvent : public ANGLETest<>
{
  protected:
    id<MTLSharedEvent> createMetalSharedEvent() const
    {
        id<MTLDevice> device           = getMetalDevice();
        id<MTLSharedEvent> sharedEvent = [device newSharedEvent];
        sharedEvent.label              = @"TestSharedEvent";
        return sharedEvent;
    }

    id<MTLDevice> getMetalDevice() const
    {
        EGLAttrib angleDevice = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

        EGLAttrib device = 0;
        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_METAL_DEVICE_ANGLE, &device));

        return (__bridge id<MTLDevice>)reinterpret_cast<void *>(device);
    }

    bool hasEGLDisplayExtension(const char *extname) const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), extname);
    }

    bool hasSyncMetalSharedEventExtension() const
    {
        return hasEGLDisplayExtension("EGL_ANGLE_metal_shared_event_sync");
    }

    EGLAttrib sharedEventAsAttrib(id<MTLSharedEvent> sharedEvent) const
    {
        return reinterpret_cast<EGLAttrib>(sharedEvent);
    }

    id<MTLSharedEvent> sharedEventFromVoidPtr(void *ptr) const
    {
        return (__bridge id<MTLSharedEvent>)ptr;
    }
};

// Test existing fence is created unsignaled
TEST_P(EGLSyncTestMetalSharedEvent, BasicEGLSync)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    constexpr EGLint kSentinelAttribValue = 123456789;
    EGLint signaledValue                  = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_STATUS_KHR, &signaledValue));
    EXPECT_EQ(signaledValue, EGL_UNSIGNALED_KHR);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EGL_TRUE(eglWaitSyncKHR(display, sync, 0));

    glFinish();

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1000'000'000ul;  // 1 second
    EXPECT_EQ(EGL_CONDITION_SATISFIED_KHR,
              eglClientWaitSyncKHR(display, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, kTimeout));

    signaledValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_STATUS_KHR, &signaledValue));
    EXPECT_EQ(signaledValue, EGL_SIGNALED_KHR);

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
}

// Test usage of eglGetSyncAttrib
TEST_P(EGLSyncTestMetalSharedEvent, GetSyncAttrib)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    EXPECT_EQ([sharedEvent retainCount], 1ul);
    uint64_t initialSignalValue = sharedEvent.signaledValue;
    EXPECT_EQ(initialSignalValue, 0u);

    EGLDisplay display      = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,
                               sharedEventAsAttrib(sharedEvent), EGL_NONE};
    EXPECT_EQ([sharedEvent retainCount], 1ul);

    EGLSync sync = eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(sync, EGL_NO_SYNC);
    // sharedEvent, sync, mtlCommandBuffer
    EXPECT_GT([sharedEvent retainCount], 1ul);

    // Fence sync attributes are:
    //
    // EGL_SYNC_TYPE: EGL_SYNC_METAL_SHARED_EVENT_ANGLE
    // EGL_SYNC_STATUS: EGL_UNSIGNALED or EGL_SIGNALED
    // EGL_SYNC_CONDITION: EGL_SYNC_PRIOR_COMMANDS_COMPLETE

    constexpr EGLAttrib kSentinelAttribValue = 123456789;
    EGLAttrib attribValue                    = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_TYPE, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_METAL_SHARED_EVENT_ANGLE);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_CONDITION, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_PRIOR_COMMANDS_COMPLETE);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_STATUS, &attribValue));
    EXPECT_EQ(attribValue, EGL_UNSIGNALED);
    EXPECT_GT([sharedEvent retainCount], 1ul);

    glFinish();

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_STATUS, &attribValue));
    EXPECT_EQ(attribValue, EGL_SIGNALED);
    EXPECT_EQ(sharedEvent.signaledValue, initialSignalValue + 1);
    EXPECT_GT([sharedEvent retainCount], 1ul);

    EXPECT_EGL_TRUE(eglDestroySync(display, sync));
    EXPECT_EQ([sharedEvent retainCount], 1ul);

    sharedEvent = nil;
}

// Test usage of eglGetSyncAttrib with explicit sync condition
TEST_P(EGLSyncTestMetalSharedEvent, GetSyncAttrib_ExplicitSyncCondition)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    EXPECT_EQ(sharedEvent.signaledValue, 0u);

    EGLDisplay display          = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[3][5] = {
        {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, sharedEventAsAttrib(sharedEvent), EGL_NONE},
        {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, sharedEventAsAttrib(sharedEvent),
         EGL_SYNC_CONDITION, EGL_SYNC_PRIOR_COMMANDS_COMPLETE, EGL_NONE},
        {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, sharedEventAsAttrib(sharedEvent),
         EGL_SYNC_CONDITION, EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE, EGL_NONE}};

    EGLAttrib expectedSyncCondition[3] = {EGL_SYNC_PRIOR_COMMANDS_COMPLETE,
                                          EGL_SYNC_PRIOR_COMMANDS_COMPLETE,
                                          EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE};

    for (int i = 0; i < 3; ++i)
    {
        uint64_t initialSignalValue = sharedEvent.signaledValue;

        EGLSync sync = eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs[i]);
        EXPECT_NE(sync, EGL_NO_SYNC);

        constexpr EGLAttrib kSentinelAttribValue = 123456789;
        EGLAttrib attribValue                    = kSentinelAttribValue;
        EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_TYPE, &attribValue));
        EXPECT_EQ(attribValue, EGL_SYNC_METAL_SHARED_EVENT_ANGLE);

        attribValue = kSentinelAttribValue;
        EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_CONDITION, &attribValue));
        EXPECT_EQ(attribValue, expectedSyncCondition[i]);

        attribValue = kSentinelAttribValue;
        EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_STATUS, &attribValue));
        EXPECT_EQ(attribValue, EGL_UNSIGNALED);

        glFinish();

        if (i == 2)
        {
            sharedEvent.signaledValue += 1;
        }

        attribValue = kSentinelAttribValue;
        EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_STATUS, &attribValue));
        EXPECT_EQ(attribValue, EGL_SIGNALED);
        EXPECT_EQ(sharedEvent.signaledValue, initialSignalValue + 1);

        EXPECT_EGL_TRUE(eglDestroySync(display, sync));
    }

    sharedEvent = nil;
}

// Verify CreateSync and ClientWait for EGL_ANGLE_metal_shared_event_sync
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_ClientWait)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();

    EGLDisplay display      = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,
                               sharedEventAsAttrib(sharedEvent), EGL_NONE};

    // We can ClientWait on this
    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);

    // Create work to do
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Wait for the draw to complete
    glClear(GL_COLOR_BUFFER_BIT);

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1000000000;  // 1 second
    EGLAttrib value             = 0;
    EXPECT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, kTimeout));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent));
    sharedEvent = nil;
}

// Verify CreateSync and ClientWait for EGL_ANGLE_metal_shared_event_sync
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_ClientWait_WithSignalValue)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();

    constexpr uint64_t kSignalValue = 0xDEADBEEFCAFE;
    EGLDisplay display              = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[]         = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,
                                       sharedEventAsAttrib(sharedEvent),
                                       EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE,
                                       Uint64HighPart(kSignalValue),
                                       EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE,
                                       Uint64LowPart(kSignalValue),
                                       EGL_NONE};

    // We can ClientWait on this
    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);

    // Create work to do
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Wait for the draw to complete
    glClear(GL_COLOR_BUFFER_BIT);

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1000000000;  // 1 second
    EGLAttrib value             = 0;
    EXPECT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, kTimeout));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);
    EXPECT_EQ(sharedEvent.signaledValue, kSignalValue);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent));
    sharedEvent = nil;
}

// Verify eglCopyMetalSharedEventANGLE for EGL_ANGLE_metal_shared_event_sync
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_CopyMetalSharedEventANGLE)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // We can ClientWait on this
    EGLSync syncWithGeneratedEvent =
        eglCreateSyncKHR(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, nullptr);
    EXPECT_NE(syncWithGeneratedEvent, EGL_NO_SYNC_KHR);

    id<MTLSharedEvent> sharedEvent =
        sharedEventFromVoidPtr(eglCopyMetalSharedEventANGLE(display, syncWithGeneratedEvent));
    EXPECT_EGL_SUCCESS();
    EXPECT_GT([sharedEvent retainCount], 1ul);

    glFinish();
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithGeneratedEvent));

    // Clean up created objects.
    EXPECT_EQ([sharedEvent retainCount], 1ul);
    [sharedEvent release];
}

// Verify WaitSync with EGL_ANGLE_metal_shared_event_sync
// Simulate passing shared events across processes by passing across Contexts.
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_WaitSync)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();

    EGLAttrib value    = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();

    /*- First Context ------------------------*/

    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,
                               sharedEventAsAttrib(sharedEvent), EGL_NONE};
    // We can ClientWait on this

    EGLSync syncWithSharedEvent1 =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent1, EGL_NO_SYNC);
    if (syncWithSharedEvent1 == EGL_NO_SYNC)
    {
        // Unable to continue with test.
        return;
    }

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    /*- Second Context ------------------------*/
    EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    EGLContext context2 = getEGLWindow()->createContext(EGL_NO_CONTEXT, nullptr);
    EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context2));

    EGLSync syncWithSharedEvent2 =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent2, EGL_NO_SYNC);
    if (syncWithSharedEvent2 == EGL_NO_SYNC)
    {
        // Unable to continue with test.
        return;
    }

    // Second draw waits for first to complete. May already be signaled - ignore error.
    if (eglWaitSync(display, syncWithSharedEvent2, 0) == EGL_TRUE)
    {
        // Create work to do
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
    }

    // Wait for second draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent2,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, 1000000000));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent2, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Reset to default context and surface.
    EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, getEGLWindow()->getContext()));

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent2));
    EXPECT_EGL_TRUE(eglDestroyContext(display, context2));

    // Wait for first draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent1,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, 1000000000));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent1, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent1));
    sharedEvent = nil;
}

// Verify WaitSync with EGL_ANGLE_metal_shared_event_sync
// Simulate passing shared events across processes by passing across Contexts.
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_WaitSync_ExternallySignaled)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();

    EGLAttrib value    = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE,
                               sharedEventAsAttrib(sharedEvent), EGL_SYNC_CONDITION,
                               EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE, EGL_NONE};

    // We can ClientWait on this
    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);
    if (syncWithSharedEvent == EGL_NO_SYNC)
    {
        // Unable to continue with test.
        return;
    }

    constexpr EGLAttrib kSentinelAttribValue = 123456789;
    value                                    = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_CONDITION, &value));
    EXPECT_EQ(value, EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE);

    EXPECT_EQ(EGL_TIMEOUT_EXPIRED,
              eglClientWaitSync(display, syncWithSharedEvent, EGL_SYNC_FLUSH_COMMANDS_BIT, 0));

    value = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_UNSIGNALED);

    // Wait for previous work to complete before drawing
    EXPECT_EGL_TRUE(eglWaitSync(display, syncWithSharedEvent, 0));

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // After explicit flush, should still time out
    // TODO(djg): flushing here causes a 5s stall in `CommandBuffer::wait`
    // (mtl_command_buffer.mm:765) EXPECT_EQ(EGL_TIMEOUT_EXPIRED,
    //           eglClientWaitSync(display, syncWithSharedEvent, EGL_SYNC_FLUSH_COMMANDS_BIT, 0));
    EXPECT_EQ(EGL_TIMEOUT_EXPIRED, eglClientWaitSync(display, syncWithSharedEvent, 0, 0));

    value = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_UNSIGNALED);

    // Signal the MTLSharedEvent
    sharedEvent.signaledValue += 1;

    // Wait for draw to complete. This will be satisfied since the signalValue
    // was incremented on sharedEvent.
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSync(display, syncWithSharedEvent, EGL_SYNC_FLUSH_COMMANDS_BIT, 0));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent));
    sharedEvent = nil;
}

ANGLE_INSTANTIATE_TEST(EGLSyncTestMetalSharedEvent, ES2_METAL(), ES3_METAL());
// This test suite is not instantiated on non-Metal backends and OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLSyncTestMetalSharedEvent);
