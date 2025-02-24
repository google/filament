//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVk.h:
//    Defines the class interface for SurfaceVk, implementing SurfaceImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SURFACEVK_H_
#define LIBANGLE_RENDERER_VULKAN_SURFACEVK_H_

#include "common/CircularBuffer.h"
#include "common/SimpleMutex.h"
#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class SurfaceVk : public SurfaceImpl, public angle::ObserverInterface, public vk::Resource
{
  public:
    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

  protected:
    SurfaceVk(const egl::SurfaceState &surfaceState);
    ~SurfaceVk() override;

    void destroy(const egl::Display *display) override;
    // We monitor the staging buffer for changes. This handles staged data from outside this class.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    // width and height can change with client window resizing
    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint mWidth;
    EGLint mHeight;

    RenderTargetVk mColorRenderTarget;
    RenderTargetVk mDepthStencilRenderTarget;
};

class OffscreenSurfaceVk : public SurfaceVk
{
  public:
    OffscreenSurfaceVk(const egl::SurfaceState &surfaceState, vk::Renderer *renderer);
    ~OffscreenSurfaceVk() override;

    egl::Error initialize(const egl::Display *display) override;
    void destroy(const egl::Display *display) override;

    egl::Error unMakeCurrent(const gl::Context *context) override;
    const vk::ImageHelper *getColorImage() const { return &mColorAttachment.image; }

    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;
    egl::Error getMscRate(EGLint *numerator, EGLint *denominator) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    vk::ImageHelper *getColorAttachmentImage();

    egl::Error lockSurface(const egl::Display *display,
                           EGLint usageHint,
                           bool preservePixels,
                           uint8_t **bufferPtrOut,
                           EGLint *bufferPitchOut) override;
    egl::Error unlockSurface(const egl::Display *display, bool preservePixels) override;
    EGLint origin() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  protected:
    struct AttachmentImage final : angle::NonCopyable
    {
        AttachmentImage(SurfaceVk *surfaceVk);
        ~AttachmentImage();

        angle::Result initialize(DisplayVk *displayVk,
                                 EGLint width,
                                 EGLint height,
                                 const vk::Format &vkFormat,
                                 GLint samples,
                                 bool isRobustResourceInitEnabled,
                                 bool hasProtectedContent);

        void destroy(const egl::Display *display);

        vk::ImageHelper image;
        vk::ImageViewHelper imageViews;
        angle::ObserverBinding imageObserverBinding;
    };

    virtual angle::Result initializeImpl(DisplayVk *displayVk);

    AttachmentImage mColorAttachment;
    AttachmentImage mDepthStencilAttachment;

    // EGL_KHR_lock_surface3
    vk::BufferHelper mLockBufferHelper;
};

// Data structures used in WindowSurfaceVk
namespace impl
{
static constexpr size_t kSwapHistorySize = 2;

// Old swapchain and associated present fences/semaphores that need to be scheduled for
// recycling/destruction when appropriate.
struct SwapchainCleanupData : angle::NonCopyable
{
    SwapchainCleanupData();
    SwapchainCleanupData(SwapchainCleanupData &&other);
    ~SwapchainCleanupData();

    // Fences must not be empty (VK_EXT_swapchain_maintenance1 is supported).
    VkResult getFencesStatus(VkDevice device) const;
    // Waits fences if any. Use before force destroying the swapchain.
    void waitFences(VkDevice device, uint64_t timeout) const;
    void destroy(VkDevice device,
                 vk::Recycler<vk::Fence> *fenceRecycler,
                 vk::Recycler<vk::Semaphore> *semaphoreRecycler);

