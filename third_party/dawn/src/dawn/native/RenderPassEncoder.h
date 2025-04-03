// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_RENDERPASSENCODER_H_
#define SRC_DAWN_NATIVE_RENDERPASSENCODER_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/RenderEncoderBase.h"

namespace dawn::native {

class RenderBundleBase;

class RenderPassEncoder final : public RenderEncoderBase {
  public:
    using EndCallback = std::function<MaybeError()>;
    static Ref<RenderPassEncoder> Create(DeviceBase* device,
                                         const UnpackedPtr<RenderPassDescriptor>& descriptor,
                                         CommandEncoder* commandEncoder,
                                         EncodingContext* encodingContext,
                                         RenderPassResourceUsageTracker usageTracker,
                                         Ref<AttachmentState> attachmentState,
                                         uint32_t renderTargetWidth,
                                         uint32_t renderTargetHeight,
                                         bool depthReadOnly,
                                         bool stencilReadOnly,
                                         EndCallback endCallback = nullptr);
    static Ref<RenderPassEncoder> MakeError(DeviceBase* device,
                                            CommandEncoder* commandEncoder,
                                            EncodingContext* encodingContext,
                                            StringView label);

    ~RenderPassEncoder() override;

    ObjectType GetType() const override;

    // NOTE: this will lock the device internally. To avoid deadlock when the device is already
    // locked, use End() instead.
    void APIEnd();

    void APISetStencilReference(uint32_t reference);
    void APISetBlendConstant(const Color* color);
    void APISetViewport(float x,
                        float y,
                        float width,
                        float height,
                        float minDepth,
                        float maxDepth);
    void APISetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void APIExecuteBundles(uint32_t count, RenderBundleBase* const* renderBundles);

    void APIBeginOcclusionQuery(uint32_t queryIndex);
    void APIEndOcclusionQuery();

    void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

    void APIPixelLocalStorageBarrier();

    // Internal code that already locked the device should call this method instead of
    // APIEnd() to avoid the device being locked again.
    void End();

  protected:
    RenderPassEncoder(DeviceBase* device,
                      const UnpackedPtr<RenderPassDescriptor>& descriptor,
                      CommandEncoder* commandEncoder,
                      EncodingContext* encodingContext,
                      RenderPassResourceUsageTracker usageTracker,
                      Ref<AttachmentState> attachmentState,
                      uint32_t renderTargetWidth,
                      uint32_t renderTargetHeight,
                      bool depthReadOnly,
                      bool stencilReadOnly,
                      EndCallback endCallback = nullptr);
    RenderPassEncoder(DeviceBase* device,
                      CommandEncoder* commandEncoder,
                      EncodingContext* encodingContext,
                      ErrorTag errorTag,
                      StringView label);

  private:
    void DestroyImpl() override;

    void TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex);

    // For render and compute passes, the encoding context is borrowed from the command encoder.
    // Keep a reference to the encoder to make sure the context isn't freed.
    Ref<CommandEncoder> mCommandEncoder;

    uint32_t mRenderTargetWidth;
    uint32_t mRenderTargetHeight;

    // The resources for occlusion query
    Ref<QuerySetBase> mOcclusionQuerySet;
    uint32_t mCurrentOcclusionQueryIndex = 0;
    bool mOcclusionQueryActive = false;

    // This is the hardcoded value in the WebGPU spec.
    uint64_t mMaxDrawCount = 50000000;

    EndCallback mEndCallback;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERPASSENCODER_H_
