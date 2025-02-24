//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_cache_utils.h:
//    Contains the classes for the Pipeline State Object cache as well as the RenderPass cache.
//    Also contains the structures for the packed descriptions for the RenderPass and Pipeline.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_CACHE_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_CACHE_UTILS_H_

#include "common/Color.h"
#include "common/FixedVector.h"
#include "common/SimpleMutex.h"
#include "common/WorkerThread.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace gl
{
class ProgramExecutable;
}  // namespace gl

namespace rx
{
class ShaderInterfaceVariableInfoMap;
class UpdateDescriptorSetsBuilder;

// Some descriptor set and pipeline layout constants.
//
// The set/binding assignment is done as following:
//
// - Set 0 contains uniform blocks created to encompass default uniforms.  1 binding is used per
//   pipeline stage.  Additionally, transform feedback buffers are bound from binding 2 and up.
//   For internal shaders, set 0 is used for all the needed resources.
// - Set 1 contains all textures (including texture buffers).
// - Set 2 contains all other shader resources, such as uniform and storage blocks, atomic counter
//   buffers, images and image buffers.
// - Set 3 reserved for OpenCL

enum class DescriptorSetIndex : uint32_t
{
    Internal       = 0,         // Internal shaders
    UniformsAndXfb = Internal,  // Uniforms set index
    Texture        = 1,         // Textures set index
    ShaderResource = 2,         // Other shader resources set index

    // CL specific naming for set indices
    LiteralSampler  = 0,
    KernelArguments = 1,
    ModuleConstants = 2,
    Printf          = 3,

    InvalidEnum = 4,
    EnumCount   = InvalidEnum,
};

namespace vk
{
class Context;
class BufferHelper;
class DynamicDescriptorPool;
class SamplerHelper;
enum class ImageLayout;
class PipelineCacheAccess;
class RenderPassCommandBufferHelper;
class PackedClearValuesArray;
class AttachmentOpsArray;

using PipelineLayoutPtr      = AtomicSharedPtr<PipelineLayout>;
using DescriptorSetLayoutPtr = AtomicSharedPtr<DescriptorSetLayout>;

// Packed Vk resource descriptions.
// Most Vk types use many more bits than required to represent the underlying data.
// Since ANGLE wants to cache things like RenderPasses and Pipeline State Objects using
// hashing (and also needs to check equality) we can optimize these operations by
// using fewer bits. Hence the packed types.
//
// One implementation note: these types could potentially be improved by using even
// fewer bits. For example, boolean values could be represented by a single bit instead
// of a uint8_t. However at the current time there are concerns about the portability
// of bitfield operators, and complexity issues with using bit mask operations. This is
// something we will likely want to investigate as the Vulkan implementation progresses.
//
// Second implementation note: the struct packing is also a bit fragile, and some of the
// packing requirements depend on using alignas and field ordering to get the result of
// packing nicely into the desired space. This is something we could also potentially fix
// with a redesign to use bitfields or bit mask operations.

// Enable struct padding warnings for the code below since it is used in caches.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

enum class ResourceAccess
{
    Unused    = 0x0,
    ReadOnly  = 0x1,
    WriteOnly = 0x2,
    ReadWrite = ReadOnly | WriteOnly,
};

inline void UpdateAccess(ResourceAccess *oldAccess, ResourceAccess newAccess)
{
    *oldAccess = static_cast<ResourceAccess>(ToUnderlying(newAccess) | ToUnderlying(*oldAccess));
}
inline bool HasResourceWriteAccess(ResourceAccess access)
{
    return (ToUnderlying(access) & ToUnderlying(ResourceAccess::WriteOnly)) != 0;
}

enum class RenderPassLoadOp
{
    Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
    Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    None,
};
enum class RenderPassStoreOp
{
    Store    = VK_ATTACHMENT_STORE_OP_STORE,
    DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    None,
};

enum class FramebufferFetchMode
{
    None,
    Color,
    DepthStencil,
    ColorAndDepthStencil,
};
FramebufferFetchMode GetProgramFramebufferFetchMode(const gl::ProgramExecutable *executable);
ANGLE_INLINE bool FramebufferFetchModeHasColor(FramebufferFetchMode framebufferFetchMode)
{
    static_assert(ToUnderlying(FramebufferFetchMode::Color) == 0x1);
    static_assert(ToUnderlying(FramebufferFetchMode::ColorAndDepthStencil) == 0x3);
    return (ToUnderlying(framebufferFetchMode) & 0x1) != 0;
}
ANGLE_INLINE bool FramebufferFetchModeHasDepthStencil(FramebufferFetchMode framebufferFetchMode)
{
    static_assert(ToUnderlying(FramebufferFetchMode::DepthStencil) == 0x2);
    static_assert(ToUnderlying(FramebufferFetchMode::ColorAndDepthStencil) == 0x3);
    return (ToUnderlying(framebufferFetchMode) & 0x2) != 0;
}
ANGLE_INLINE FramebufferFetchMode FramebufferFetchModeMerge(FramebufferFetchMode mode1,
                                                            FramebufferFetchMode mode2)
{
    constexpr uint32_t kNone         = ToUnderlying(FramebufferFetchMode::None);
    constexpr uint32_t kColor        = ToUnderlying(FramebufferFetchMode::Color);
    constexpr uint32_t kDepthStencil = ToUnderlying(FramebufferFetchMode::DepthStencil);
    constexpr uint32_t kColorAndDepthStencil =
        ToUnderlying(FramebufferFetchMode::ColorAndDepthStencil);
    static_assert(kNone == 0);
    static_assert((kColor & kColorAndDepthStencil) == kColor);
    static_assert((kDepthStencil & kColorAndDepthStencil) == kDepthStencil);
    static_assert((kColor | kDepthStencil) == kColorAndDepthStencil);

    return static_cast<FramebufferFetchMode>(ToUnderlying(mode1) | ToUnderlying(mode2));
}

// There can be a maximum of IMPLEMENTATION_MAX_DRAW_BUFFERS color and resolve attachments, plus -
// - one depth/stencil attachment
// - one depth/stencil resolve attachment
// - one fragment shading rate attachment
constexpr size_t kMaxFramebufferAttachments = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS * 2 + 3;
template <typename T>
using FramebufferAttachmentArray = std::array<T, kMaxFramebufferAttachments>;
template <typename T>
using FramebufferAttachmentsVector = angle::FixedVector<T, kMaxFramebufferAttachments>;
using FramebufferAttachmentMask    = angle::BitSet<kMaxFramebufferAttachments>;

constexpr size_t kMaxFramebufferNonResolveAttachments = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS + 1;
template <typename T>
using FramebufferNonResolveAttachmentArray = std::array<T, kMaxFramebufferNonResolveAttachments>;
using FramebufferNonResolveAttachmentMask  = angle::BitSet16<kMaxFramebufferNonResolveAttachments>;

class PackedAttachmentIndex;

class alignas(4) RenderPassDesc final
{
  public:
    RenderPassDesc();
    ~RenderPassDesc();
    RenderPassDesc(const RenderPassDesc &other);
    RenderPassDesc &operator=(const RenderPassDesc &other);

    // Set format for an enabled GL color attachment.
    void packColorAttachment(size_t colorIndexGL, angle::FormatID formatID);
    // Mark a GL color attachment index as disabled.
    void packColorAttachmentGap(size_t colorIndexGL);
    // The caller must pack the depth/stencil attachment last, which is packed right after the color
    // attachments (including gaps), i.e. with an index starting from |colorAttachmentRange()|.
    void packDepthStencilAttachment(angle::FormatID angleFormatID);
    void updateDepthStencilAccess(ResourceAccess access);
    // Indicate that a color attachment should have a corresponding resolve attachment.
    void packColorResolveAttachment(size_t colorIndexGL);
    // Indicate that a YUV texture is attached to the resolve attachment.
    void packYUVResolveAttachment(size_t colorIndexGL);
    // Remove the resolve attachment.  Used when optimizing blit through resolve attachment to
    // temporarily pack a resolve attachment and then remove it.
    void removeColorResolveAttachment(size_t colorIndexGL);
    // Indicate that a color attachment should take its data from the resolve attachment initially.
    void packColorUnresolveAttachment(size_t colorIndexGL);
    void removeColorUnresolveAttachment(size_t colorIndexGL);
    // Indicate that a depth/stencil attachment should have a corresponding resolve attachment.
    void packDepthResolveAttachment();
    void packStencilResolveAttachment();
    // Indicate that a depth/stencil attachment should take its data from the resolve attachment
    // initially.
    void packDepthUnresolveAttachment();
    void packStencilUnresolveAttachment();
    void removeDepthStencilUnresolveAttachment();

    PackedAttachmentIndex getPackedColorAttachmentIndex(size_t colorIndexGL);

    void setWriteControlMode(gl::SrgbWriteControlMode mode);

    size_t hash() const;

    // Color attachments are in [0, colorAttachmentRange()), with possible gaps.
    size_t colorAttachmentRange() const { return mColorAttachmentRange; }
    size_t depthStencilAttachmentIndex() const { return colorAttachmentRange(); }

    bool isColorAttachmentEnabled(size_t colorIndexGL) const;
    bool hasYUVResolveAttachment() const { return mIsYUVResolve; }
    bool hasDepthStencilAttachment() const;
    gl::DrawBufferMask getColorResolveAttachmentMask() const { return mColorResolveAttachmentMask; }
    bool hasColorResolveAttachment(size_t colorIndexGL) const
    {
        return mColorResolveAttachmentMask.test(colorIndexGL);
    }
    gl::DrawBufferMask getColorUnresolveAttachmentMask() const
    {
        return mColorUnresolveAttachmentMask;
    }
    bool hasColorUnresolveAttachment(size_t colorIndexGL) const
    {
        return mColorUnresolveAttachmentMask.test(colorIndexGL);
    }
    bool hasDepthStencilResolveAttachment() const { return mResolveDepth || mResolveStencil; }
    bool hasDepthResolveAttachment() const { return mResolveDepth; }
    bool hasStencilResolveAttachment() const { return mResolveStencil; }
    bool hasDepthStencilUnresolveAttachment() const { return mUnresolveDepth || mUnresolveStencil; }
    bool hasDepthUnresolveAttachment() const { return mUnresolveDepth; }
    bool hasStencilUnresolveAttachment() const { return mUnresolveStencil; }
    gl::SrgbWriteControlMode getSRGBWriteControlMode() const
    {
        return static_cast<gl::SrgbWriteControlMode>(mSrgbWriteControl);
    }

    bool isLegacyDitherEnabled() const { return mLegacyDitherEnabled; }

    void setLegacyDither(bool enabled);

    // Get the number of clearable attachments in the Vulkan render pass, i.e. after removing
    // disabled color attachments.
    size_t clearableAttachmentCount() const;
    // Get the total number of attachments in the Vulkan render pass, i.e. after removing disabled
    // color attachments.
    size_t attachmentCount() const;

    void setSamples(GLint samples) { mSamples = static_cast<uint8_t>(samples); }
    uint8_t samples() const { return mSamples; }

    void setViewCount(GLsizei viewCount) { mViewCount = static_cast<uint8_t>(viewCount); }
    uint8_t viewCount() const { return mViewCount; }

    void setFramebufferFetchMode(FramebufferFetchMode framebufferFetchMode)
    {
        SetBitField(mFramebufferFetchMode, framebufferFetchMode);
    }
    FramebufferFetchMode framebufferFetchMode() const
    {
        return static_cast<FramebufferFetchMode>(mFramebufferFetchMode);
    }
    bool hasColorFramebufferFetch() const
    {
        return FramebufferFetchModeHasColor(framebufferFetchMode());
    }
    bool hasDepthStencilFramebufferFetch() const
    {
        return FramebufferFetchModeHasDepthStencil(framebufferFetchMode());
    }

    void updateRenderToTexture(bool isRenderToTexture) { mIsRenderToTexture = isRenderToTexture; }
    bool isRenderToTexture() const { return mIsRenderToTexture; }

    void setFragmentShadingAttachment(bool value) { mHasFragmentShadingAttachment = value; }
    bool hasFragmentShadingAttachment() const { return mHasFragmentShadingAttachment; }

    angle::FormatID operator[](size_t index) const
    {
        ASSERT(index < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS + 1);
        return static_cast<angle::FormatID>(mAttachmentFormats[index]);
    }

    // Start a render pass with a render pass object.
    void beginRenderPass(ErrorContext *context,
                         PrimaryCommandBuffer *primary,
                         const RenderPass &renderPass,
                         VkFramebuffer framebuffer,
                         const gl::Rectangle &renderArea,
                         VkSubpassContents subpassContents,
                         PackedClearValuesArray &clearValues,
                         const VkRenderPassAttachmentBeginInfo *attachmentBeginInfo) const;

    // Start a render pass with dynamic rendering.
    void beginRendering(ErrorContext *context,
                        PrimaryCommandBuffer *primary,
                        const gl::Rectangle &renderArea,
                        VkSubpassContents subpassContents,
                        const FramebufferAttachmentsVector<VkImageView> &attachmentViews,
                        const AttachmentOpsArray &ops,
                        PackedClearValuesArray &clearValues,
                        uint32_t layerCount) const;

    void populateRenderingInheritanceInfo(
        Renderer *renderer,
        VkCommandBufferInheritanceRenderingInfo *infoOut,
        gl::DrawBuffersArray<VkFormat> *colorFormatStorageOut) const;

    // Calculate perf counters for a dynamic rendering render pass instance.  For render pass
    // objects, the perf counters are updated when creating the render pass, where access to
    // ContextVk is available.
    void updatePerfCounters(ErrorContext *context,
                            const FramebufferAttachmentsVector<VkImageView> &attachmentViews,
                            const AttachmentOpsArray &ops,
                            angle::VulkanPerfCounters *countersOut);

  private:
    uint8_t mSamples;
    uint8_t mColorAttachmentRange;

    // Multiview
    uint8_t mViewCount;

    // sRGB
    uint8_t mSrgbWriteControl : 1;

    // Framebuffer fetch, one of FramebufferFetchMode values
    uint8_t mFramebufferFetchMode : 2;

    // Depth/stencil resolve
    uint8_t mResolveDepth : 1;
    uint8_t mResolveStencil : 1;

    // Multisampled render to texture
    uint8_t mIsRenderToTexture : 1;
    uint8_t mUnresolveDepth : 1;
    uint8_t mUnresolveStencil : 1;

    // Dithering state when using VK_EXT_legacy_dithering
    uint8_t mLegacyDitherEnabled : 1;

    // external_format_resolve
    uint8_t mIsYUVResolve : 1;

    // Foveated rendering
    uint8_t mHasFragmentShadingAttachment : 1;

    // Available space for expansion.
    uint8_t mPadding2 : 5;

