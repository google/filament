// Copyright 2026 The Dawn & Tint Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "src/dawn/replay/CaptureWalker.h"
#include "src/dawn/replay/ReadHead.h"
#include "src/dawn/replay/SurfaceDiscovery.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn::replay {
namespace {

using testing::ElementsAre;

class MockResourceVisitor : public ResourceVisitor {
  public:
    VisitResult operator()(const BindGroupData&) override { return VisitBindGroup(); }
    VisitResult operator()(const BindGroupLayoutData&) override { return VisitBindGroupLayout(); }
    VisitResult operator()(const schema::Buffer&) override { return VisitBuffer(); }
    VisitResult operator()(const CommandBufferData& data) override {
        return VisitCommandBuffer(data);
    }
    VisitResult operator()(const schema::ComputePipeline&) override {
        return VisitComputePipeline();
    }
    VisitResult operator()(const DeviceData&) override { return VisitDevice(); }
    VisitResult operator()(const schema::ExternalTexture&) override {
        return VisitExternalTexture();
    }
    VisitResult operator()(const schema::PipelineLayout&) override { return VisitPipelineLayout(); }
    VisitResult operator()(const schema::QuerySet&) override { return VisitQuerySet(); }
    VisitResult operator()(const RenderBundleData& data) override {
        return VisitRenderBundle(data);
    }
    VisitResult operator()(const schema::RenderPipeline&) override { return VisitRenderPipeline(); }
    VisitResult operator()(const schema::Sampler&) override { return VisitSampler(); }
    VisitResult operator()(const schema::ShaderModule&) override { return VisitShaderModule(); }
    VisitResult operator()(const InvalidData&) override { return VisitInvalid(); }
    VisitResult operator()(const schema::TexelBufferView&) override {
        return VisitTexelBufferView();
    }
    VisitResult operator()(const schema::Texture&) override { return VisitTexture(); }
    VisitResult operator()(const schema::TextureView&) override { return VisitTextureView(); }
    VisitResult operator()(const std::monostate&) override { return VisitMonostate(); }

    MOCK_METHOD(VisitResult, VisitBindGroup, ());
    MOCK_METHOD(VisitResult, VisitBindGroupLayout, ());
    MOCK_METHOD(VisitResult, VisitBuffer, ());
    MOCK_METHOD(VisitResult, VisitCommandBuffer, (const CommandBufferData&));
    MOCK_METHOD(VisitResult, VisitComputePipeline, ());
    MOCK_METHOD(VisitResult, VisitDevice, ());
    MOCK_METHOD(VisitResult, VisitExternalTexture, ());
    MOCK_METHOD(VisitResult, VisitPipelineLayout, ());
    MOCK_METHOD(VisitResult, VisitQuerySet, ());
    MOCK_METHOD(VisitResult, VisitRenderBundle, (const RenderBundleData&));
    MOCK_METHOD(VisitResult, VisitRenderPipeline, ());
    MOCK_METHOD(VisitResult, VisitSampler, ());
    MOCK_METHOD(VisitResult, VisitShaderModule, ());
    MOCK_METHOD(VisitResult, VisitInvalid, ());
    MOCK_METHOD(VisitResult, VisitTexelBufferView, ());
    MOCK_METHOD(VisitResult, VisitTexture, ());
    MOCK_METHOD(VisitResult, VisitTextureView, ());
    MOCK_METHOD(VisitResult, VisitMonostate, ());
};

class MockRootCommandVisitor : public RootCommandVisitor {
  public:
    MOCK_METHOD(void, SetContentReadHead, (ReadHead * readHead), (override));
    MOCK_METHOD(ResourceVisitor&, GetResourceVisitor, (), (override));

    VisitResult operator()(const CreateResourceData& data) override {
        return VisitCreateResource(data);
    }
    VisitResult operator()(const schema::RootCommandWriteBufferCmdData& data) override {
        return VisitWriteBuffer(data);
    }
    VisitResult operator()(const schema::RootCommandWriteTextureCmdData& data) override {
        return VisitWriteTexture(data);
    }
    VisitResult operator()(const schema::RootCommandQueueSubmitCmdData& data) override {
        return VisitQueueSubmit(data);
    }
    VisitResult operator()(const schema::RootCommandSetLabelCmdData& data) override {
        return VisitSetLabel(data);
    }
    VisitResult operator()(const schema::RootCommandInitTextureCmdData& data) override {
        return VisitInitTexture(data);
    }
    VisitResult operator()(const schema::RootCommandSurfaceConfigureCmdData& data) override {
        return VisitSurfaceConfigure(data);
    }
    VisitResult operator()(const schema::RootCommandSurfaceUnconfigureCmdData& data) override {
        return VisitSurfaceUnconfigure(data);
    }
    VisitResult operator()(const schema::RootCommandSurfacePresentCmdData& data) override {
        return VisitSurfacePresent(data);
    }
    VisitResult operator()(
        const schema::RootCommandSurfaceGetCurrentTextureCmdData& data) override {
        return VisitSurfaceGetCurrentTexture(data);
    }
    VisitResult operator()(const schema::RootCommandEndCmdData& data) override {
        return VisitEnd(data);
    }
    VisitResult operator()(const std::monostate& data) override { return VisitMonostate(data); }