    // The swapchain to be destroyed.
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    // Any present fences/semaphores that were pending recycle at the time the swapchain was
    // recreated will be scheduled for recycling at the same time as the swapchain's destruction.
    // fences must be in the present operation order.
    std::vector<vk::Fence> fences;
    std::vector<vk::Semaphore> semaphores;
};

// Each present operation is associated with a wait semaphore.  To know when that semaphore can be
// recycled, a swapSerial is used.  When that swapSerial is finished, the semaphore used in the
// previous present operation involving imageIndex can be recycled.  See doc/PresentSemaphores.md
// for details.
// When VK_EXT_swapchain_maintenance1 is supported, present fence is used instead of the swapSerial.
//
// Old swapchains are scheduled to be destroyed at the same time as the last wait semaphore used to
// present an image to the old swapchains can be recycled (only relevant when
// VK_EXT_swapchain_maintenance1 is not supported).
struct ImagePresentOperation : angle::NonCopyable
{
    ImagePresentOperation();
    ImagePresentOperation(ImagePresentOperation &&other);
    ImagePresentOperation &operator=(ImagePresentOperation &&other);
    ~ImagePresentOperation();

    void destroy(VkDevice device,
                 vk::Recycler<vk::Fence> *fenceRecycler,
                 vk::Recycler<vk::Semaphore> *semaphoreRecycler);

    // fence is only used when VK_EXT_swapchain_maintenance1 is supported.
    vk::Fence fence;
    vk::Semaphore semaphore;

    // Below members only relevant when VK_EXT_swapchain_maintenance1 is not supported.
    // Used to associate a swapSerial with the previous present operation of the image.
    uint32_t imageIndex;
    QueueSerial queueSerial;
    std::deque<SwapchainCleanupData> oldSwapchains;
};

// Swapchain images and their associated objects.
struct SwapchainImage : angle::NonCopyable
{
    SwapchainImage();
    SwapchainImage(SwapchainImage &&other);
    ~SwapchainImage();

    std::unique_ptr<vk::ImageHelper> image;
    vk::ImageViewHelper imageViews;
    vk::Framebuffer framebuffer;
    vk::Framebuffer fetchFramebuffer;

    uint64_t frameNumber = 0;
};

enum class ImageAcquireState
{
    Ready,
    NeedToAcquire,
    NeedToProcessResult,
};

// Associated data for a call to vkAcquireNextImageKHR without necessarily holding the share group
// and global locks but ONLY from a thread where Surface is current.
struct UnlockedAcquireData : angle::NonCopyable
{
    // Given that the CPU is throttled after a number of swaps, there is an upper bound to the
    // number of semaphores that are used to acquire swapchain images, and that is
    // kSwapHistorySize+1:
    //
    //             Unrelated submission in     Submission as part of
    //               the middle of frame          buffer swap
    //                              |                 |
    //                              V                 V
    //     Frame i:     ... ANI ... QS (fence Fa) ... QS (Fence Fb) QP Wait(..)
    //     Frame i+1:   ... ANI ... QS (fence Fc) ... QS (Fence Fd) QP Wait(..) <--\
    //     Frame i+2:   ... ANI ... QS (fence Fe) ... QS (Fence Ff) QP Wait(Fb)    |
    //                                                                  ^          |
    //                                                                  |          |
    //                                                           CPU throttling    |
    //                                                                             |
    //                               Note: app should throttle itself here (equivalent of Wait(Fb))
    //
    // In frame i+2 (2 is kSwapHistorySize), ANGLE waits on fence Fb which means that the semaphore
    // used for Frame i's ANI can be reused (because Fb-is-signalled implies Fa-is-signalled).
    // Before this wait, there were three acquire semaphores in use corresponding to frames i, i+1
    // and i+2.  Frame i+3 can reuse the semaphore of frame i.
    angle::CircularBuffer<vk::Semaphore, impl::kSwapHistorySize + 1> acquireImageSemaphores;
};

struct UnlockedAcquireResult : angle::NonCopyable
{
    // The result of the call to vkAcquireNextImageKHR.
    VkResult result = VK_SUCCESS;

    // Semaphore to signal.
    VkSemaphore acquireSemaphore = VK_NULL_HANDLE;

    // Image index that was acquired
    uint32_t imageIndex = std::numeric_limits<uint32_t>::max();
};

struct ImageAcquireOperation : angle::NonCopyable
{
    // Initially image needs to be acquired.
    ImageAcquireState state = ImageAcquireState::NeedToAcquire;

    // No synchronization is necessary when making the vkAcquireNextImageKHR call since it is ONLY
    // possible on a thread where Surface is current.
    UnlockedAcquireData unlockedAcquireData;
    UnlockedAcquireResult unlockedAcquireResult;
};
}  // namespace impl