    // Whether each color attachment has a corresponding resolve attachment.  Color resolve
    // attachments can be used to optimize resolve through glBlitFramebuffer() as well as support
    // GL_EXT_multisampled_render_to_texture and GL_EXT_multisampled_render_to_texture2.
    gl::DrawBufferMask mColorResolveAttachmentMask;

    // Whether each color attachment with a corresponding resolve attachment should be initialized
    // with said resolve attachment in an initial subpass.  This is an optimization to avoid
    // loadOp=LOAD on the implicit multisampled image used with multisampled-render-to-texture
    // render targets.  This operation is referred to as "unresolve".
    //
    // Unused when VK_EXT_multisampled_render_to_single_sampled is available.
    gl::DrawBufferMask mColorUnresolveAttachmentMask;

    // Color attachment formats are stored with their GL attachment indices.  The depth/stencil
    // attachment formats follow the last enabled color attachment.  When creating a render pass,
    // the disabled attachments are removed and the resulting attachments are packed.
    //
    // The attachment indices provided as input to various functions in this file are thus GL
    // attachment indices.  These indices are marked as such, e.g. colorIndexGL.  The render pass
    // (and corresponding framebuffer object) lists the packed attachments, with the corresponding
    // indices marked with Vk, e.g. colorIndexVk.  The subpass attachment references create the
    // link between the two index spaces.  The subpass declares attachment references with GL
    // indices (which corresponds to the location decoration of shader outputs).  The attachment
    // references then contain the Vulkan indices or VK_ATTACHMENT_UNUSED.
    //
    // For example, if GL uses color attachments 0 and 3, then there are two render pass
    // attachments (indexed 0 and 1) and 4 subpass attachments:
    //
    //  - Subpass attachment 0 -> Renderpass attachment 0
    //  - Subpass attachment 1 -> VK_ATTACHMENT_UNUSED
    //  - Subpass attachment 2 -> VK_ATTACHMENT_UNUSED
    //  - Subpass attachment 3 -> Renderpass attachment 1
    //
    // The resolve attachments are packed after the non-resolve attachments.  They use the same
    // formats, so they are not specified in this array.
    FramebufferNonResolveAttachmentArray<uint8_t> mAttachmentFormats;
};

bool operator==(const RenderPassDesc &lhs, const RenderPassDesc &rhs);

constexpr size_t kRenderPassDescSize = sizeof(RenderPassDesc);
static_assert(kRenderPassDescSize == 16, "Size check failed");

enum class GraphicsPipelineSubset
{
    Complete,  // Including all subsets
    VertexInput,
    Shaders,
    FragmentOutput,
};

enum class CacheLookUpFeedback
{
    None,
    Hit,
    Miss,
    LinkedDrawHit,
    LinkedDrawMiss,
    WarmUpHit,
    WarmUpMiss,
    UtilsHit,
    UtilsMiss,
};

struct PackedAttachmentOpsDesc final
{
    // RenderPassLoadOp is in range [0, 3], and RenderPassStoreOp is in range [0, 2].
    uint16_t loadOp : 2;
    uint16_t storeOp : 2;
    uint16_t stencilLoadOp : 2;
    uint16_t stencilStoreOp : 2;
    // If a corresponding resolve attachment exists, storeOp may already be DONT_CARE, and it's
    // unclear whether the attachment was invalidated or not.  This information is passed along here
    // so that the resolve attachment's storeOp can be set to DONT_CARE if the attachment is
    // invalidated, and if possible removed from the list of resolve attachments altogether.  Note
    // that the latter may not be possible if the render pass has multiple subpasses due to Vulkan
    // render pass compatibility rules (not an issue with dynamic rendering).
    uint16_t isInvalidated : 1;
    uint16_t isStencilInvalidated : 1;
    uint16_t padding1 : 6;

    // Layouts take values from ImageLayout, so they are small.  Layouts that are possible here are
    // placed at the beginning of that enum.
    uint16_t initialLayout : 5;
    uint16_t finalLayout : 5;
    uint16_t finalResolveLayout : 5;
    uint16_t padding2 : 1;
};

static_assert(sizeof(PackedAttachmentOpsDesc) == 4, "Size check failed");

class AttachmentOpsArray final
{
  public:
    AttachmentOpsArray();
    ~AttachmentOpsArray();
    AttachmentOpsArray(const AttachmentOpsArray &other);
    AttachmentOpsArray &operator=(const AttachmentOpsArray &other);

    const PackedAttachmentOpsDesc &operator[](PackedAttachmentIndex index) const
    {
        return mOps[index.get()];
    }
    PackedAttachmentOpsDesc &operator[](PackedAttachmentIndex index) { return mOps[index.get()]; }

    // Initialize an attachment op with all load and store operations.
    void initWithLoadStore(PackedAttachmentIndex index,
                           ImageLayout initialLayout,
                           ImageLayout finalLayout);

    void setLayouts(PackedAttachmentIndex index,
                    ImageLayout initialLayout,
                    ImageLayout finalLayout);
    void setOps(PackedAttachmentIndex index, RenderPassLoadOp loadOp, RenderPassStoreOp storeOp);
    void setStencilOps(PackedAttachmentIndex index,
                       RenderPassLoadOp loadOp,
                       RenderPassStoreOp storeOp);

    void setClearOp(PackedAttachmentIndex index);
    void setClearStencilOp(PackedAttachmentIndex index);

    size_t hash() const;

  private:
    gl::AttachmentArray<PackedAttachmentOpsDesc> mOps;
};

bool operator==(const AttachmentOpsArray &lhs, const AttachmentOpsArray &rhs);

static_assert(sizeof(AttachmentOpsArray) == 40, "Size check failed");

struct PackedAttribDesc final
{
    uint8_t format;
    uint8_t divisor;
    uint16_t offset : kAttributeOffsetMaxBits;
    uint16_t compressed : 1;
};

constexpr size_t kPackedAttribDescSize = sizeof(PackedAttribDesc);
static_assert(kPackedAttribDescSize == 4, "Size mismatch");

struct PackedVertexInputAttributes final
{
    PackedAttribDesc attribs[gl::MAX_VERTEX_ATTRIBS];

    // Component type of the corresponding input in the program.  Used to adjust the format if
    // necessary.  Takes values from gl::ComponentType.
    uint32_t shaderAttribComponentType;

    // Although technically stride can be any value in ES 2.0, in practice supporting stride
    // greater than MAX_USHORT should not be that helpful. Note that stride limits are
    // introduced in ES 3.1.
    // Dynamic in VK_EXT_extended_dynamic_state
    uint16_t strides[gl::MAX_VERTEX_ATTRIBS];
};

constexpr size_t kPackedVertexInputAttributesSize = sizeof(PackedVertexInputAttributes);
static_assert(kPackedVertexInputAttributesSize == 100, "Size mismatch");

struct PackedInputAssemblyState final
{
    struct
    {
        uint32_t topology : 4;

        // Dynamic in VK_EXT_extended_dynamic_state2
        uint32_t primitiveRestartEnable : 1;  // ds2

        // Whether dynamic state for vertex stride from VK_EXT_extended_dynamic_state can be used
        // for.  Used by GraphicsPipelineDesc::hash() to exclude |vertexStrides| from the hash
        uint32_t useVertexInputBindingStrideDynamicState : 1;

        // Whether dynamic state for vertex input state from VK_EXT_vertex_input_dynamic_state can
        // be used by GraphicsPipelineDesc::hash() to exclude |PackedVertexInputAttributes| from the
        // hash
        uint32_t useVertexInputDynamicState : 1;

        // Whether the pipeline is robust (vertex input copy)
        uint32_t isRobustContext : 1;
        // Whether the pipeline needs access to protected content (vertex input copy)
        uint32_t isProtectedContext : 1;

        // Which attributes are actually active in the program and should affect the pipeline.
        uint32_t programActiveAttributeLocations : gl::MAX_VERTEX_ATTRIBS;

        uint32_t padding : 23 - gl::MAX_VERTEX_ATTRIBS;
    } bits;
};

constexpr size_t kPackedInputAssemblyStateSize = sizeof(PackedInputAssemblyState);
static_assert(kPackedInputAssemblyStateSize == 4, "Size mismatch");

struct PackedStencilOpState final
{
    uint8_t fail : 4;
    uint8_t pass : 4;
    uint8_t depthFail : 4;
    uint8_t compare : 4;
};

constexpr size_t kPackedStencilOpSize = sizeof(PackedStencilOpState);
static_assert(kPackedStencilOpSize == 2, "Size check failed");

struct PackedPreRasterizationAndFragmentStates final
{
    struct
    {
        // Affecting VkPipelineViewportStateCreateInfo
        uint32_t viewportNegativeOneToOne : 1;

        // Affecting VkPipelineRasterizationStateCreateInfo
        uint32_t depthClampEnable : 1;
        uint32_t polygonMode : 2;
        // Dynamic in VK_EXT_extended_dynamic_state
        uint32_t cullMode : 4;
        uint32_t frontFace : 4;
        // Dynamic in VK_EXT_extended_dynamic_state2
        uint32_t rasterizerDiscardEnable : 1;
        uint32_t depthBiasEnable : 1;

        // Affecting VkPipelineTessellationStateCreateInfo
        uint32_t patchVertices : 6;

        // Affecting VkPipelineDepthStencilStateCreateInfo
        uint32_t depthBoundsTest : 1;
        // Dynamic in VK_EXT_extended_dynamic_state
        uint32_t depthTest : 1;
        uint32_t depthWrite : 1;
        uint32_t stencilTest : 1;
        uint32_t nonZeroStencilWriteMaskWorkaround : 1;
        // Dynamic in VK_EXT_extended_dynamic_state2
        uint32_t depthCompareOp : 4;

        // Affecting specialization constants
        uint32_t surfaceRotation : 1;

        // Whether the pipeline is robust (shader stages copy)
        uint32_t isRobustContext : 1;
        // Whether the pipeline needs access to protected content (shader stages copy)
        uint32_t isProtectedContext : 1;
    } bits;

    // Affecting specialization constants
    static_assert(gl::IMPLEMENTATION_MAX_DRAW_BUFFERS <= 8,
                  "2 bits per draw buffer is needed for dither emulation");
    uint16_t emulatedDitherControl;
    uint16_t padding;

    // Affecting VkPipelineDepthStencilStateCreateInfo
    // Dynamic in VK_EXT_extended_dynamic_state
    PackedStencilOpState front;
    PackedStencilOpState back;
};

constexpr size_t kPackedPreRasterizationAndFragmentStatesSize =
    sizeof(PackedPreRasterizationAndFragmentStates);
static_assert(kPackedPreRasterizationAndFragmentStatesSize == 12, "Size check failed");

struct PackedMultisampleAndSubpassState final
{
    struct
    {
        // Affecting VkPipelineMultisampleStateCreateInfo
        // Note: Only up to 16xMSAA is supported in the Vulkan backend.
        uint16_t sampleMask;
        // Stored as minus one so sample count 16 can fit in 4 bits.
        uint16_t rasterizationSamplesMinusOne : 4;
        uint16_t sampleShadingEnable : 1;
        uint16_t alphaToCoverageEnable : 1;
        uint16_t alphaToOneEnable : 1;
        // The subpass index affects both the shader stages and the fragment output similarly to
        // multisampled state, so they are grouped together.
        // Note: Currently only 2 subpasses possible.
        uint16_t subpass : 1;
        // 8-bit normalized instead of float to align the struct.
        uint16_t minSampleShading : 8;
    } bits;
};

constexpr size_t kPackedMultisampleAndSubpassStateSize = sizeof(PackedMultisampleAndSubpassState);
static_assert(kPackedMultisampleAndSubpassStateSize == 4, "Size check failed");

struct PackedColorBlendAttachmentState final
{
    uint16_t srcColorBlendFactor : 5;
    uint16_t dstColorBlendFactor : 5;
    uint16_t colorBlendOp : 6;
    uint16_t srcAlphaBlendFactor : 5;
    uint16_t dstAlphaBlendFactor : 5;
    uint16_t alphaBlendOp : 6;
};

constexpr size_t kPackedColorBlendAttachmentStateSize = sizeof(PackedColorBlendAttachmentState);
static_assert(kPackedColorBlendAttachmentStateSize == 4, "Size check failed");

struct PackedColorBlendState final
{
    uint8_t colorWriteMaskBits[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS / 2];
    PackedColorBlendAttachmentState attachments[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS];
};

constexpr size_t kPackedColorBlendStateSize = sizeof(PackedColorBlendState);
static_assert(kPackedColorBlendStateSize == 36, "Size check failed");

struct PackedBlendMaskAndLogicOpState final
{
    struct
    {
        uint32_t blendEnableMask : 8;
        uint32_t logicOpEnable : 1;
        // Dynamic in VK_EXT_extended_dynamic_state2
        uint32_t logicOp : 4;

        // Whether the pipeline needs access to protected content (fragment output copy)
        uint32_t isProtectedContext : 1;

        // Output that is present in the framebuffer but is never written to in the shader.  Used by
        // GL_ANGLE_robust_fragment_shader_output which defines the behavior in this case (which is
        // to mask these outputs)
        uint32_t missingOutputsMask : gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;

        uint32_t padding : 18 - gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
    } bits;
};

constexpr size_t kPackedBlendMaskAndLogicOpStateSize = sizeof(PackedBlendMaskAndLogicOpState);
static_assert(kPackedBlendMaskAndLogicOpStateSize == 4, "Size check failed");

// The vertex input subset of the pipeline.
struct PipelineVertexInputState final
{
    PackedInputAssemblyState inputAssembly;
    PackedVertexInputAttributes vertex;
};

// The pre-rasterization and fragment shader subsets of the pipeline.  This is excluding
// multisampled and render pass states which are shared with fragment output.
struct PipelineShadersState final
{
    PackedPreRasterizationAndFragmentStates shaders;
};

// Multisampled and render pass states.
struct PipelineSharedNonVertexInputState final
{
    PackedMultisampleAndSubpassState multisample;
    RenderPassDesc renderPass;
};

// The fragment output subset of the pipeline.  This is excluding multisampled and render pass
// states which are shared with the shader subsets.
struct PipelineFragmentOutputState final
{
    PackedColorBlendState blend;
    PackedBlendMaskAndLogicOpState blendMaskAndLogic;
};

constexpr size_t kGraphicsPipelineVertexInputStateSize =
    kPackedVertexInputAttributesSize + kPackedInputAssemblyStateSize;
constexpr size_t kGraphicsPipelineShadersStateSize = kPackedPreRasterizationAndFragmentStatesSize;
constexpr size_t kGraphicsPipelineSharedNonVertexInputStateSize =
    kPackedMultisampleAndSubpassStateSize + kRenderPassDescSize;
constexpr size_t kGraphicsPipelineFragmentOutputStateSize =
    kPackedColorBlendStateSize + kPackedBlendMaskAndLogicOpStateSize;

constexpr size_t kGraphicsPipelineDescSumOfSizes =
    kGraphicsPipelineVertexInputStateSize + kGraphicsPipelineShadersStateSize +
    kGraphicsPipelineSharedNonVertexInputStateSize + kGraphicsPipelineFragmentOutputStateSize;

// Number of dirty bits in the dirty bit set.
constexpr size_t kGraphicsPipelineDirtyBitBytes = 4;
constexpr static size_t kNumGraphicsPipelineDirtyBits =
    kGraphicsPipelineDescSumOfSizes / kGraphicsPipelineDirtyBitBytes;
static_assert(kNumGraphicsPipelineDirtyBits <= 64, "Too many pipeline dirty bits");

// Set of dirty bits. Each bit represents kGraphicsPipelineDirtyBitBytes in the desc.
using GraphicsPipelineTransitionBits = angle::BitSet<kNumGraphicsPipelineDirtyBits>;

GraphicsPipelineTransitionBits GetGraphicsPipelineTransitionBitsMask(GraphicsPipelineSubset subset);

// Disable padding warnings for a few helper structs that aggregate Vulkan state objects.  These are
// not used as hash keys, they just simplify passing them around to functions.
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

struct GraphicsPipelineVertexInputVulkanStructs
{
    VkPipelineVertexInputStateCreateInfo vertexInputState       = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState   = {};
    VkPipelineVertexInputDivisorStateCreateInfoEXT divisorState = {};