    MOCK_METHOD(VisitResult, VisitCreateResource, (const CreateResourceData& data));
    MOCK_METHOD(VisitResult, VisitWriteBuffer, (const schema::RootCommandWriteBufferCmdData& data));
    MOCK_METHOD(VisitResult,
                VisitWriteTexture,
                (const schema::RootCommandWriteTextureCmdData& data));
    MOCK_METHOD(VisitResult, VisitQueueSubmit, (const schema::RootCommandQueueSubmitCmdData& data));
    MOCK_METHOD(VisitResult, VisitSetLabel, (const schema::RootCommandSetLabelCmdData& data));
    MOCK_METHOD(VisitResult, VisitInitTexture, (const schema::RootCommandInitTextureCmdData& data));
    MOCK_METHOD(VisitResult,
                VisitSurfaceConfigure,
                (const schema::RootCommandSurfaceConfigureCmdData& data));
    MOCK_METHOD(VisitResult,
                VisitSurfaceUnconfigure,
                (const schema::RootCommandSurfaceUnconfigureCmdData& data));
    MOCK_METHOD(VisitResult,
                VisitSurfacePresent,
                (const schema::RootCommandSurfacePresentCmdData& data));
    MOCK_METHOD(VisitResult,
                VisitSurfaceGetCurrentTexture,
                (const schema::RootCommandSurfaceGetCurrentTextureCmdData& data));
    MOCK_METHOD(VisitResult, VisitEnd, (const schema::RootCommandEndCmdData& data));
    MOCK_METHOD(VisitResult, VisitMonostate, (const std::monostate&));
};

class TestCaptureWalker : public CaptureWalker {
  public:
    explicit TestCaptureWalker(std::vector<uint8_t> commands) : mCommands(std::move(commands)) {}

  protected:
    ReadHead GetCommandReadHead() const override { return ReadHead(mCommands); }
    ReadHead GetContentReadHead() const override { return ReadHead(mContent); }

  private:
    std::vector<uint8_t> mCommands;
    std::vector<uint8_t> mContent;
};

// Test that CaptureWalker correctly skips nested CommandBuffer commands when using
// SurfaceDiscoveryVisitor.
TEST(CaptureWalkerTests, SurfaceDiscoverySkipsCommandBuffer) {
    std::vector<uint8_t> commands;

    auto Emit = [&](auto v) {
        Span<const uint8_t> p = ReinterpretSpan<const uint8_t>(ByteSpanFromRef(v));
        commands.insert(commands.end(), p.begin(), p.end());
    };

    auto EmitString = [&](std::string s) {
        Emit(static_cast<size_t>(s.size()));
        commands.insert(commands.end(), s.begin(), s.end());
    };

    // 1. CreateResource (CommandBuffer)
    Emit(schema::RootCommand::CreateResource);
    // LabeledResource
    Emit(schema::ObjectType::CommandBuffer);  // type
    Emit(static_cast<schema::ObjectId>(10));  // id
    EmitString("MyCommandBuffer");            // label
    // CommandBufferData (empty, but nested commands follow)
    // Nested Command: CopyBufferToBuffer
    Emit(schema::CommandBufferCommand::CopyBufferToBuffer);
    Emit(static_cast<schema::ObjectId>(1));  // src
    Emit(static_cast<uint64_t>(0));          // srcOffset
    Emit(static_cast<schema::ObjectId>(2));  // dst
    Emit(static_cast<uint64_t>(0));          // dstOffset
    Emit(static_cast<uint64_t>(1024));       // size
    // Nested Command: End
    Emit(schema::CommandBufferCommand::End);

    // 2. SurfaceConfigure (Surface) - This is what we want to discover
    Emit(schema::RootCommand::SurfaceConfigure);
    Emit(static_cast<schema::ObjectId>(20));  // surfaceId
    // SurfaceConfiguration
    Emit(static_cast<schema::ObjectId>(1));      // deviceId
    Emit(wgpu::TextureFormat::RGBA8Unorm);       // format
    Emit(wgpu::TextureUsage::RenderAttachment);  // usage
    Emit(static_cast<size_t>(0));                // viewFormats count
    Emit(wgpu::CompositeAlphaMode::Opaque);      // alphaMode
    Emit(static_cast<uint32_t>(100));            // width
    Emit(static_cast<uint32_t>(200));            // height
    Emit(wgpu::PresentMode::Fifo);               // presentMode

    // 3. End
    Emit(schema::RootCommand::End);

    TestCaptureWalker walker(commands);
    SurfaceDiscoveryVisitor visitor;
    MaybeError result = walker.Walk(visitor);
    EXPECT_FALSE(result.IsError());

    std::vector<schema::ObjectId> surfaceIds = visitor.GetSurfaceIds();
    EXPECT_THAT(surfaceIds, ElementsAre(20));
}

}  // anonymous namespace
}  // namespace dawn::replay