class WindowSurfaceVk : public SurfaceVk
{
  public:
    WindowSurfaceVk(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVk() override;

    void destroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display) override;

    egl::Error unMakeCurrent(const gl::Context *context) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    egl::Error prepareSwap(const gl::Context *context) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error swapWithDamage(const gl::Context *context,
                              const EGLint *rects,
                              EGLint n_rects) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;
    egl::Error getMscRate(EGLint *numerator, EGLint *denominator) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    // Note: windows cannot be resized on Android.  The approach requires
    // calling vkGetPhysicalDeviceSurfaceCapabilitiesKHR.  However, that is
    // expensive; and there are troublesome timing issues for other parts of
    // ANGLE (which cause test failures and crashes).  Therefore, a
    // special-Android-only path is created just for the querying of EGL_WIDTH
    // and EGL_HEIGHT.
    // https://issuetracker.google.com/issues/153329980
    egl::Error getUserWidth(const egl::Display *display, EGLint *value) const override;
    egl::Error getUserHeight(const egl::Display *display, EGLint *value) const override;
    angle::Result getUserExtentsImpl(DisplayVk *displayVk,
                                     VkSurfaceCapabilitiesKHR *surfaceCaps) const;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    vk::Framebuffer &chooseFramebuffer();

    angle::Result getCurrentFramebuffer(ContextVk *context,
                                        vk::FramebufferFetchMode fetchMode,
                                        const vk::RenderPass &compatibleRenderPass,
                                        vk::Framebuffer *framebufferOut);

    VkSurfaceTransformFlagBitsKHR getPreTransform() const
    {
        if (mEmulatedPreTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            return mEmulatedPreTransform;
        }
        return mPreTransform;
    }

    egl::Error setAutoRefreshEnabled(bool enabled) override;

    egl::Error getBufferAge(const gl::Context *context, EGLint *age) override;

    egl::Error setRenderBuffer(EGLint renderBuffer) override;

    bool isSharedPresentMode() const
    {
        return (mSwapchainPresentMode == vk::PresentMode::SharedDemandRefreshKHR ||
                mSwapchainPresentMode == vk::PresentMode::SharedContinuousRefreshKHR);
    }

    bool isSharedPresentModeDesired() const
    {
        vk::PresentMode desiredSwapchainPresentMode = getDesiredSwapchainPresentMode();
        return (desiredSwapchainPresentMode == vk::PresentMode::SharedDemandRefreshKHR ||
                desiredSwapchainPresentMode == vk::PresentMode::SharedContinuousRefreshKHR);
    }

    egl::Error lockSurface(const egl::Display *display,
                           EGLint usageHint,
                           bool preservePixels,
                           uint8_t **bufferPtrOut,
                           EGLint *bufferPitchOut) override;
    egl::Error unlockSurface(const egl::Display *display, bool preservePixels) override;
    EGLint origin() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

    angle::Result onSharedPresentContextFlush(const gl::Context *context);

    bool hasStagedUpdates() const;

    void setTimestampsEnabled(bool enabled) override;

    EGLint getCompressionRate(const egl::Display *display) const override;

  protected:
    angle::Result swapImpl(const gl::Context *context,
                           const EGLint *rects,
                           EGLint n_rects,
                           const void *pNextChain);

    EGLNativeWindowType mNativeWindowType;
    VkSurfaceKHR mSurface;
    VkSurfaceCapabilitiesKHR mSurfaceCaps;
    VkBool32 mSupportsProtectedSwapchain;