    // Support storage
    gl::AttribArray<VkVertexInputBindingDescription> bindingDescs;
    gl::AttribArray<VkVertexInputAttributeDescription> attributeDescs;
    gl::AttribArray<VkVertexInputBindingDivisorDescriptionEXT> divisorDesc;
};

struct GraphicsPipelineShadersVulkanStructs
{
    VkPipelineViewportStateCreateInfo viewportState                               = {};
    VkPipelineRasterizationStateCreateInfo rasterState                            = {};
    VkPipelineDepthStencilStateCreateInfo depthStencilState                       = {};
    VkPipelineTessellationStateCreateInfo tessellationState                       = {};
    VkPipelineTessellationDomainOriginStateCreateInfo domainOriginState           = {};
    VkPipelineViewportDepthClipControlCreateInfoEXT depthClipControl              = {};
    VkPipelineRasterizationLineStateCreateInfoEXT rasterLineState                 = {};
    VkPipelineRasterizationProvokingVertexStateCreateInfoEXT provokingVertexState = {};
    VkPipelineRasterizationStateStreamCreateInfoEXT rasterStreamState             = {};
    VkSpecializationInfo specializationInfo                                       = {};

    // Support storage
    angle::FixedVector<VkPipelineShaderStageCreateInfo, 5> shaderStages;
    SpecializationConstantMap<VkSpecializationMapEntry> specializationEntries;
};

struct GraphicsPipelineSharedNonVertexInputVulkanStructs
{
    VkPipelineMultisampleStateCreateInfo multisampleState = {};

    // Support storage
    uint32_t sampleMask;
};

struct GraphicsPipelineFragmentOutputVulkanStructs
{
    VkPipelineColorBlendStateCreateInfo blendState = {};

    // Support storage
    gl::DrawBuffersArray<VkPipelineColorBlendAttachmentState> blendAttachmentState;
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

using GraphicsPipelineDynamicStateList = angle::FixedVector<VkDynamicState, 23>;

enum class PipelineRobustness
{
    NonRobust,
    Robust,
};

enum class PipelineProtectedAccess
{
    Unprotected,
    Protected,
};

// Context state that can affect a compute pipeline
union ComputePipelineOptions final
{
    struct
    {
        // Whether VK_EXT_pipeline_robustness should be used to make the pipeline robust.  Note that
        // programs are allowed to be shared between robust and non-robust contexts, so different
        // pipelines can be created for the same compute program.
        uint8_t robustness : 1;
        // Whether VK_EXT_pipeline_protected_access should be used to make the pipeline
        // protected-only. Similar to robustness, EGL allows protected and unprotected to be in the
        // same share group.
        uint8_t protectedAccess : 1;
        uint8_t reserved : 6;  // must initialize to zero
    };
    uint8_t permutationIndex;
    static constexpr uint32_t kPermutationCount = 0x1 << 2;
};
static_assert(sizeof(ComputePipelineOptions) == 1, "Size check failed");
ComputePipelineOptions GetComputePipelineOptions(vk::PipelineRobustness robustness,
                                                 vk::PipelineProtectedAccess protectedAccess);

// Compute Pipeline Description
class ComputePipelineDesc final
{
  public:
    void *operator new(std::size_t size);
    void operator delete(void *ptr);

    ComputePipelineDesc();
    ComputePipelineDesc(const ComputePipelineDesc &other);
    ComputePipelineDesc &operator=(const ComputePipelineDesc &other);

    ComputePipelineDesc(VkSpecializationInfo *specializationInfo,
                        vk::ComputePipelineOptions pipelineOptions);
    ~ComputePipelineDesc() = default;

    size_t hash() const;
    bool keyEqual(const ComputePipelineDesc &other) const;

    template <typename T>
    const T *getPtr() const
    {
        return reinterpret_cast<const T *>(this);
    }

    std::vector<uint32_t> getConstantIds() const { return mConstantIds; }
    std::vector<uint32_t> getConstants() const { return mConstants; }
    ComputePipelineOptions getPipelineOptions() const { return mPipelineOptions; }

  private:
    std::vector<uint32_t> mConstantIds, mConstants;
    ComputePipelineOptions mPipelineOptions = {};
    char mPadding[7]                        = {};
};

// State changes are applied through the update methods. Each update method can also have a
// sibling method that applies the update without marking a state transition. The non-transition
// update methods are used for internal shader pipelines. Not every non-transition update method
// is implemented yet as not every state is used in internal shaders.
class GraphicsPipelineDesc final
{
  public:
    // Use aligned allocation and free so we can use the alignas keyword.
    void *operator new(std::size_t size);
    void operator delete(void *ptr);

    GraphicsPipelineDesc();
    ~GraphicsPipelineDesc();
    GraphicsPipelineDesc(const GraphicsPipelineDesc &other);
    GraphicsPipelineDesc &operator=(const GraphicsPipelineDesc &other);

    size_t hash(GraphicsPipelineSubset subset) const;
    bool keyEqual(const GraphicsPipelineDesc &other, GraphicsPipelineSubset subset) const;

    void initDefaults(const ErrorContext *context,
                      GraphicsPipelineSubset subset,
                      PipelineRobustness contextRobustness,
                      PipelineProtectedAccess contextProtectedAccess);

    // For custom comparisons.
    template <typename T>
    const T *getPtr() const
    {
        return reinterpret_cast<const T *>(this);
    }

    VkResult initializePipeline(ErrorContext *context,
                                PipelineCacheAccess *pipelineCache,
                                GraphicsPipelineSubset subset,
                                const RenderPass &compatibleRenderPass,
                                const PipelineLayout &pipelineLayout,
                                const ShaderModuleMap &shaders,
                                const SpecializationConstants &specConsts,
                                Pipeline *pipelineOut,
                                CacheLookUpFeedback *feedbackOut) const;

    // Vertex input state. For ES 3.1 this should be separated into binding and attribute.
    void updateVertexInput(ContextVk *contextVk,
                           GraphicsPipelineTransitionBits *transition,
                           uint32_t attribIndex,
                           GLuint stride,
                           GLuint divisor,
                           angle::FormatID format,
                           bool compressed,
                           GLuint relativeOffset);
    void setVertexShaderComponentTypes(gl::AttributesMask activeAttribLocations,
                                       gl::ComponentTypeMask componentTypeMask);
    void updateVertexShaderComponentTypes(GraphicsPipelineTransitionBits *transition,
                                          gl::AttributesMask activeAttribLocations,
                                          gl::ComponentTypeMask componentTypeMask);

    // Input assembly info
    void setTopology(gl::PrimitiveMode drawMode);
    void updateTopology(GraphicsPipelineTransitionBits *transition, gl::PrimitiveMode drawMode);
    void updatePrimitiveRestartEnabled(GraphicsPipelineTransitionBits *transition,
                                       bool primitiveRestartEnabled);

    // Viewport states
    void updateDepthClipControl(GraphicsPipelineTransitionBits *transition, bool negativeOneToOne);

    // Raster states
    void updatePolygonMode(GraphicsPipelineTransitionBits *transition, gl::PolygonMode polygonMode);
    void updateCullMode(GraphicsPipelineTransitionBits *transition,
                        const gl::RasterizerState &rasterState);
    void updateFrontFace(GraphicsPipelineTransitionBits *transition,
                         const gl::RasterizerState &rasterState,
                         bool invertFrontFace);
    void updateRasterizerDiscardEnabled(GraphicsPipelineTransitionBits *transition,
                                        bool rasterizerDiscardEnabled);

    // Multisample states
    uint32_t getRasterizationSamples() const;
    void setRasterizationSamples(uint32_t rasterizationSamples);
    void updateRasterizationSamples(GraphicsPipelineTransitionBits *transition,
                                    uint32_t rasterizationSamples);
    void updateAlphaToCoverageEnable(GraphicsPipelineTransitionBits *transition, bool enable);
    void updateAlphaToOneEnable(GraphicsPipelineTransitionBits *transition, bool enable);
    void updateSampleMask(GraphicsPipelineTransitionBits *transition,
                          uint32_t maskNumber,
                          uint32_t mask);

    void updateSampleShading(GraphicsPipelineTransitionBits *transition, bool enable, float value);

    // RenderPass description.
    const RenderPassDesc &getRenderPassDesc() const { return mSharedNonVertexInput.renderPass; }

    void setRenderPassDesc(const RenderPassDesc &renderPassDesc);
    void updateRenderPassDesc(GraphicsPipelineTransitionBits *transition,
                              const angle::FeaturesVk &features,
                              const RenderPassDesc &renderPassDesc,
                              FramebufferFetchMode framebufferFetchMode);
    void setRenderPassSampleCount(GLint samples);
    void setRenderPassFramebufferFetchMode(FramebufferFetchMode framebufferFetchMode);
    bool getRenderPassColorFramebufferFetchMode() const
    {
        return mSharedNonVertexInput.renderPass.hasColorFramebufferFetch();
    }
    bool getRenderPassDepthStencilFramebufferFetchMode() const
    {
        return mSharedNonVertexInput.renderPass.hasDepthStencilFramebufferFetch();
    }

    void setRenderPassFoveation(bool isFoveated);
    bool getRenderPassFoveation() const
    {
        return mSharedNonVertexInput.renderPass.hasFragmentShadingAttachment();
    }

    void setRenderPassColorAttachmentFormat(size_t colorIndexGL, angle::FormatID formatID);

    // Blend states
    void setSingleBlend(uint32_t colorIndexGL,
                        bool enabled,
                        VkBlendOp op,
                        VkBlendFactor srcFactor,
                        VkBlendFactor dstFactor);
    void updateBlendEnabled(GraphicsPipelineTransitionBits *transition,
                            gl::DrawBufferMask blendEnabledMask);
    void updateBlendFuncs(GraphicsPipelineTransitionBits *transition,
                          const gl::BlendStateExt &blendStateExt,
                          gl::DrawBufferMask attachmentMask);
    void updateBlendEquations(GraphicsPipelineTransitionBits *transition,
                              const gl::BlendStateExt &blendStateExt,
                              gl::DrawBufferMask attachmentMask);
    void resetBlendFuncsAndEquations(GraphicsPipelineTransitionBits *transition,
                                     const gl::BlendStateExt &blendStateExt,
                                     gl::DrawBufferMask previousAttachmentsMask,
                                     gl::DrawBufferMask newAttachmentsMask);
    void setColorWriteMasks(gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
                            const gl::DrawBufferMask &alphaMask,
                            const gl::DrawBufferMask &enabledDrawBuffers);
    void setSingleColorWriteMask(uint32_t colorIndexGL, VkColorComponentFlags colorComponentFlags);
    void updateColorWriteMasks(GraphicsPipelineTransitionBits *transition,
                               gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
                               const gl::DrawBufferMask &alphaMask,
                               const gl::DrawBufferMask &enabledDrawBuffers);
    void updateMissingOutputsMask(GraphicsPipelineTransitionBits *transition,
                                  gl::DrawBufferMask missingOutputsMask);

    // Logic op
    void updateLogicOpEnabled(GraphicsPipelineTransitionBits *transition, bool enable);
    void updateLogicOp(GraphicsPipelineTransitionBits *transition, VkLogicOp logicOp);

    // Depth/stencil states.
    void setDepthTestEnabled(bool enabled);
    void setDepthWriteEnabled(bool enabled);
    void setDepthFunc(VkCompareOp op);
    void setDepthClampEnabled(bool enabled);
    void setStencilTestEnabled(bool enabled);
    void setStencilFrontFuncs(VkCompareOp compareOp);
    void setStencilBackFuncs(VkCompareOp compareOp);
    void setStencilFrontOps(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp);
    void setStencilBackOps(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp);
    void setStencilFrontWriteMask(uint8_t mask);
    void setStencilBackWriteMask(uint8_t mask);
    void updateDepthTestEnabled(GraphicsPipelineTransitionBits *transition,
                                const gl::DepthStencilState &depthStencilState,
                                const gl::Framebuffer *drawFramebuffer);
    void updateDepthFunc(GraphicsPipelineTransitionBits *transition,
                         const gl::DepthStencilState &depthStencilState);
    void updateDepthClampEnabled(GraphicsPipelineTransitionBits *transition, bool enabled);
    void updateDepthWriteEnabled(GraphicsPipelineTransitionBits *transition,
                                 const gl::DepthStencilState &depthStencilState,
                                 const gl::Framebuffer *drawFramebuffer);
    void updateStencilTestEnabled(GraphicsPipelineTransitionBits *transition,
                                  const gl::DepthStencilState &depthStencilState,
                                  const gl::Framebuffer *drawFramebuffer);
    void updateStencilFrontFuncs(GraphicsPipelineTransitionBits *transition,
                                 const gl::DepthStencilState &depthStencilState);
    void updateStencilBackFuncs(GraphicsPipelineTransitionBits *transition,
                                const gl::DepthStencilState &depthStencilState);
    void updateStencilFrontOps(GraphicsPipelineTransitionBits *transition,
                               const gl::DepthStencilState &depthStencilState);
    void updateStencilBackOps(GraphicsPipelineTransitionBits *transition,
                              const gl::DepthStencilState &depthStencilState);

    // Depth offset.
    void updatePolygonOffsetEnabled(GraphicsPipelineTransitionBits *transition, bool enabled);

    // Tessellation
    void updatePatchVertices(GraphicsPipelineTransitionBits *transition, GLuint value);

