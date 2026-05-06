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

#ifndef SRC_DAWN_REPLAY_CAPTUREWALKER_H_
#define SRC_DAWN_REPLAY_CAPTUREWALKER_H_

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "dawn/replay/Deserialization.h"
#include "dawn/replay/ReadHead.h"

namespace dawn::replay {

// These x-macros use DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES to generate
// an std::variant that includes each type of BindGroupLayoutEntry. This
// std::variant can then be used in CreateBindGroupLayout with
// a visitor to separate deserialization from actual use.
#define DAWN_REPLAY_BINDGROUPLAYOUT_VARIANT_TYPE(NAME) schema::BindGroupLayoutEntryType##NAME,
using BindGroupLayoutEntryVariant = std::variant<DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES(
    DAWN_REPLAY_BINDGROUPLAYOUT_VARIANT_TYPE) std::monostate>;
#undef DAWN_REPLAY_BINDGROUPLAYOUT_VARIANT_TYPE

// These x-macros use DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES to generate
// an std::variant that includes each type of BindGroupEntry. This
// std::variant can then be used in CreateBindGroup with
// a visitor to separate deserialization from actual use.
#define DAWN_REPLAY_BINDGROUP_VARIANT_TYPE(NAME) schema::BindGroupEntryType##NAME,
#define DAWN_REPLAY_GEN_BINDGROUP_VARIANT(ENUM_NAME, MEMBERS) \
    using BindGroupEntryVariant =                             \
        std::variant<MEMBERS(DAWN_REPLAY_BINDGROUP_VARIANT_TYPE) std::monostate>;

DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES_ENUM(DAWN_REPLAY_GEN_BINDGROUP_VARIANT)

#undef DAWN_REPLAY_GEN_BINDGROUP_VARIANT
#undef DAWN_REPLAY_BINDGROUP_VARIANT_TYPE

// These structures are used to gather data in a generic way to pass to
// visitors for resource creation. For example, BindGroupData is used to
// select the bindGroupEntries that, as serialized are different types,
// and deserialize them into an std::variant of the types that can then
// be passed to a visitor in a generic way. BindGroupLayoutData does
// the same for bindGroupLayoutEntries. For CommandBufferData and
// RenderBundleData, the deserialization is passed on to lower level
// functions and visitors. See DAWN_REPLAY_RESOURCE_DATA_MAP below.
struct InvalidData {};
struct DeviceData {};

struct BindGroupData {
    schema::BindGroup bg;
    std::vector<BindGroupEntryVariant> entries;
};

struct BindGroupLayoutData {
    schema::BindGroupLayout bgl;
    std::vector<BindGroupLayoutEntryVariant> entries;
};

class ReadHead;

class ComputePassVisitor {
  public:
    virtual ~ComputePassVisitor() = default;
#define VISITOR_METHOD(NAME) \
    virtual MaybeError operator()(const schema::CommandBufferCommand##NAME##CmdData& data) = 0;
    DAWN_REPLAY_COMPUTE_PASS_COMMANDS(VISITOR_METHOD)
#undef VISITOR_METHOD
};

class RenderPassVisitor {
  public:
    virtual ~RenderPassVisitor() = default;
#define VISITOR_METHOD(NAME) \
    virtual MaybeError operator()(const schema::CommandBufferCommand##NAME##CmdData& data) = 0;
    DAWN_REPLAY_RENDER_PASS_COMMANDS(VISITOR_METHOD)
#undef VISITOR_METHOD
};

class RenderBundleVisitor {
  public:
    virtual ~RenderBundleVisitor() = default;
#define VISITOR_METHOD(NAME) \
    virtual MaybeError operator()(const schema::CommandBufferCommand##NAME##CmdData& data) = 0;
    DAWN_REPLAY_RENDER_BUNDLE_COMMANDS(VISITOR_METHOD)
#undef VISITOR_METHOD
};

class EncoderVisitor {
  public:
    virtual ~EncoderVisitor() = default;