  private:
    virtual angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) = 0;
    virtual angle::Result getCurrentWindowSize(vk::ErrorContext *context,
                                               gl::Extents *extentsOut)                       = 0;

    vk::PresentMode getDesiredSwapchainPresentMode() const;
    void setDesiredSwapchainPresentMode(vk::PresentMode presentMode);
    void setDesiredSwapInterval(EGLint interval);

    angle::Result initializeImpl(DisplayVk *displayVk, bool *anyMatchesOut);
    angle::Result recreateSwapchain(ContextVk *contextVk, const gl::Extents &extents);
    angle::Result createSwapChain(vk::ErrorContext *context, const gl::Extents &extents);
    angle::Result collectOldSwapchain(ContextVk *contextVk, VkSwapchainKHR swapchain);
    angle::Result queryAndAdjustSurfaceCaps(ContextVk *contextVk,
                                            VkSurfaceCapabilitiesKHR *surfaceCaps);
    angle::Result checkForOutOfDateSwapchain(ContextVk *contextVk, bool forceRecreate);
    angle::Result resizeSwapchainImages(vk::ErrorContext *context, uint32_t imageCount);
    void releaseSwapchainImages(ContextVk *contextVk);
    void destroySwapChainImages(DisplayVk *displayVk);
    angle::Result prepareForAcquireNextSwapchainImage(const gl::Context *context,
                                                      bool forceSwapchainRecreate);
    // Called when a swapchain image whose acquisition was deferred must be acquired.  This method
    // will recreate the swapchain (if needed due to present returning OUT_OF_DATE, swap interval
    // changing, surface size changing etc, by calling prepareForAcquireNextSwapchainImage()) and
    // call the doDeferredAcquireNextImageWithUsableSwapchain() method.
    angle::Result doDeferredAcquireNextImage(const gl::Context *context,
                                             bool forceSwapchainRecreate);
    // Calls acquireNextSwapchainImage() and sets up the acquired image.  On some platforms,
    // vkAcquireNextImageKHR returns OUT_OF_DATE instead of present, so this function may still
    // recreate the swapchain.  The main difference with doDeferredAcquireNextImage is that it does
    // not check for surface property changes for the purposes of swapchain recreation (because
    // that's already done by prepareForAcquireNextSwapchainImage.
    angle::Result doDeferredAcquireNextImageWithUsableSwapchain(const gl::Context *context);
    // This method calls vkAcquireNextImageKHR() to acquire the next swapchain image or to process
    // unlocked ANI result.  It is scheduled to be called later by deferAcquireNextImage().
    VkResult acquireNextSwapchainImage(vk::ErrorContext *context);
    // Process the result of vkAcquireNextImageKHR.
    VkResult postProcessUnlockedAcquire(vk::ErrorContext *context);
    // This method is called when a swapchain image is presented.  It schedules
    // acquireNextSwapchainImage() to be called later.
    void deferAcquireNextImage();
    bool skipAcquireNextSwapchainImageForSharedPresentMode() const;

    angle::Result computePresentOutOfDate(vk::ErrorContext *context,
                                          VkResult result,
                                          bool *presentOutOfDate);
    angle::Result prePresentSubmit(ContextVk *contextVk, const vk::Semaphore &presentSemaphore);
    angle::Result present(ContextVk *contextVk,
                          const EGLint *rects,
                          EGLint n_rects,
                          const void *pNextChain,
                          bool *presentOutOfDate);

    angle::Result cleanUpPresentHistory(vk::ErrorContext *context);
    angle::Result cleanUpOldSwapchains(vk::ErrorContext *context);

    // Throttle the CPU such that application's logic and command buffer recording doesn't get more
    // than two frame ahead of the frame being rendered (and three frames ahead of the one being
    // presented).  This is a failsafe, as the application should ensure command buffer recording is
    // not ahead of the frame being rendered by *one* frame.
    angle::Result throttleCPU(vk::ErrorContext *context, const QueueSerial &currentSubmitSerial);

    // Finish all GPU operations on the surface
    angle::Result finish(vk::ErrorContext *context);

    void updateOverlay(ContextVk *contextVk) const;
    bool overlayHasEnabledWidget(ContextVk *contextVk) const;
    angle::Result drawOverlay(ContextVk *contextVk, impl::SwapchainImage *image) const;

    bool isMultiSampled() const;

    bool supportsPresentMode(vk::PresentMode presentMode) const;

    bool updateColorSpace(DisplayVk *displayVk);

    angle::FormatID getIntendedFormatID(vk::Renderer *renderer);
    angle::FormatID getActualFormatID(vk::Renderer *renderer);

    std::vector<vk::PresentMode> mPresentModes;

    VkSwapchainKHR mSwapchain;      // Current swapchain (same as last created or NULL)
    VkSwapchainKHR mLastSwapchain;  // Last created non retired swapchain (or NULL if retired)
    // Cached information used to recreate swapchains.
    vk::PresentMode mSwapchainPresentMode;                      // Current swapchain mode
    std::atomic<vk::PresentMode> mDesiredSwapchainPresentMode;  // Desired swapchain mode
    uint32_t mMinImageCount;
    VkSurfaceTransformFlagBitsKHR mPreTransform;
    VkSurfaceTransformFlagBitsKHR mEmulatedPreTransform;
    VkCompositeAlphaFlagBitsKHR mCompositeAlpha;
    VkColorSpaceKHR mSurfaceColorSpace;
    VkImageCompressionFlagBitsEXT mCompressionFlags;
    VkImageCompressionFixedRateFlagsEXT mFixedRateFlags;

    // Present modes that are compatible with the current mode.  If mDesiredSwapchainPresentMode is
    // in this list, mode switch can happen without the need to recreate the swapchain.
    // There are currently only 6 possible present modes but vector is bigger for a workaround.
    static constexpr uint32_t kCompatiblePresentModesSize = 10;
    angle::FixedVector<VkPresentModeKHR, kCompatiblePresentModesSize> mCompatiblePresentModes;

    // A circular buffer that stores the serial of the submission fence of the context on every
    // swap. The CPU is throttled by waiting for the 2nd previous serial to finish.  This should
    // normally be a no-op, as the application should pace itself to avoid input lag, and is
    // implemented in ANGLE as a fail safe.  Removing this throttling requires untangling it from
    // acquire semaphore recycling (see mAcquireImageSemaphores above)
    angle::CircularBuffer<QueueSerial, impl::kSwapHistorySize> mSwapHistory;

    // The previous swapchain which needs to be scheduled for destruction when appropriate.  This
    // will be done when the first image of the current swapchain is presented or when fences are
    // signaled (when VK_EXT_swapchain_maintenance1 is supported).  If there were older swapchains
    // pending destruction when the swapchain is recreated, they will accumulate and be destroyed
    // with the previous swapchain.
    //
    // Note that if the user resizes the window such that the swapchain is recreated every frame,
    // this array can go grow indefinitely.
    std::deque<impl::SwapchainCleanupData> mOldSwapchains;

    std::vector<impl::SwapchainImage> mSwapchainImages;
    std::vector<angle::ObserverBinding> mSwapchainImageBindings;
    uint32_t mCurrentSwapchainImageIndex;

    // There is no direct signal from Vulkan regarding when a Present semaphore can be be reused.
    // During window resizing when swapchains are recreated every frame, the number of in-flight
    // present semaphores can grow indefinitely.  See doc/PresentSemaphores.md.
    vk::Recycler<vk::Semaphore> mPresentSemaphoreRecycler;
    // Fences are associated with present semaphores to know when they can be recycled.
    vk::Recycler<vk::Fence> mPresentFenceRecycler;

    // The presentation history, used to recycle semaphores and destroy old swapchains.
    std::deque<impl::ImagePresentOperation> mPresentHistory;

    // Depth/stencil image.  Possibly multisampled.
    vk::ImageHelper mDepthStencilImage;
    vk::ImageViewHelper mDepthStencilImageViews;
    angle::ObserverBinding mDepthStencilImageBinding;

    // Multisample color image, view and framebuffer, if multisampling enabled.
    vk::ImageHelper mColorImageMS;
    vk::ImageViewHelper mColorImageMSViews;
    angle::ObserverBinding mColorImageMSBinding;
    vk::Framebuffer mFramebufferMS;

    impl::ImageAcquireOperation mAcquireOperation;

    // EGL_EXT_buffer_age: Track frame count.
    uint64_t mFrameCount;

    // EGL_KHR_lock_surface3
    vk::BufferHelper mLockBufferHelper;

    // EGL_KHR_partial_update
    uint64_t mBufferAgeQueryFrameNumber;

    // GL_EXT_shader_framebuffer_fetch
    vk::FramebufferFetchMode mFramebufferFetchMode = vk::FramebufferFetchMode::None;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SURFACEVK_H_