    // Subpass
    void resetSubpass(GraphicsPipelineTransitionBits *transition);
    void nextSubpass(GraphicsPipelineTransitionBits *transition);
    void setSubpass(uint32_t subpass);
    uint32_t getSubpass() const;

    void updateSurfaceRotation(GraphicsPipelineTransitionBits *transition,
                               bool isRotatedAspectRatio);
    bool getSurfaceRotation() const { return mShaders.shaders.bits.surfaceRotation; }

    void updateEmulatedDitherControl(GraphicsPipelineTransitionBits *transition, uint16_t value);
    uint32_t getEmulatedDitherControl() const { return mShaders.shaders.emulatedDitherControl; }

    bool isLegacyDitherEnabled() const
    {
        return mSharedNonVertexInput.renderPass.isLegacyDitherEnabled();
    }

    void updateNonZeroStencilWriteMaskWorkaround(GraphicsPipelineTransitionBits *transition,
                                                 bool enabled);

    void setSupportsDynamicStateForTest(bool supports)
    {
        mVertexInput.inputAssembly.bits.useVertexInputBindingStrideDynamicState = supports;
        mShaders.shaders.bits.nonZeroStencilWriteMaskWorkaround                 = false;
    }

    static VkFormat getPipelineVertexInputStateFormat(ErrorContext *context,
                                                      angle::FormatID formatID,
                                                      bool compressed,
                                                      const gl::ComponentType programAttribType,
                                                      uint32_t attribIndex);

    // Helpers to dump the state
    const PipelineVertexInputState &getVertexInputStateForLog() const { return mVertexInput; }
    const PipelineShadersState &getShadersStateForLog() const { return mShaders; }
    const PipelineSharedNonVertexInputState &getSharedNonVertexInputStateForLog() const
    {
        return mSharedNonVertexInput;
    }
    const PipelineFragmentOutputState &getFragmentOutputStateForLog() const
    {
        return mFragmentOutput;
    }

  private:
    void updateSubpass(GraphicsPipelineTransitionBits *transition, uint32_t subpass);

    const void *getPipelineSubsetMemory(GraphicsPipelineSubset subset, size_t *sizeOut) const;

    void initializePipelineVertexInputState(
        ErrorContext *context,
        GraphicsPipelineVertexInputVulkanStructs *stateOut,
        GraphicsPipelineDynamicStateList *dynamicStateListOut) const;

    void initializePipelineShadersState(
        ErrorContext *context,
        const ShaderModuleMap &shaders,
        const SpecializationConstants &specConsts,
        GraphicsPipelineShadersVulkanStructs *stateOut,
        GraphicsPipelineDynamicStateList *dynamicStateListOut) const;

    void initializePipelineSharedNonVertexInputState(
        ErrorContext *context,
        GraphicsPipelineSharedNonVertexInputVulkanStructs *stateOut,
        GraphicsPipelineDynamicStateList *dynamicStateListOut) const;

    void initializePipelineFragmentOutputState(
        ErrorContext *context,
        GraphicsPipelineFragmentOutputVulkanStructs *stateOut,
        GraphicsPipelineDynamicStateList *dynamicStateListOut) const;

    PipelineShadersState mShaders;
    PipelineSharedNonVertexInputState mSharedNonVertexInput;
    PipelineFragmentOutputState mFragmentOutput;
    PipelineVertexInputState mVertexInput;
};

// Verify the packed pipeline description has no gaps in the packing.
// This is not guaranteed by the spec, but is validated by a compile-time check.
// No gaps or padding at the end ensures that hashing and memcmp checks will not run
// into uninitialized memory regions.
constexpr size_t kGraphicsPipelineDescSize = sizeof(GraphicsPipelineDesc);
static_assert(kGraphicsPipelineDescSize == kGraphicsPipelineDescSumOfSizes, "Size mismatch");

// Values are based on data recorded here -> https://anglebug.com/42267114#comment5
constexpr size_t kDefaultDescriptorSetLayoutBindingsCount = 8;
constexpr size_t kDefaultImmutableSamplerBindingsCount    = 1;
using DescriptorSetLayoutBindingVector =
    angle::FastVector<VkDescriptorSetLayoutBinding, kDefaultDescriptorSetLayoutBindingsCount>;

// A packed description of a descriptor set layout. Use similarly to RenderPassDesc and
// GraphicsPipelineDesc. Currently we only need to differentiate layouts based on sampler and ubo
// usage. In the future we could generalize this.
class DescriptorSetLayoutDesc final
{
  public:
    DescriptorSetLayoutDesc();
    ~DescriptorSetLayoutDesc();
    DescriptorSetLayoutDesc(const DescriptorSetLayoutDesc &other);
    DescriptorSetLayoutDesc &operator=(const DescriptorSetLayoutDesc &other);

    size_t hash() const;
    bool operator==(const DescriptorSetLayoutDesc &other) const;

    void addBinding(uint32_t bindingIndex,
                    VkDescriptorType descriptorType,
                    uint32_t count,
                    VkShaderStageFlags stages,
                    const Sampler *immutableSampler);

    void unpackBindings(DescriptorSetLayoutBindingVector *bindings) const;

    bool empty() const { return mDescriptorSetLayoutBindings.empty(); }

  private:
    // There is a small risk of an issue if the sampler cache is evicted but not the descriptor
    // cache we would have an invalid handle here. Thus propose follow-up work:
    // TODO: https://issuetracker.google.com/issues/159156775: Have immutable sampler use serial
    union PackedDescriptorSetBinding
    {
        static constexpr uint8_t kInvalidType = 255;

        struct
        {
            uint8_t type;                      // Stores a packed VkDescriptorType descriptorType.
            uint8_t stages;                    // Stores a packed VkShaderStageFlags.
            uint16_t count : 15;               // Stores a packed uint32_t descriptorCount
            uint16_t hasImmutableSampler : 1;  // Whether this binding has an immutable sampler
        };
        uint32_t value;

        bool operator==(const PackedDescriptorSetBinding &other) const
        {
            return value == other.value;
        }
    };

    // 1x 32bit
    static_assert(sizeof(PackedDescriptorSetBinding) == 4, "Unexpected size");

    angle::FastVector<VkSampler, kDefaultImmutableSamplerBindingsCount> mImmutableSamplers;
    angle::FastVector<PackedDescriptorSetBinding, kDefaultDescriptorSetLayoutBindingsCount>
        mDescriptorSetLayoutBindings;

#if !defined(ANGLE_IS_64_BIT_CPU)
    ANGLE_MAYBE_UNUSED_PRIVATE_FIELD uint32_t mPadding = 0;
#endif
};

// The following are for caching descriptor set layouts. Limited to max three descriptor set
// layouts. This can be extended in the future.
constexpr size_t kMaxDescriptorSetLayouts = ToUnderlying(DescriptorSetIndex::EnumCount);

union PackedPushConstantRange
{
    struct
    {
        uint8_t offset;
        uint8_t size;
        uint16_t stageMask;
    };
    uint32_t value;

    bool operator==(const PackedPushConstantRange &other) const { return value == other.value; }
};

static_assert(sizeof(PackedPushConstantRange) == sizeof(uint32_t), "Unexpected Size");

template <typename T>
using DescriptorSetArray = angle::PackedEnumMap<DescriptorSetIndex, T>;
using DescriptorSetLayoutPointerArray = DescriptorSetArray<DescriptorSetLayoutPtr>;

class PipelineLayoutDesc final
{
  public:
    PipelineLayoutDesc();
    ~PipelineLayoutDesc();
    PipelineLayoutDesc(const PipelineLayoutDesc &other);
    PipelineLayoutDesc &operator=(const PipelineLayoutDesc &rhs);

    size_t hash() const;
    bool operator==(const PipelineLayoutDesc &other) const;

    void updateDescriptorSetLayout(DescriptorSetIndex setIndex,
                                   const DescriptorSetLayoutDesc &desc);
    void updatePushConstantRange(VkShaderStageFlags stageMask, uint32_t offset, uint32_t size);

    const PackedPushConstantRange &getPushConstantRange() const { return mPushConstantRange; }

  private:
    DescriptorSetArray<DescriptorSetLayoutDesc> mDescriptorSetLayouts;
    PackedPushConstantRange mPushConstantRange;
    ANGLE_MAYBE_UNUSED_PRIVATE_FIELD uint32_t mPadding;

    // Verify the arrays are properly packed.
    static_assert(sizeof(decltype(mDescriptorSetLayouts)) ==
                      (sizeof(DescriptorSetLayoutDesc) * kMaxDescriptorSetLayouts),
                  "Unexpected size");
};

// Verify the structure is properly packed.
static_assert(sizeof(PipelineLayoutDesc) == sizeof(DescriptorSetArray<DescriptorSetLayoutDesc>) +
                                                sizeof(PackedPushConstantRange) + sizeof(uint32_t),
              "Unexpected Size");

enum class YcbcrLinearFilterSupport
{
    Unsupported,
    Supported,
};

class YcbcrConversionDesc final
{
  public:
    YcbcrConversionDesc();
    ~YcbcrConversionDesc();
    YcbcrConversionDesc(const YcbcrConversionDesc &other);
    YcbcrConversionDesc &operator=(const YcbcrConversionDesc &other);

    size_t hash() const;
    bool operator==(const YcbcrConversionDesc &other) const;

    bool valid() const { return mExternalOrVkFormat != 0; }
    void reset();
    void update(Renderer *renderer,
                uint64_t externalFormat,
                VkSamplerYcbcrModelConversion conversionModel,
                VkSamplerYcbcrRange colorRange,
                VkChromaLocation xChromaOffset,
                VkChromaLocation yChromaOffset,
                VkFilter chromaFilter,
                VkComponentMapping components,
                angle::FormatID intendedFormatID,
                YcbcrLinearFilterSupport linearFilterSupported);
    VkFilter getChromaFilter() const { return static_cast<VkFilter>(mChromaFilter); }
    bool updateChromaFilter(Renderer *renderer, VkFilter filter);
    void updateConversionModel(VkSamplerYcbcrModelConversion conversionModel);
    uint64_t getExternalFormat() const { return mIsExternalFormat ? mExternalOrVkFormat : 0; }

    angle::Result init(ErrorContext *context, SamplerYcbcrConversion *conversionOut) const;

  private:
    // If the sampler needs to convert the image content (e.g. from YUV to RGB) then
    // mExternalOrVkFormat will be non-zero. The value is either the external format
    // as returned by vkGetAndroidHardwareBufferPropertiesANDROID or a YUV VkFormat.
    // For VkSamplerYcbcrConversion, mExternalOrVkFormat along with mIsExternalFormat,
    // mConversionModel and mColorRange works as a Serial() used elsewhere in ANGLE.
    uint64_t mExternalOrVkFormat;
    // 1 bit to identify if external format is used
    uint32_t mIsExternalFormat : 1;
    // 3 bits to identify conversion model
    uint32_t mConversionModel : 3;
    // 1 bit to identify color component range
    uint32_t mColorRange : 1;
    // 1 bit to identify x chroma location
    uint32_t mXChromaOffset : 1;
    // 1 bit to identify y chroma location
    uint32_t mYChromaOffset : 1;
    // 1 bit to identify chroma filtering
    uint32_t mChromaFilter : 1;
    // 3 bit to identify R component swizzle
    uint32_t mRSwizzle : 3;
    // 3 bit to identify G component swizzle
    uint32_t mGSwizzle : 3;
    // 3 bit to identify B component swizzle
    uint32_t mBSwizzle : 3;
    // 3 bit to identify A component swizzle
    uint32_t mASwizzle : 3;
    // 1 bit for whether linear filtering is supported (independent of whether currently enabled)
    uint32_t mLinearFilterSupported : 1;
    uint32_t mPadding : 11;
    uint32_t mReserved;
};

static_assert(sizeof(YcbcrConversionDesc) == 16, "Unexpected YcbcrConversionDesc size");

// Packed sampler description for the sampler cache.
class SamplerDesc final
{
  public:
    SamplerDesc();
    SamplerDesc(ErrorContext *context,
                const gl::SamplerState &samplerState,
                bool stencilMode,
                const YcbcrConversionDesc *ycbcrConversionDesc,
                angle::FormatID intendedFormatID);
    ~SamplerDesc();

    SamplerDesc(const SamplerDesc &other);
    SamplerDesc &operator=(const SamplerDesc &rhs);

    void update(Renderer *renderer,
                const gl::SamplerState &samplerState,
                bool stencilMode,
                const YcbcrConversionDesc *ycbcrConversionDesc,
                angle::FormatID intendedFormatID);
    void reset();
    angle::Result init(ContextVk *contextVk, Sampler *sampler) const;

    size_t hash() const;
    bool operator==(const SamplerDesc &other) const;

  private:
    // 32*4 bits for floating point data.
    // Note: anisotropy enabled is implicitly determined by maxAnisotropy and caps.
    float mMipLodBias;
    float mMaxAnisotropy;
    float mMinLod;
    float mMaxLod;

    // 16*8 bits to uniquely identify a YCbCr conversion sampler.
    YcbcrConversionDesc mYcbcrConversionDesc;

    // 16 bits for modes + states.
    // 1 bit per filter (only 2 possible values in GL: linear/nearest)
    uint16_t mMagFilter : 1;
    uint16_t mMinFilter : 1;
    uint16_t mMipmapMode : 1;

    // 3 bits per address mode (5 possible values)
    uint16_t mAddressModeU : 3;
    uint16_t mAddressModeV : 3;
    uint16_t mAddressModeW : 3;

    // 1 bit for compare enabled (2 possible values)
    uint16_t mCompareEnabled : 1;

    // 3 bits for compare op. (8 possible values)
    uint16_t mCompareOp : 3;

    // Values from angle::ColorGeneric::Type. Float is 0 and others are 1.
    uint16_t mBorderColorType : 1;

    uint16_t mPadding : 15;

    // 16*8 bits for BorderColor
    angle::ColorF mBorderColor;

    // 32 bits reserved for future use.
    uint32_t mReserved;
};

static_assert(sizeof(SamplerDesc) == 56, "Unexpected SamplerDesc size");

// Disable warnings about struct padding.
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

class PipelineHelper;

struct GraphicsPipelineTransition
{
    GraphicsPipelineTransition();
    GraphicsPipelineTransition(const GraphicsPipelineTransition &other);
    GraphicsPipelineTransition(GraphicsPipelineTransitionBits bits,
                               const GraphicsPipelineDesc *desc,
                               PipelineHelper *pipeline);