    virtual ResultOrError<ComputePassVisitor*> BeginComputePass(
        const schema::CommandBufferCommandBeginComputePassCmdData& data) = 0;
    virtual ResultOrError<RenderPassVisitor*> BeginRenderPass(
        const schema::CommandBufferCommandBeginRenderPassCmdData& data) = 0;

#define VISITOR_METHOD(NAME) \
    virtual MaybeError operator()(const schema::CommandBufferCommand##NAME##CmdData& data) = 0;
    DAWN_REPLAY_ENCODER_NON_CREATION_COMMANDS(VISITOR_METHOD)
#undef VISITOR_METHOD
};

struct CommandBufferData {
    ReadHead* readHead;
};

struct RenderBundleData {
    schema::RenderBundle bundle;
    ReadHead* readHead;
};

// This is needed to map a command to a deserialization data type.
// In particular, BindGroupData, BindGroupLayoutData, have
// packed variants, meaning, entry has a type enum, and then only
// the data needed for that particular variant. In order to use
// std::variant, these need to be expanded to an variant that is
// padded to the largest type.
#define DAWN_REPLAY_RESOURCE_DATA_MAP(X)        \
    X(BindGroup, BindGroupData)                 \
    X(BindGroupLayout, BindGroupLayoutData)     \
    X(Buffer, schema::Buffer)                   \
    X(CommandBuffer, CommandBufferData)         \
    X(ComputePipeline, schema::ComputePipeline) \
    X(Device, DeviceData)                       \
    X(ExternalTexture, schema::ExternalTexture) \
    X(PipelineLayout, schema::PipelineLayout)   \
    X(QuerySet, schema::QuerySet)               \
    X(RenderBundle, RenderBundleData)           \
    X(RenderPipeline, schema::RenderPipeline)   \
    X(Sampler, schema::Sampler)                 \
    X(ShaderModule, schema::ShaderModule)       \
    X(TexelBufferView, schema::TexelBufferView) \
    X(Texture, schema::Texture)                 \
    X(TextureView, schema::TextureView)

#define AS_DATA_TYPE(NAME, TYPE) TYPE,
using ResourceData = std::variant<DAWN_REPLAY_RESOURCE_DATA_MAP(AS_DATA_TYPE) std::monostate>;
#undef AS_DATA_TYPE

struct CreateResourceData {
    schema::LabeledResource resource;
    ResourceData data;
};

class ResourceVisitor {
  public:
    virtual ~ResourceVisitor() = default;

#define DAWN_REPLAY_RESOURCE_VISITOR(ENUM, TYPE) \
    virtual MaybeError operator()(const TYPE& data) = 0;
    DAWN_REPLAY_RESOURCE_DATA_MAP(DAWN_REPLAY_RESOURCE_VISITOR)
#undef DAWN_REPLAY_RESOURCE_VISITOR

    virtual MaybeError operator()(const InvalidData& data);
    virtual MaybeError operator()(const std::monostate&);
};

class RootCommandVisitor {
  public:
    virtual ~RootCommandVisitor() = default;

    virtual void SetContentReadHead(ReadHead* readHead) = 0;
    virtual ResourceVisitor& GetResourceVisitor() = 0;
    virtual MaybeError operator()(const CreateResourceData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandWriteBufferCmdData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandWriteTextureCmdData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandQueueSubmitCmdData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandSetLabelCmdData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandInitTextureCmdData& data) = 0;
    virtual MaybeError operator()(const schema::RootCommandEndCmdData& data) = 0;
    virtual MaybeError operator()(const std::monostate&);
};

MaybeError ProcessEncoderCommands(ReadHead* readHead, EncoderVisitor* visitor);
MaybeError ProcessComputePassCommands(ReadHead* readHead, ComputePassVisitor* visitor);
MaybeError ProcessRenderPassCommands(ReadHead* readHead, RenderPassVisitor* visitor);
MaybeError ProcessRenderBundleCommands(ReadHead* readHead, RenderBundleVisitor* visitor);

class CaptureWalker {
  public:
    virtual ~CaptureWalker() = default;
    MaybeError Walk(RootCommandVisitor& visitor);

  protected:
    virtual ReadHead GetCommandReadHead() const = 0;
    virtual ReadHead GetContentReadHead() const = 0;
};

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_CAPTUREWALKER_H_