    GraphicsPipelineTransitionBits bits;
    const GraphicsPipelineDesc *desc;
    PipelineHelper *target;
};

ANGLE_INLINE GraphicsPipelineTransition::GraphicsPipelineTransition() = default;

ANGLE_INLINE GraphicsPipelineTransition::GraphicsPipelineTransition(
    const GraphicsPipelineTransition &other) = default;

ANGLE_INLINE GraphicsPipelineTransition::GraphicsPipelineTransition(
    GraphicsPipelineTransitionBits bits,
    const GraphicsPipelineDesc *desc,
    PipelineHelper *pipeline)
    : bits(bits), desc(desc), target(pipeline)
{}

ANGLE_INLINE bool GraphicsPipelineTransitionMatch(GraphicsPipelineTransitionBits bitsA,
                                                  GraphicsPipelineTransitionBits bitsB,
                                                  const GraphicsPipelineDesc &descA,
                                                  const GraphicsPipelineDesc &descB)
{
    if (bitsA != bitsB)
        return false;

    // We currently mask over 4 bytes of the pipeline description with each dirty bit.
    // We could consider using 8 bytes and a mask of 32 bits. This would make some parts
    // of the code faster. The for loop below would scan over twice as many bits per iteration.
    // But there may be more collisions between the same dirty bit masks leading to different
    // transitions. Thus there may be additional cost when applications use many transitions.
    // We should revisit this in the future and investigate using different bit widths.
    static_assert(sizeof(uint32_t) == kGraphicsPipelineDirtyBitBytes, "Size mismatch");

    const uint32_t *rawPtrA = descA.getPtr<uint32_t>();
    const uint32_t *rawPtrB = descB.getPtr<uint32_t>();

    for (size_t dirtyBit : bitsA)
    {
        if (rawPtrA[dirtyBit] != rawPtrB[dirtyBit])
            return false;
    }

    return true;
}

// A class that encapsulates the vk::PipelineCache and associated mutex.  The mutex may be nullptr
// if synchronization is not necessary.
class PipelineCacheAccess
{
  public:
    PipelineCacheAccess()  = default;
    ~PipelineCacheAccess() = default;

    void init(const vk::PipelineCache *pipelineCache, angle::SimpleMutex *mutex)
    {
        mPipelineCache = pipelineCache;
        mMutex         = mutex;
    }

    VkResult createGraphicsPipeline(vk::ErrorContext *context,
                                    const VkGraphicsPipelineCreateInfo &createInfo,
                                    vk::Pipeline *pipelineOut);
    VkResult createComputePipeline(vk::ErrorContext *context,
                                   const VkComputePipelineCreateInfo &createInfo,
                                   vk::Pipeline *pipelineOut);

    VkResult getCacheData(vk::ErrorContext *context, size_t *cacheSize, void *cacheData);

    void merge(Renderer *renderer, const vk::PipelineCache &pipelineCache);

    bool isThreadSafe() const { return mMutex != nullptr; }

  private:
    std::unique_lock<angle::SimpleMutex> getLock();

    const vk::PipelineCache *mPipelineCache = nullptr;
    angle::SimpleMutex *mMutex;
};

// Monolithic pipeline creation tasks are created as soon as a pipeline is created out of libraries.
// However, they are not immediately posted to the worker queue to allow pacing.  On each use of a
// pipeline, an attempt is made to post the task.
class CreateMonolithicPipelineTask : public ErrorContext, public angle::Closure
{
  public:
    CreateMonolithicPipelineTask(Renderer *renderer,
                                 const PipelineCacheAccess &pipelineCache,
                                 const PipelineLayout &pipelineLayout,
                                 const ShaderModuleMap &shaders,
                                 const SpecializationConstants &specConsts,
                                 const GraphicsPipelineDesc &desc);

    // The compatible render pass is set only when the task is ready to run.  This is because the
    // render pass cache may have been cleared since the task was created (e.g. to accomodate
    // framebuffer fetch).  Such render pass cache clears ensure there are no active tasks, so it's
    // safe to hold on to this pointer for the brief period between task post and completion.
    //
    // Not applicable to dynamic rendering.
    const RenderPassDesc &getRenderPassDesc() const { return mDesc.getRenderPassDesc(); }
    void setCompatibleRenderPass(const RenderPass *compatibleRenderPass);

    void operator()() override;

    VkResult getResult() const { return mResult; }
    Pipeline &getPipeline() { return mPipeline; }
    CacheLookUpFeedback getFeedback() const { return mFeedback; }

    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

  private:
    // Input to pipeline creation
    PipelineCacheAccess mPipelineCache;
    const RenderPass *mCompatibleRenderPass;
    const PipelineLayout &mPipelineLayout;
    const ShaderModuleMap &mShaders;
    SpecializationConstants mSpecConsts;
    GraphicsPipelineDesc mDesc;

    // Results
    VkResult mResult;
    Pipeline mPipeline;
    CacheLookUpFeedback mFeedback;
};

class WaitableMonolithicPipelineCreationTask
{
  public:
    ~WaitableMonolithicPipelineCreationTask();

    void setTask(std::shared_ptr<CreateMonolithicPipelineTask> &&task) { mTask = std::move(task); }
    void setRenderPass(const RenderPass *compatibleRenderPass)
    {
        mTask->setCompatibleRenderPass(compatibleRenderPass);
    }
    void onSchedule(const std::shared_ptr<angle::WaitableEvent> &waitableEvent)
    {
        mWaitableEvent = waitableEvent;
    }
    void reset()
    {
        mWaitableEvent.reset();
        mTask.reset();
    }

    bool isValid() const { return mTask.get() != nullptr; }
    bool isPosted() const { return mWaitableEvent.get() != nullptr; }
    bool isReady() { return mWaitableEvent->isReady(); }
    void wait() { return mWaitableEvent->wait(); }

    std::shared_ptr<CreateMonolithicPipelineTask> getTask() const { return mTask; }

  private:
    std::shared_ptr<angle::WaitableEvent> mWaitableEvent;
    std::shared_ptr<CreateMonolithicPipelineTask> mTask;
};

class PipelineHelper final : public Resource
{
  public:
    PipelineHelper();
    ~PipelineHelper() override;
    inline explicit PipelineHelper(Pipeline &&pipeline, CacheLookUpFeedback feedback);
    PipelineHelper &operator=(PipelineHelper &&other);

    void destroy(VkDevice device);
    void release(ErrorContext *context);

    bool valid() const { return mPipeline.valid(); }
    const Pipeline &getPipeline() const { return mPipeline; }

    // Get the pipeline.  If there is a monolithic pipeline creation task pending, scheduling it is
    // attempted.  If that task is done, the pipeline is replaced with the results and the old
    // pipeline released.
    angle::Result getPreferredPipeline(ContextVk *contextVk, const Pipeline **pipelineOut);

    ANGLE_INLINE bool findTransition(GraphicsPipelineTransitionBits bits,
                                     const GraphicsPipelineDesc &desc,
                                     PipelineHelper **pipelineOut) const
    {
        // Search could be improved using sorting or hashing.
        for (const GraphicsPipelineTransition &transition : mTransitions)
        {
            if (GraphicsPipelineTransitionMatch(transition.bits, bits, *transition.desc, desc))
            {
                *pipelineOut = transition.target;
                return true;
            }
        }

        return false;
    }

    void addTransition(GraphicsPipelineTransitionBits bits,
                       const GraphicsPipelineDesc *desc,
                       PipelineHelper *pipeline);

    const std::vector<GraphicsPipelineTransition> getTransitions() const { return mTransitions; }

    void setComputePipeline(Pipeline &&pipeline, CacheLookUpFeedback feedback)
    {
        ASSERT(!mPipeline.valid());
        mPipeline = std::move(pipeline);

        ASSERT(mCacheLookUpFeedback == CacheLookUpFeedback::None);
        mCacheLookUpFeedback = feedback;
    }
    CacheLookUpFeedback getCacheLookUpFeedback() const { return mCacheLookUpFeedback; }

    void setLinkedLibraryReferences(vk::PipelineHelper *shadersPipeline);

    void retainInRenderPass(RenderPassCommandBufferHelper *renderPassCommands);

    void setMonolithicPipelineCreationTask(std::shared_ptr<CreateMonolithicPipelineTask> &&task)
    {
        mMonolithicPipelineCreationTask.setTask(std::move(task));
    }

  private:
    void reset();

    std::vector<GraphicsPipelineTransition> mTransitions;
    Pipeline mPipeline;
    CacheLookUpFeedback mCacheLookUpFeedback           = CacheLookUpFeedback::None;
    CacheLookUpFeedback mMonolithicCacheLookUpFeedback = CacheLookUpFeedback::None;

    // The list of pipeline helpers that were referenced when creating a linked pipeline.  These
    // pipelines must be kept alive, so their serial is updated at the same time as this object.
    // Not necessary for vertex input and fragment output as they stay alive until context's
    // destruction.
    PipelineHelper *mLinkedShaders = nullptr;

    // If pipeline libraries are used and monolithic pipelines are created in parallel, this is the
    // temporary library created (previously in |mPipeline|) that is now replaced by the monolithic
    // one.  It is not immediately garbage collected when replaced, because there is currently a bug
    // with that.  http://anglebug.com/42266335
    Pipeline mLinkedPipelineToRelease;

    // An async task to create a monolithic pipeline.  Only used if the pipeline was originally
    // created as a linked library.  The |getPipeline()| call will attempt to schedule this task
    // through the share group, which manages and paces these tasks.  Once the task results are
    // ready, |mPipeline| is released and replaced by the result of this task.
    WaitableMonolithicPipelineCreationTask mMonolithicPipelineCreationTask;
};

class FramebufferHelper : public Resource
{
  public:
    FramebufferHelper();
    ~FramebufferHelper() override;

    FramebufferHelper(FramebufferHelper &&other);
    FramebufferHelper &operator=(FramebufferHelper &&other);

    angle::Result init(ErrorContext *context, const VkFramebufferCreateInfo &createInfo);
    void destroy(Renderer *renderer);
    void release(ContextVk *contextVk);

    bool valid() { return mFramebuffer.valid(); }

    const Framebuffer &getFramebuffer() const
    {
        ASSERT(mFramebuffer.valid());
        return mFramebuffer;
    }

    Framebuffer &getFramebuffer()
    {
        ASSERT(mFramebuffer.valid());
        return mFramebuffer;
    }

  private:
    // Vulkan object.
    Framebuffer mFramebuffer;
};

ANGLE_INLINE PipelineHelper::PipelineHelper(Pipeline &&pipeline, CacheLookUpFeedback feedback)
    : mPipeline(std::move(pipeline)), mCacheLookUpFeedback(feedback)
{}

ANGLE_INLINE PipelineHelper &PipelineHelper::operator=(PipelineHelper &&other)
{
    ASSERT(!mPipeline.valid());

    std::swap(mPipeline, other.mPipeline);
    mCacheLookUpFeedback = other.mCacheLookUpFeedback;

    return *this;
}

struct ImageSubresourceRange
{
    // GL max is 1000 (fits in 10 bits).
    uint32_t level : 10;
    // Max 31 levels (2 ** 5 - 1). Can store levelCount-1 if we need to save another bit.
    uint32_t levelCount : 5;
    // Implementation max is 4096 (12 bits).
    uint32_t layer : 12;
    // One of vk::LayerMode values.  If 0, it means all layers.  Otherwise it's the count of layers
    // which is usually 1, except for multiview in which case it can be up to
    // gl::IMPLEMENTATION_MAX_2D_ARRAY_TEXTURE_LAYERS.
    uint32_t layerMode : 3;
    // For reads: Values are either ImageViewColorspace::Linear or ImageViewColorspace::SRGB
    uint32_t readColorspace : 1;
    // For writes: Values are either ImageViewColorspace::Linear or ImageViewColorspace::SRGB
    uint32_t writeColorspace : 1;

    static_assert(gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS < (1 << 5),
                  "Not enough bits for level count");
    static_assert(gl::IMPLEMENTATION_MAX_2D_ARRAY_TEXTURE_LAYERS <= (1 << 12),
                  "Not enough bits for layer index");
    static_assert(gl::IMPLEMENTATION_ANGLE_MULTIVIEW_MAX_VIEWS <= (1 << 3),
                  "Not enough bits for layer count");
};

static_assert(sizeof(ImageSubresourceRange) == sizeof(uint32_t), "Size mismatch");

inline bool operator==(const ImageSubresourceRange &a, const ImageSubresourceRange &b)
{
    return a.level == b.level && a.levelCount == b.levelCount && a.layer == b.layer &&
           a.layerMode == b.layerMode && a.readColorspace == b.readColorspace &&
           a.writeColorspace == b.writeColorspace;
}

constexpr ImageSubresourceRange kInvalidImageSubresourceRange = {0, 0, 0, 0, 0, 0};

struct ImageOrBufferViewSubresourceSerial
{
    ImageOrBufferViewSerial viewSerial;
    ImageSubresourceRange subresource;
};

inline bool operator==(const ImageOrBufferViewSubresourceSerial &a,
                       const ImageOrBufferViewSubresourceSerial &b)
{
    return a.viewSerial == b.viewSerial && a.subresource == b.subresource;
}

constexpr ImageOrBufferViewSubresourceSerial kInvalidImageOrBufferViewSubresourceSerial = {
    kInvalidImageOrBufferViewSerial, kInvalidImageSubresourceRange};

// Always starts with array element zero, with descriptorCount descriptors.
struct WriteDescriptorDesc
{
    uint8_t binding;              // Redundant: determined by the containing WriteDesc array.
    uint8_t descriptorCount;      // Number of array elements in this descriptor write.
    uint8_t descriptorType;       // Packed VkDescriptorType.
    uint8_t descriptorInfoIndex;  // Base index into an array of DescriptorInfoDescs.
};

static_assert(sizeof(WriteDescriptorDesc) == 4, "Size mismatch");

struct DescriptorInfoDesc
{
    uint32_t samplerOrBufferSerial;
    uint32_t imageViewSerialOrOffset;
    uint32_t imageLayoutOrRange;  // Packed VkImageLayout
    uint32_t imageSubresourceRange;
};

static_assert(sizeof(DescriptorInfoDesc) == 16, "Size mismatch");

// Generic description of a descriptor set. Used as a key when indexing descriptor set caches. The
// key storage is an angle:FixedVector. Beyond a certain fixed size we'll end up using heap memory
// to store keys. Currently we specialize the structure for three use cases: uniforms, textures,
// and other shader resources. Because of the way the specialization works we can't currently cache
// programs that use some types of resources.
static constexpr size_t kFastDescriptorSetDescLimit = 8;

struct DescriptorDescHandles
{
    VkBuffer buffer;
    VkSampler sampler;
    VkImageView imageView;
    VkBufferView bufferView;
};

class WriteDescriptorDescs
{
  public:
    void reset()
    {
        mDescs.clear();
        mDynamicDescriptorSetCount = 0;
        mCurrentInfoIndex          = 0;
    }

    void updateShaderBuffers(const ShaderInterfaceVariableInfoMap &variableInfoMap,
                             const std::vector<gl::InterfaceBlock> &blocks,
                             VkDescriptorType descriptorType);

    void updateAtomicCounters(const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers);

    void updateImages(const gl::ProgramExecutable &executable,
                      const ShaderInterfaceVariableInfoMap &variableInfoMap);

    void updateInputAttachments(const gl::ProgramExecutable &executable,
                                const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                FramebufferVk *framebufferVk);

    void updateExecutableActiveTextures(const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                        const gl::ProgramExecutable &executable);

    void updateDefaultUniform(gl::ShaderBitSet shaderTypes,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              const gl::ProgramExecutable &executable);

    void updateTransformFeedbackWrite(const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                      const gl::ProgramExecutable &executable);

    void updateDynamicDescriptorsCount();

    size_t size() const { return mDescs.size(); }
    bool empty() const { return mDescs.size() == 0; }

    const WriteDescriptorDesc &operator[](uint32_t bindingIndex) const
    {
        return mDescs[bindingIndex];
    }

    size_t getTotalDescriptorCount() const { return mCurrentInfoIndex; }
    size_t getDynamicDescriptorSetCount() const { return mDynamicDescriptorSetCount; }

  private:
    bool hasWriteDescAtIndex(uint32_t bindingIndex) const
    {
        return bindingIndex < mDescs.size() && mDescs[bindingIndex].descriptorCount > 0;
    }

    void incrementDescriptorCount(uint32_t bindingIndex, uint32_t count)
    {
        // Validate we have no subsequent writes.
        ASSERT(hasWriteDescAtIndex(bindingIndex));
        mDescs[bindingIndex].descriptorCount += count;
    }

    void updateWriteDesc(uint32_t bindingIndex,
                         VkDescriptorType descriptorType,
                         uint32_t descriptorCount);

    void updateInputAttachment(uint32_t binding,
                               ImageLayout layout,
                               RenderTargetVk *renderTargetVk);

    // After a preliminary minimum size, use heap memory.
    angle::FastMap<WriteDescriptorDesc, kFastDescriptorSetDescLimit> mDescs;
    size_t mDynamicDescriptorSetCount = 0;
    uint32_t mCurrentInfoIndex        = 0;
};
std::ostream &operator<<(std::ostream &os, const WriteDescriptorDescs &desc);

class DescriptorSetDesc
{
  public:
    DescriptorSetDesc()  = default;
    ~DescriptorSetDesc() = default;

    DescriptorSetDesc(const DescriptorSetDesc &other) : mDescriptorInfos(other.mDescriptorInfos) {}

    DescriptorSetDesc &operator=(const DescriptorSetDesc &other)
    {
        mDescriptorInfos = other.mDescriptorInfos;
        return *this;
    }

    size_t hash() const;

    size_t size() const { return mDescriptorInfos.size(); }
    void resize(size_t count) { mDescriptorInfos.resize(count); }

    size_t getKeySizeBytes() const { return mDescriptorInfos.size() * sizeof(DescriptorInfoDesc); }

    bool operator==(const DescriptorSetDesc &other) const
    {
        return mDescriptorInfos.size() == other.mDescriptorInfos.size() &&
               memcmp(mDescriptorInfos.data(), other.mDescriptorInfos.data(),
                      mDescriptorInfos.size() * sizeof(DescriptorInfoDesc)) == 0;
    }

    DescriptorInfoDesc &getInfoDesc(uint32_t infoDescIndex)
    {
        return mDescriptorInfos[infoDescIndex];
    }

    const DescriptorInfoDesc &getInfoDesc(uint32_t infoDescIndex) const
    {
        return mDescriptorInfos[infoDescIndex];
    }

    void updateDescriptorSet(Renderer *renderer,
                             const WriteDescriptorDescs &writeDescriptorDescs,
                             UpdateDescriptorSetsBuilder *updateBuilder,
                             const DescriptorDescHandles *handles,
                             VkDescriptorSet descriptorSet) const;

  private:
    // After a preliminary minimum size, use heap memory.
    angle::FastVector<DescriptorInfoDesc, kFastDescriptorSetDescLimit> mDescriptorInfos;
};
std::ostream &operator<<(std::ostream &os, const DescriptorSetDesc &desc);

class DescriptorPoolHelper;

// SharedDescriptorSetCacheKey.
// Because DescriptorSet must associate with a pool, we need to define a structure that wraps both.
class DescriptorSetDescAndPool final
{
  public:
    DescriptorSetDescAndPool() : mPool(nullptr) {}
    DescriptorSetDescAndPool(const DescriptorSetDesc &desc, DynamicDescriptorPool *pool)
        : mDesc(desc), mPool(pool)
    {}
    DescriptorSetDescAndPool(DescriptorSetDescAndPool &&other)
        : mDesc(other.mDesc), mPool(other.mPool)
    {
        other.mPool = nullptr;
    }
    ~DescriptorSetDescAndPool() { ASSERT(!valid()); }
    void destroy(VkDevice /*device*/) { mPool = nullptr; }

    void destroyCachedObject(Renderer *renderer);
    void releaseCachedObject(ContextVk *contextVk) { UNREACHABLE(); }
    void releaseCachedObject(Renderer *renderer);
    bool valid() const { return mPool != nullptr; }
    const DescriptorSetDesc &getDesc() const
    {
        ASSERT(valid());
        return mDesc;
    }
    bool operator==(const DescriptorSetDescAndPool &other) const
    {
        return mDesc == other.mDesc && mPool == other.mPool;
    }

  private:
    DescriptorSetDesc mDesc;
    DynamicDescriptorPool *mPool;
};
using SharedDescriptorSetCacheKey = SharedPtr<DescriptorSetDescAndPool>;
ANGLE_INLINE const SharedDescriptorSetCacheKey
CreateSharedDescriptorSetCacheKey(const DescriptorSetDesc &desc, DynamicDescriptorPool *pool)
{
    return SharedDescriptorSetCacheKey::MakeShared(VK_NULL_HANDLE, desc, pool);
}

constexpr VkDescriptorType kStorageBufferDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

// Manages a descriptor set desc with a few helper routines and also stores object handles.
class DescriptorSetDescBuilder final
{
  public:
    DescriptorSetDescBuilder();
    DescriptorSetDescBuilder(size_t descriptorCount);
    ~DescriptorSetDescBuilder();

    DescriptorSetDescBuilder(const DescriptorSetDescBuilder &other);
    DescriptorSetDescBuilder &operator=(const DescriptorSetDescBuilder &other);

    const DescriptorSetDesc &getDesc() const { return mDesc; }

    void resize(size_t descriptorCount)
    {
        mDesc.resize(descriptorCount);
        mHandles.resize(descriptorCount);
        mDynamicOffsets.resize(descriptorCount);
    }

    // Specific helpers for uniforms/xfb descriptors.
    void updateUniformBuffer(uint32_t shaderIndex,
                             const WriteDescriptorDescs &writeDescriptorDescs,
                             const BufferHelper &bufferHelper,
                             VkDeviceSize bufferRange);

    void updateTransformFeedbackBuffer(const Context *context,
                                       const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                       const WriteDescriptorDescs &writeDescriptorDescs,
                                       uint32_t xfbBufferIndex,
                                       const BufferHelper &bufferHelper,
                                       VkDeviceSize bufferOffset,
                                       VkDeviceSize bufferRange);

    void updateUniformsAndXfb(Context *context,
                              const gl::ProgramExecutable &executable,
                              const WriteDescriptorDescs &writeDescriptorDescs,
                              const BufferHelper *currentUniformBuffer,
                              const BufferHelper &emptyBuffer,
                              bool activeUnpaused,
                              TransformFeedbackVk *transformFeedbackVk);

    // Specific helpers for shader resource descriptors.
    template <typename CommandBufferT>
    void updateOneShaderBuffer(Context *context,
                               CommandBufferT *commandBufferHelper,
                               const ShaderInterfaceVariableInfoMap &variableInfoMap,
                               const gl::BufferVector &buffers,
                               const gl::InterfaceBlock &block,
                               uint32_t bufferIndex,
                               VkDescriptorType descriptorType,
                               VkDeviceSize maxBoundBufferRange,
                               const BufferHelper &emptyBuffer,
                               const WriteDescriptorDescs &writeDescriptorDescs,
                               const GLbitfield memoryBarrierBits);
    template <typename CommandBufferT>
    void updateShaderBuffers(Context *context,
                             CommandBufferT *commandBufferHelper,
                             const gl::ProgramExecutable &executable,
                             const ShaderInterfaceVariableInfoMap &variableInfoMap,
                             const gl::BufferVector &buffers,
                             const std::vector<gl::InterfaceBlock> &blocks,
                             VkDescriptorType descriptorType,
                             VkDeviceSize maxBoundBufferRange,
                             const BufferHelper &emptyBuffer,
                             const WriteDescriptorDescs &writeDescriptorDescs,
                             const GLbitfield memoryBarrierBits);
    template <typename CommandBufferT>
    void updateAtomicCounters(Context *context,
                              CommandBufferT *commandBufferHelper,
                              const gl::ProgramExecutable &executable,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              const gl::BufferVector &buffers,
                              const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
                              const VkDeviceSize requiredOffsetAlignment,
                              const BufferHelper &emptyBuffer,
                              const WriteDescriptorDescs &writeDescriptorDescs);
    angle::Result updateImages(Context *context,
                               const gl::ProgramExecutable &executable,
                               const ShaderInterfaceVariableInfoMap &variableInfoMap,
                               const gl::ActiveTextureArray<TextureVk *> &activeImages,
                               const std::vector<gl::ImageUnit> &imageUnits,
                               const WriteDescriptorDescs &writeDescriptorDescs);
    angle::Result updateInputAttachments(vk::Context *context,
                                         const gl::ProgramExecutable &executable,
                                         const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                         FramebufferVk *framebufferVk,
                                         const WriteDescriptorDescs &writeDescriptorDescs);

    // Specialized update for textures.
    void updatePreCacheActiveTextures(Context *context,
                                      const gl::ProgramExecutable &executable,
                                      const gl::ActiveTextureArray<TextureVk *> &textures,
                                      const gl::SamplerBindingVector &samplers);

    void updateDescriptorSet(Renderer *renderer,
                             const WriteDescriptorDescs &writeDescriptorDescs,
                             UpdateDescriptorSetsBuilder *updateBuilder,
                             VkDescriptorSet descriptorSet) const;

    const uint32_t *getDynamicOffsets() const { return mDynamicOffsets.data(); }
    size_t getDynamicOffsetsSize() const { return mDynamicOffsets.size(); }

  private:
    void updateInputAttachment(Context *context,
                               uint32_t binding,
                               ImageLayout layout,
                               const vk::ImageView *imageView,
                               ImageOrBufferViewSubresourceSerial serial,
                               const WriteDescriptorDescs &writeDescriptorDescs);

    void setEmptyBuffer(uint32_t infoDescIndex,
                        VkDescriptorType descriptorType,
                        const BufferHelper &emptyBuffer);

    DescriptorSetDesc mDesc;
    angle::FastVector<DescriptorDescHandles, kFastDescriptorSetDescLimit> mHandles;
    angle::FastVector<uint32_t, kFastDescriptorSetDescLimit> mDynamicOffsets;
};

// In the FramebufferDesc object:
//  - Depth/stencil serial is at index 0
//  - Color serials are at indices [1, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS]
//  - Depth/stencil resolve attachment is at index gl::IMPLEMENTATION_MAX_DRAW_BUFFERS+1
//  - Resolve attachments are at indices [gl::IMPLEMENTATION_MAX_DRAW_BUFFERS+2,
//                                        gl::IMPLEMENTATION_MAX_DRAW_BUFFERS*2+1]
//    Fragment shading rate attachment serial is at index
//    (gl::IMPLEMENTATION_MAX_DRAW_BUFFERS*2+1)+1
constexpr size_t kFramebufferDescDepthStencilIndex = 0;
constexpr size_t kFramebufferDescColorIndexOffset  = kFramebufferDescDepthStencilIndex + 1;
constexpr size_t kFramebufferDescDepthStencilResolveIndexOffset =
    kFramebufferDescColorIndexOffset + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
constexpr size_t kFramebufferDescColorResolveIndexOffset =
    kFramebufferDescDepthStencilResolveIndexOffset + 1;
constexpr size_t kFramebufferDescFragmentShadingRateAttachmentIndexOffset =
    kFramebufferDescColorResolveIndexOffset + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;

// Enable struct padding warnings for the code below since it is used in caches.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

class FramebufferDesc
{
  public:
    FramebufferDesc();
    ~FramebufferDesc();

    FramebufferDesc(const FramebufferDesc &other);
    FramebufferDesc &operator=(const FramebufferDesc &other);

    void updateColor(uint32_t index, ImageOrBufferViewSubresourceSerial serial);
    void updateColorResolve(uint32_t index, ImageOrBufferViewSubresourceSerial serial);
    void updateUnresolveMask(FramebufferNonResolveAttachmentMask unresolveMask);
    void updateDepthStencil(ImageOrBufferViewSubresourceSerial serial);
    void updateDepthStencilResolve(ImageOrBufferViewSubresourceSerial serial);
    ANGLE_INLINE void setWriteControlMode(gl::SrgbWriteControlMode mode)
    {
        mSrgbWriteControlMode = static_cast<uint16_t>(mode);
    }
    void updateIsMultiview(bool isMultiview) { mIsMultiview = isMultiview; }
    size_t hash() const;

    bool operator==(const FramebufferDesc &other) const;

    uint32_t attachmentCount() const;

    ImageOrBufferViewSubresourceSerial getColorImageViewSerial(uint32_t index)
    {
        ASSERT(kFramebufferDescColorIndexOffset + index < mSerials.size());
        return mSerials[kFramebufferDescColorIndexOffset + index];
    }

    FramebufferNonResolveAttachmentMask getUnresolveAttachmentMask() const;
    ANGLE_INLINE gl::SrgbWriteControlMode getWriteControlMode() const
    {
        return (mSrgbWriteControlMode == 1) ? gl::SrgbWriteControlMode::Linear
                                            : gl::SrgbWriteControlMode::Default;
    }

    void updateLayerCount(uint32_t layerCount);
    uint32_t getLayerCount() const { return mLayerCount; }
    void setColorFramebufferFetchMode(bool hasColorFramebufferFetch);
    bool hasColorFramebufferFetch() const { return mHasColorFramebufferFetch; }

    bool isMultiview() const { return mIsMultiview; }

    void updateRenderToTexture(bool isRenderToTexture);

    void updateFragmentShadingRate(ImageOrBufferViewSubresourceSerial serial);
    bool hasFragmentShadingRateAttachment() const;

    // Used by SharedFramebufferCacheKey
    void destroy(VkDevice /*device*/) { SetBitField(mIsValid, 0); }
    void destroyCachedObject(Renderer *renderer);
    void releaseCachedObject(Renderer *renderer) { UNREACHABLE(); }
    void releaseCachedObject(ContextVk *contextVk);
    bool valid() const { return mIsValid; }

  private:
    void reset();
    void update(uint32_t index, ImageOrBufferViewSubresourceSerial serial);

    // Note: this is an exclusive index. If there is one index it will be "1".
    // Maximum value is 18
    uint16_t mMaxIndex : 5;

    // Whether the render pass has input attachments or not.
    // Note that depth/stencil framebuffer fetch is only implemented for dynamic rendering, and so
    // does not interact with this class.
    uint16_t mHasColorFramebufferFetch : 1;
    static_assert(gl::IMPLEMENTATION_MAX_FRAMEBUFFER_LAYERS < (1 << 9) - 1,
                  "Not enough bits for mLayerCount");

    uint16_t mLayerCount : 9;

    uint16_t mSrgbWriteControlMode : 1;

    // If the render pass contains an initial subpass to unresolve a number of attachments, the
    // subpass description is derived from the following mask, specifying which attachments need
    // to be unresolved.  Includes both color and depth/stencil attachments.
    uint16_t mUnresolveAttachmentMask : kMaxFramebufferNonResolveAttachments;

    // Whether this is a multisampled-render-to-single-sampled framebuffer.  Only used when using
    // VK_EXT_multisampled_render_to_single_sampled.  Only one bit is used and the rest is padding.
    uint16_t mIsRenderToTexture : 14 - kMaxFramebufferNonResolveAttachments;

    uint16_t mIsMultiview : 1;
    // Used by SharedFramebufferCacheKey to indicate if this cache key is valid or not.
    uint16_t mIsValid : 1;

    FramebufferAttachmentArray<ImageOrBufferViewSubresourceSerial> mSerials;
};

constexpr size_t kFramebufferDescSize = sizeof(FramebufferDesc);
static_assert(kFramebufferDescSize == 156, "Size check failed");

// Disable warnings about struct padding.
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

using SharedFramebufferCacheKey = SharedPtr<FramebufferDesc>;
ANGLE_INLINE const SharedFramebufferCacheKey
CreateSharedFramebufferCacheKey(const FramebufferDesc &desc)
{
    return SharedFramebufferCacheKey::MakeShared(VK_NULL_HANDLE, desc);
}

// The SamplerHelper allows a Sampler to be coupled with a serial.
// Must be included before we declare SamplerCache.
class SamplerHelper final : angle::NonCopyable
{
  public:
    SamplerHelper() = default;
    ~SamplerHelper() { ASSERT(!valid()); }

    explicit SamplerHelper(SamplerHelper &&samplerHelper);
    SamplerHelper &operator=(SamplerHelper &&rhs);

    angle::Result init(ErrorContext *context, const VkSamplerCreateInfo &createInfo);
    angle::Result init(ContextVk *contextVk, const SamplerDesc &desc);
    void destroy(VkDevice device) { mSampler.destroy(device); }
    void destroy() { ASSERT(!valid()); }
    bool valid() const { return mSampler.valid(); }
    const Sampler &get() const { return mSampler; }
    SamplerSerial getSamplerSerial() const { return mSamplerSerial; }

  private:
    Sampler mSampler;
    SamplerSerial mSamplerSerial;
};

using SharedSamplerPtr = SharedPtr<SamplerHelper>;

class RenderPassHelper final : angle::NonCopyable
{
  public:
    RenderPassHelper();
    ~RenderPassHelper();

    RenderPassHelper(RenderPassHelper &&other);
    RenderPassHelper &operator=(RenderPassHelper &&other);

    void destroy(VkDevice device);
    void release(ContextVk *contextVk);

    const RenderPass &getRenderPass() const;
    RenderPass &getRenderPass();

    const RenderPassPerfCounters &getPerfCounters() const;
    RenderPassPerfCounters &getPerfCounters();

  private:
    RenderPass mRenderPass;
    RenderPassPerfCounters mPerfCounters;
};

// Helper class manages the lifetime of various cache objects so that the cache entry can be
// destroyed when one of the components becomes invalid.
template <class SharedCacheKeyT>
class SharedCacheKeyManager
{
  public:
    SharedCacheKeyManager() = default;
    ~SharedCacheKeyManager() { ASSERT(empty()); }
    // Store the pointer to the cache key and retains it
    void addKey(const SharedCacheKeyT &key);
    // Iterate over the descriptor array and release the descriptor and cache.
    void releaseKeys(ContextVk *contextVk);
    void releaseKeys(Renderer *renderer);
    // Iterate over the descriptor array and destroy the descriptor and cache.
    void destroyKeys(Renderer *renderer);
    void clear();

    // The following APIs are expected to be used for assertion only
    bool containsKey(const SharedCacheKeyT &key) const;
    bool empty() const { return mSharedCacheKeys.empty(); }
    void assertAllEntriesDestroyed();

  private:
    size_t updateEmptySlotBits();

    // Tracks an array of cache keys with refcounting. Note this owns one refcount of
    // SharedCacheKeyT object.
    std::deque<SharedCacheKeyT> mSharedCacheKeys;

    // To speed up searching for available slot in the mSharedCacheKeys, we use bitset to track
    // available (i.e, empty) slot
    static constexpr size_t kInvalidSlot  = -1;
    static constexpr size_t kSlotBitCount = 64;
    using SlotBitMask                     = angle::BitSet64<kSlotBitCount>;
    std::vector<SlotBitMask> mEmptySlotBits;
};

using FramebufferCacheManager   = SharedCacheKeyManager<SharedFramebufferCacheKey>;
using DescriptorSetCacheManager = SharedCacheKeyManager<SharedDescriptorSetCacheKey>;
}  // namespace vk
}  // namespace rx

// Introduce std::hash for the above classes.
namespace std
{
template <>
struct hash<rx::vk::RenderPassDesc>
{
    size_t operator()(const rx::vk::RenderPassDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::AttachmentOpsArray>
{
    size_t operator()(const rx::vk::AttachmentOpsArray &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::DescriptorSetLayoutDesc>
{
    size_t operator()(const rx::vk::DescriptorSetLayoutDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::PipelineLayoutDesc>
{
    size_t operator()(const rx::vk::PipelineLayoutDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::ImageSubresourceRange>
{
    size_t operator()(const rx::vk::ImageSubresourceRange &key) const
    {
        return *reinterpret_cast<const uint32_t *>(&key);
    }
};

template <>
struct hash<rx::vk::DescriptorSetDesc>
{
    size_t operator()(const rx::vk::DescriptorSetDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::FramebufferDesc>
{
    size_t operator()(const rx::vk::FramebufferDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::YcbcrConversionDesc>
{
    size_t operator()(const rx::vk::YcbcrConversionDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::SamplerDesc>
{
    size_t operator()(const rx::vk::SamplerDesc &key) const { return key.hash(); }
};

// See Resource Serial types defined in vk_utils.h.
#define ANGLE_HASH_VK_SERIAL(Type)                               \
    template <>                                                  \
    struct hash<rx::vk::Type##Serial>                            \
    {                                                            \
        size_t operator()(const rx::vk::Type##Serial &key) const \
        {                                                        \
            return key.getValue();                               \
        }                                                        \
    };

ANGLE_VK_SERIAL_OP(ANGLE_HASH_VK_SERIAL)

}  // namespace std

namespace rx
{
// Cache types for various Vulkan objects
enum class VulkanCacheType
{
    CompatibleRenderPass,
    RenderPassWithOps,
    GraphicsPipeline,
    ComputePipeline,
    PipelineLayout,
    Sampler,
    SamplerYcbcrConversion,
    DescriptorSetLayout,
    DriverUniformsDescriptors,
    TextureDescriptors,
    UniformsAndXfbDescriptors,
    ShaderResourcesDescriptors,
    Framebuffer,
    DescriptorMetaCache,
    EnumCount
};

// Base class for all caches. Provides cache hit and miss counters.
class CacheStats final : angle::NonCopyable
{
  public:
    CacheStats() { reset(); }
    ~CacheStats() {}

    CacheStats(const CacheStats &rhs)
        : mHitCount(rhs.mHitCount), mMissCount(rhs.mMissCount), mSize(rhs.mSize)
    {}

    CacheStats &operator=(const CacheStats &rhs)
    {
        mHitCount  = rhs.mHitCount;
        mMissCount = rhs.mMissCount;
        mSize      = rhs.mSize;
        return *this;
    }

    ANGLE_INLINE void hit() { mHitCount++; }
    ANGLE_INLINE void miss() { mMissCount++; }
    ANGLE_INLINE void incrementSize() { mSize++; }
    ANGLE_INLINE void decrementSize() { mSize--; }
    ANGLE_INLINE void missAndIncrementSize()
    {
        mMissCount++;
        mSize++;
    }
    ANGLE_INLINE void accumulate(const CacheStats &stats)
    {
        mHitCount += stats.mHitCount;
        mMissCount += stats.mMissCount;
        mSize += stats.mSize;
    }

    uint32_t getHitCount() const { return mHitCount; }
    uint32_t getMissCount() const { return mMissCount; }

    ANGLE_INLINE double getHitRatio() const
    {
        if (mHitCount + mMissCount == 0)
        {
            return 0;
        }
        else
        {
            return static_cast<double>(mHitCount) / (mHitCount + mMissCount);
        }
    }

    ANGLE_INLINE uint32_t getSize() const { return mSize; }
    ANGLE_INLINE void setSize(uint32_t size) { mSize = size; }

    void reset()
    {
        mHitCount  = 0;
        mMissCount = 0;
        mSize      = 0;
    }

    void resetHitAndMissCount()
    {
        mHitCount  = 0;
        mMissCount = 0;
    }

    void accumulateCacheStats(VulkanCacheType cacheType, const CacheStats &cacheStats)
    {
        mHitCount += cacheStats.getHitCount();
        mMissCount += cacheStats.getMissCount();
    }

  private:
    uint32_t mHitCount;
    uint32_t mMissCount;
    uint32_t mSize;
};

template <VulkanCacheType CacheType>
class HasCacheStats : angle::NonCopyable
{
  public:
    template <typename Accumulator>
    void accumulateCacheStats(Accumulator *accum)
    {
        accum->accumulateCacheStats(CacheType, mCacheStats);
        mCacheStats.reset();
    }

    void getCacheStats(CacheStats *accum) const { accum->accumulate(mCacheStats); }

  protected:
    HasCacheStats()          = default;
    virtual ~HasCacheStats() = default;

    CacheStats mCacheStats;
};

using VulkanCacheStats = angle::PackedEnumMap<VulkanCacheType, CacheStats>;

// FramebufferVk Cache
class FramebufferCache final : angle::NonCopyable
{
  public:
    FramebufferCache() = default;
    ~FramebufferCache() { ASSERT(mPayload.empty()); }

    void destroy(vk::Renderer *renderer);

    bool get(ContextVk *contextVk, const vk::FramebufferDesc &desc, vk::Framebuffer &framebuffer);
    void insert(ContextVk *contextVk,
                const vk::FramebufferDesc &desc,
                vk::FramebufferHelper &&framebufferHelper);
    void erase(ContextVk *contextVk, const vk::FramebufferDesc &desc);

    size_t getSize() const { return mPayload.size(); }
    bool empty() const { return mPayload.empty(); }

  private:
    angle::HashMap<vk::FramebufferDesc, vk::FramebufferHelper> mPayload;
    CacheStats mCacheStats;
};

// TODO(jmadill): Add cache trimming/eviction.
class RenderPassCache final : angle::NonCopyable
{
  public:
    RenderPassCache();
    ~RenderPassCache();

    void destroy(ContextVk *contextVk);
    void clear(ContextVk *contextVk);

    ANGLE_INLINE angle::Result getCompatibleRenderPass(ContextVk *contextVk,
                                                       const vk::RenderPassDesc &desc,
                                                       const vk::RenderPass **renderPassOut)
    {
        auto outerIt = mPayload.find(desc);
        if (outerIt != mPayload.end())
        {
            InnerCache &innerCache = outerIt->second;
            ASSERT(!innerCache.empty());

            // Find the first element and return it.
            *renderPassOut = &innerCache.begin()->second.getRenderPass();
            mCompatibleRenderPassCacheStats.hit();
            return angle::Result::Continue;
        }

        mCompatibleRenderPassCacheStats.missAndIncrementSize();
        return addCompatibleRenderPass(contextVk, desc, renderPassOut);
    }

    angle::Result getRenderPassWithOps(ContextVk *contextVk,
                                       const vk::RenderPassDesc &desc,
                                       const vk::AttachmentOpsArray &attachmentOps,
                                       const vk::RenderPass **renderPassOut);

    static void InitializeOpsForCompatibleRenderPass(const vk::RenderPassDesc &desc,
                                                     vk::AttachmentOpsArray *opsOut);
    static angle::Result MakeRenderPass(vk::ErrorContext *context,
                                        const vk::RenderPassDesc &desc,
                                        const vk::AttachmentOpsArray &ops,
                                        vk::RenderPass *renderPass,
                                        vk::RenderPassPerfCounters *renderPassCounters);

  private:
    angle::Result getRenderPassWithOpsImpl(ContextVk *contextVk,
                                           const vk::RenderPassDesc &desc,
                                           const vk::AttachmentOpsArray &attachmentOps,
                                           bool updatePerfCounters,
                                           const vk::RenderPass **renderPassOut);

    angle::Result addCompatibleRenderPass(ContextVk *contextVk,
                                          const vk::RenderPassDesc &desc,
                                          const vk::RenderPass **renderPassOut);

    // Use a two-layer caching scheme. The top level matches the "compatible" RenderPass elements.
    // The second layer caches the attachment load/store ops and initial/final layout.
    // Switch to `std::unordered_map` to retain pointer stability.
    using InnerCache = std::unordered_map<vk::AttachmentOpsArray, vk::RenderPassHelper>;
    using OuterCache = std::unordered_map<vk::RenderPassDesc, InnerCache>;

    OuterCache mPayload;
    CacheStats mCompatibleRenderPassCacheStats;
    CacheStats mRenderPassWithOpsCacheStats;
};

enum class PipelineSource
{
    // Pipeline created when warming up the program's pipeline cache
    WarmUp,
    // Monolithic pipeline created at draw time
    Draw,
    // Pipeline created at draw time by linking partial pipeline libraries
    DrawLinked,
    // Pipeline created for UtilsVk
    Utils,
    // Pipeline created at dispatch time
    Dispatch
};

struct ComputePipelineDescHash
{
    size_t operator()(const rx::vk::ComputePipelineDesc &key) const { return key.hash(); }
};
struct GraphicsPipelineDescCompleteHash
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &key) const
    {
        return key.hash(vk::GraphicsPipelineSubset::Complete);
    }
};
struct GraphicsPipelineDescVertexInputHash
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &key) const
    {
        return key.hash(vk::GraphicsPipelineSubset::VertexInput);
    }
};
struct GraphicsPipelineDescShadersHash
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &key) const
    {
        return key.hash(vk::GraphicsPipelineSubset::Shaders);
    }
};
struct GraphicsPipelineDescFragmentOutputHash
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &key) const
    {
        return key.hash(vk::GraphicsPipelineSubset::FragmentOutput);
    }
};

struct ComputePipelineDescKeyEqual
{
    size_t operator()(const rx::vk::ComputePipelineDesc &first,
                      const rx::vk::ComputePipelineDesc &second) const
    {
        return first.keyEqual(second);
    }
};
struct GraphicsPipelineDescCompleteKeyEqual
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &first,
                      const rx::vk::GraphicsPipelineDesc &second) const
    {
        return first.keyEqual(second, vk::GraphicsPipelineSubset::Complete);
    }
};
struct GraphicsPipelineDescVertexInputKeyEqual
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &first,
                      const rx::vk::GraphicsPipelineDesc &second) const
    {
        return first.keyEqual(second, vk::GraphicsPipelineSubset::VertexInput);
    }
};
struct GraphicsPipelineDescShadersKeyEqual
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &first,
                      const rx::vk::GraphicsPipelineDesc &second) const
    {
        return first.keyEqual(second, vk::GraphicsPipelineSubset::Shaders);
    }
};
struct GraphicsPipelineDescFragmentOutputKeyEqual
{
    size_t operator()(const rx::vk::GraphicsPipelineDesc &first,
                      const rx::vk::GraphicsPipelineDesc &second) const
    {
        return first.keyEqual(second, vk::GraphicsPipelineSubset::FragmentOutput);
    }
};

// Derive the KeyEqual and GraphicsPipelineSubset enum from the Hash struct
template <typename Hash>
struct GraphicsPipelineCacheTypeHelper
{
    using KeyEqual                                      = GraphicsPipelineDescCompleteKeyEqual;
    static constexpr vk::GraphicsPipelineSubset kSubset = vk::GraphicsPipelineSubset::Complete;
};

template <>
struct GraphicsPipelineCacheTypeHelper<GraphicsPipelineDescVertexInputHash>
{
    using KeyEqual                                      = GraphicsPipelineDescVertexInputKeyEqual;
    static constexpr vk::GraphicsPipelineSubset kSubset = vk::GraphicsPipelineSubset::VertexInput;
};
template <>
struct GraphicsPipelineCacheTypeHelper<GraphicsPipelineDescShadersHash>
{
    using KeyEqual                                      = GraphicsPipelineDescShadersKeyEqual;
    static constexpr vk::GraphicsPipelineSubset kSubset = vk::GraphicsPipelineSubset::Shaders;
};
template <>
struct GraphicsPipelineCacheTypeHelper<GraphicsPipelineDescFragmentOutputHash>
{
    using KeyEqual = GraphicsPipelineDescFragmentOutputKeyEqual;
    static constexpr vk::GraphicsPipelineSubset kSubset =
        vk::GraphicsPipelineSubset::FragmentOutput;
};

// Compute Pipeline Cache implementation
// TODO(aannestrand): Add cache trimming/eviction.
// http://anglebug.com/391672281
class ComputePipelineCache final : HasCacheStats<rx::VulkanCacheType::ComputePipeline>
{
  public:
    ComputePipelineCache() = default;
    ~ComputePipelineCache() override { ASSERT(mPayload.empty()); }

    void destroy(vk::ErrorContext *context);
    void release(vk::ErrorContext *context);

    angle::Result getOrCreatePipeline(vk::ErrorContext *context,
                                      vk::PipelineCacheAccess *pipelineCache,
                                      const vk::PipelineLayout &pipelineLayout,
                                      vk::ComputePipelineOptions &pipelineOptions,
                                      PipelineSource source,
                                      vk::PipelineHelper **pipelineOut,
                                      const char *shaderName,
                                      VkSpecializationInfo *specializationInfo,
                                      const vk::ShaderModuleMap &shaderModuleMap);

  private:
    angle::Result createPipeline(vk::ErrorContext *context,
                                 vk::PipelineCacheAccess *pipelineCache,
                                 const vk::PipelineLayout &pipelineLayout,
                                 vk::ComputePipelineOptions &pipelineOptions,
                                 PipelineSource source,
                                 const char *shaderName,
                                 const vk::ShaderModule &shaderModule,
                                 VkSpecializationInfo *specializationInfo,
                                 const vk::ComputePipelineDesc &desc,
                                 vk::PipelineHelper **pipelineOut);

    std::unordered_map<vk::ComputePipelineDesc,
                       vk::PipelineHelper,
                       ComputePipelineDescHash,
                       ComputePipelineDescKeyEqual>
        mPayload;
};

// TODO(jmadill): Add cache trimming/eviction.
template <typename Hash>
class GraphicsPipelineCache final : public HasCacheStats<VulkanCacheType::GraphicsPipeline>
{
  public:
    GraphicsPipelineCache() = default;
    ~GraphicsPipelineCache() override { ASSERT(mPayload.empty()); }

    void destroy(vk::ErrorContext *context);
    void release(vk::ErrorContext *context);

    void populate(const vk::GraphicsPipelineDesc &desc,
                  vk::Pipeline &&pipeline,
                  vk::PipelineHelper **pipelineHelperOut);

    // Get a pipeline from the cache, if it exists
    ANGLE_INLINE bool getPipeline(const vk::GraphicsPipelineDesc &desc,
                                  const vk::GraphicsPipelineDesc **descPtrOut,
                                  vk::PipelineHelper **pipelineOut)
    {
        auto item = mPayload.find(desc);
        if (item == mPayload.end())
        {
            return false;
        }

        *descPtrOut  = &item->first;
        *pipelineOut = &item->second;

        mCacheStats.hit();

        return true;
    }

    angle::Result createPipeline(vk::ErrorContext *context,
                                 vk::PipelineCacheAccess *pipelineCache,
                                 const vk::RenderPass &compatibleRenderPass,
                                 const vk::PipelineLayout &pipelineLayout,
                                 const vk::ShaderModuleMap &shaders,
                                 const vk::SpecializationConstants &specConsts,
                                 PipelineSource source,
                                 const vk::GraphicsPipelineDesc &desc,
                                 const vk::GraphicsPipelineDesc **descPtrOut,
                                 vk::PipelineHelper **pipelineOut);

    angle::Result linkLibraries(vk::ErrorContext *context,
                                vk::PipelineCacheAccess *pipelineCache,
                                const vk::GraphicsPipelineDesc &desc,
                                const vk::PipelineLayout &pipelineLayout,
                                vk::PipelineHelper *vertexInputPipeline,
                                vk::PipelineHelper *shadersPipeline,
                                vk::PipelineHelper *fragmentOutputPipeline,
                                const vk::GraphicsPipelineDesc **descPtrOut,
                                vk::PipelineHelper **pipelineOut);

    // Helper for VulkanPipelineCachePerf that resets the object without destroying any object.
    void reset() { mPayload.clear(); }

  private:
    void addToCache(PipelineSource source,
                    const vk::GraphicsPipelineDesc &desc,
                    vk::Pipeline &&pipeline,
                    vk::CacheLookUpFeedback feedback,
                    const vk::GraphicsPipelineDesc **descPtrOut,
                    vk::PipelineHelper **pipelineOut);

    using KeyEqual = typename GraphicsPipelineCacheTypeHelper<Hash>::KeyEqual;
    std::unordered_map<vk::GraphicsPipelineDesc, vk::PipelineHelper, Hash, KeyEqual> mPayload;
};

using CompleteGraphicsPipelineCache    = GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>;
using VertexInputGraphicsPipelineCache = GraphicsPipelineCache<GraphicsPipelineDescVertexInputHash>;
using ShadersGraphicsPipelineCache     = GraphicsPipelineCache<GraphicsPipelineDescShadersHash>;
using FragmentOutputGraphicsPipelineCache =
    GraphicsPipelineCache<GraphicsPipelineDescFragmentOutputHash>;

class DescriptorSetLayoutCache final : angle::NonCopyable
{
  public:
    DescriptorSetLayoutCache();
    ~DescriptorSetLayoutCache();

    void destroy(vk::Renderer *renderer);

    angle::Result getDescriptorSetLayout(vk::ErrorContext *context,
                                         const vk::DescriptorSetLayoutDesc &desc,
                                         vk::DescriptorSetLayoutPtr *descriptorSetLayoutOut);

    // Helpers for white box tests
    size_t getCacheHitCount() const { return mCacheStats.getHitCount(); }
    size_t getCacheMissCount() const { return mCacheStats.getMissCount(); }

  private:
    mutable angle::SimpleMutex mMutex;
    std::unordered_map<vk::DescriptorSetLayoutDesc, vk::DescriptorSetLayoutPtr> mPayload;
    CacheStats mCacheStats;
};

class PipelineLayoutCache final : public HasCacheStats<VulkanCacheType::PipelineLayout>
{
  public:
    PipelineLayoutCache();
    ~PipelineLayoutCache() override;

    void destroy(vk::Renderer *renderer);

    angle::Result getPipelineLayout(vk::ErrorContext *context,
                                    const vk::PipelineLayoutDesc &desc,
                                    const vk::DescriptorSetLayoutPointerArray &descriptorSetLayouts,
                                    vk::PipelineLayoutPtr *pipelineLayoutOut);

  private:
    mutable angle::SimpleMutex mMutex;
    std::unordered_map<vk::PipelineLayoutDesc, vk::PipelineLayoutPtr> mPayload;
};

class SamplerCache final : public HasCacheStats<VulkanCacheType::Sampler>
{
  public:
    SamplerCache();
    ~SamplerCache() override;

    void destroy(vk::Renderer *renderer);

    angle::Result getSampler(ContextVk *contextVk,
                             const vk::SamplerDesc &desc,
                             vk::SharedSamplerPtr *samplerOut);

  private:
    std::unordered_map<vk::SamplerDesc, vk::SharedSamplerPtr> mPayload;
};

// YuvConversion Cache
class SamplerYcbcrConversionCache final
    : public HasCacheStats<VulkanCacheType::SamplerYcbcrConversion>
{
  public:
    SamplerYcbcrConversionCache();
    ~SamplerYcbcrConversionCache() override;

    void destroy(vk::Renderer *renderer);

    angle::Result getSamplerYcbcrConversion(vk::ErrorContext *context,
                                            const vk::YcbcrConversionDesc &ycbcrConversionDesc,
                                            VkSamplerYcbcrConversion *vkSamplerYcbcrConversionOut);

  private:
    using SamplerYcbcrConversionMap =
        std::unordered_map<vk::YcbcrConversionDesc, vk::SamplerYcbcrConversion>;
    SamplerYcbcrConversionMap mExternalFormatPayload;
    SamplerYcbcrConversionMap mVkFormatPayload;
};

// Descriptor Set Cache
template <typename T>
class DescriptorSetCache final : angle::NonCopyable
{
  public:
    DescriptorSetCache() = default;
    ~DescriptorSetCache() { ASSERT(mPayload.empty()); }

    DescriptorSetCache(DescriptorSetCache &&other) : DescriptorSetCache()
    {
        *this = std::move(other);
    }

    DescriptorSetCache &operator=(DescriptorSetCache &&other)
    {
        std::swap(mPayload, other.mPayload);
        return *this;
    }

    void clear() { mPayload.clear(); }

    bool getDescriptorSet(const vk::DescriptorSetDesc &desc, T *descriptorSetOut)
    {
        auto iter = mPayload.find(desc);
        if (iter != mPayload.end())
        {
            *descriptorSetOut = iter->second;
            return true;
        }
        return false;
    }

    void insertDescriptorSet(const vk::DescriptorSetDesc &desc, const T &descriptorSetHelper)
    {
        mPayload.emplace(desc, descriptorSetHelper);
    }

    bool eraseDescriptorSet(const vk::DescriptorSetDesc &desc, T *descriptorSetOut)
    {
        auto iter = mPayload.find(desc);
        if (iter != mPayload.end())
        {
            *descriptorSetOut = std::move(iter->second);
            mPayload.erase(iter);
            return true;
        }
        return false;
    }

    bool eraseDescriptorSet(const vk::DescriptorSetDesc &desc)
    {
        auto iter = mPayload.find(desc);
        if (iter != mPayload.end())
        {
            mPayload.erase(iter);
            return true;
        }
        return false;
    }

    size_t getTotalCacheSize() const { return mPayload.size(); }

    size_t getTotalCacheKeySizeBytes() const
    {
        size_t totalSize = 0;
        for (const auto &iter : mPayload)
        {
            const vk::DescriptorSetDesc &desc = iter.first;
            totalSize += desc.getKeySizeBytes();
        }
        return totalSize;
    }
    bool empty() const { return mPayload.empty(); }

  private:
    angle::HashMap<vk::DescriptorSetDesc, T> mPayload;
};

// There is 1 default uniform binding used per stage.
constexpr uint32_t kReservedPerStageDefaultUniformBindingCount = 1;

class UpdateDescriptorSetsBuilder final : angle::NonCopyable
{
  public:
    UpdateDescriptorSetsBuilder();
    ~UpdateDescriptorSetsBuilder();

    VkDescriptorBufferInfo *allocDescriptorBufferInfos(size_t count);
    VkDescriptorImageInfo *allocDescriptorImageInfos(size_t count);
    VkWriteDescriptorSet *allocWriteDescriptorSets(size_t count);
    VkBufferView *allocBufferViews(size_t count);

    VkDescriptorBufferInfo &allocDescriptorBufferInfo() { return *allocDescriptorBufferInfos(1); }
    VkDescriptorImageInfo &allocDescriptorImageInfo() { return *allocDescriptorImageInfos(1); }
    VkWriteDescriptorSet &allocWriteDescriptorSet() { return *allocWriteDescriptorSets(1); }
    VkBufferView &allocBufferView() { return *allocBufferViews(1); }

    // Returns the number of written descriptor sets.
    uint32_t flushDescriptorSetUpdates(VkDevice device);

  private:
    template <typename T, const T *VkWriteDescriptorSet::*pInfo>
    T *allocDescriptorInfos(std::vector<T> *descriptorVector, size_t count);
    template <typename T, const T *VkWriteDescriptorSet::*pInfo>
    void growDescriptorCapacity(std::vector<T> *descriptorVector, size_t newSize);

    std::vector<VkDescriptorBufferInfo> mDescriptorBufferInfos;
    std::vector<VkDescriptorImageInfo> mDescriptorImageInfos;
    std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
    std::vector<VkBufferView> mBufferViews;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VK_CACHE_UTILS_H_
