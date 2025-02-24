//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// serialize.cpp:
//   ANGLE GL state serialization.
//

#include "libANGLE/capture/serialize.h"

#include "common/Color.h"
#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "common/gl_enum_utils.h"
#include "common/serializer/JsonSerializer.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Query.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/ResourceMap.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/State.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/RenderbufferImpl.h"

#include <vector>

#if !ANGLE_CAPTURE_ENABLED
#    error Frame capture must be enabled to build this file.
#endif  // !ANGLE_CAPTURE_ENABLED

// Note: when diagnosing serialization comparison failures, you can disable the unused function
// compiler warning to allow bisecting the comparison function. One first check is to disable
// Framebuffer Attachment pixel comparison which includes the pixel contents of the default FBO.
// ANGLE_DISABLE_UNUSED_FUNCTION_WARNING

namespace angle
{
namespace
{
template <typename ArgT>
std::string ToString(const ArgT &arg)
{
    std::ostringstream strstr;
    strstr << arg;
    return strstr.str();
}

#define ENUM_TO_STRING(C, M) \
    case C ::M:              \
        return #M

const char *InitStateToString(gl::InitState state)
{
    switch (state)
    {
        ENUM_TO_STRING(gl::InitState, Initialized);
        ENUM_TO_STRING(gl::InitState, MayNeedInit);
        default:
            return "invalid";
    }
}

const char *SrgbOverrideToString(gl::SrgbOverride value)
{
    switch (value)
    {
        ENUM_TO_STRING(gl::SrgbOverride, Default);
        ENUM_TO_STRING(gl::SrgbOverride, SRGB);
        default:
            return "invalid";
    }
}

const char *ColorGenericTypeToString(gl::ColorGeneric::Type type)
{
    switch (type)
    {
        ENUM_TO_STRING(gl::ColorGeneric::Type, Float);
        ENUM_TO_STRING(gl::ColorGeneric::Type, Int);
        ENUM_TO_STRING(gl::ColorGeneric::Type, UInt);
        default:
            return "invalid";
    }
}

const char *CompileStatusToString(gl::CompileStatus status)
{
    switch (status)
    {
        ENUM_TO_STRING(gl::CompileStatus, NOT_COMPILED);
        ENUM_TO_STRING(gl::CompileStatus, COMPILE_REQUESTED);
        ENUM_TO_STRING(gl::CompileStatus, COMPILED);
        default:
            return "invalid";
    }
}

#undef ENUM_TO_STRING

class [[nodiscard]] GroupScope
{
  public:
    GroupScope(JsonSerializer *json, const std::string &name) : mJson(json)
    {
        mJson->startGroup(name);
    }

    GroupScope(JsonSerializer *json, const std::string &name, int index) : mJson(json)
    {
        constexpr size_t kBufSize = 255;
        char buf[kBufSize + 1]    = {};
        snprintf(buf, kBufSize, "%s%s%03d", name.c_str(), name.empty() ? "" : " ", index);
        mJson->startGroup(buf);
    }

    GroupScope(JsonSerializer *json, int index) : GroupScope(json, "", index) {}

    ~GroupScope() { mJson->endGroup(); }

  private:
    JsonSerializer *mJson;
};

void SerializeColorF(JsonSerializer *json, const ColorF &color)
{
    json->addScalar("red", color.red);
    json->addScalar("green", color.green);
    json->addScalar("blue", color.blue);
    json->addScalar("alpha", color.alpha);
}

void SerializeColorFWithGroup(JsonSerializer *json, const char *groupName, const ColorF &color)
{
    GroupScope group(json, groupName);
    SerializeColorF(json, color);
}

void SerializeColorI(JsonSerializer *json, const ColorI &color)
{
    json->addScalar("Red", color.red);
    json->addScalar("Green", color.green);
    json->addScalar("Blue", color.blue);
    json->addScalar("Alpha", color.alpha);
}

void SerializeColorUI(JsonSerializer *json, const ColorUI &color)
{
    json->addScalar("Red", color.red);
    json->addScalar("Green", color.green);
    json->addScalar("Blue", color.blue);
    json->addScalar("Alpha", color.alpha);
}

void SerializeExtents(JsonSerializer *json, const gl::Extents &extents)
{
    json->addScalar("Width", extents.width);
    json->addScalar("Height", extents.height);
    json->addScalar("Depth", extents.depth);
}

template <class ObjectType>
void SerializeOffsetBindingPointerVector(
    JsonSerializer *json,
    const char *groupName,
    const std::vector<gl::OffsetBindingPointer<ObjectType>> &offsetBindingPointerVector)
{
    GroupScope vectorGroup(json, groupName);

    for (size_t i = 0; i < offsetBindingPointerVector.size(); i++)
    {
        GroupScope itemGroup(json, static_cast<int>(i));
        json->addScalar("Value", offsetBindingPointerVector[i].id().value);
        json->addScalar("Offset", offsetBindingPointerVector[i].getOffset());
        json->addScalar("Size", offsetBindingPointerVector[i].getSize());
    }
}

template <class ObjectType>
void SerializeBindingPointerVector(
    JsonSerializer *json,
    const std::vector<gl::BindingPointer<ObjectType>> &bindingPointerVector)
{
    for (size_t i = 0; i < bindingPointerVector.size(); i++)
    {
        const gl::BindingPointer<ObjectType> &obj = bindingPointerVector[i];

        // Do not serialize zero bindings, as this will create unwanted diffs
        if (obj.id().value != 0)
        {
            std::ostringstream s;
            s << std::setfill('0') << std::setw(3) << i;
            json->addScalar(s.str().c_str(), obj.id().value);
        }
    }
}

template <class T>
void SerializeRange(JsonSerializer *json, const gl::Range<T> &range)
{
    GroupScope group(json, "Range");
    json->addScalar("Low", range.low());
    json->addScalar("High", range.high());
}

bool IsValidColorAttachmentBinding(GLenum binding, size_t colorAttachmentsCount)
{
    return binding == GL_BACK || (binding >= GL_COLOR_ATTACHMENT0 &&
                                  (binding - GL_COLOR_ATTACHMENT0) < colorAttachmentsCount);
}

void SerializeFormat(JsonSerializer *json, GLenum glFormat)
{
    json->addCString("InternalFormat", gl::GLenumToString(gl::GLESEnum::InternalFormat, glFormat));
}

void SerializeInternalFormat(JsonSerializer *json, const gl::InternalFormat *internalFormat)
{
    SerializeFormat(json, internalFormat->internalFormat);
}

void SerializeANGLEFormat(JsonSerializer *json, const angle::Format *format)
{
    SerializeFormat(json, format->glInternalFormat);
}

void SerializeGLFormat(JsonSerializer *json, const gl::Format &format)
{
    SerializeInternalFormat(json, format.info);
}

Result ReadPixelsFromAttachment(const gl::Context *context,
                                gl::Framebuffer *framebuffer,
                                const gl::FramebufferAttachment &framebufferAttachment,
                                ScratchBuffer *scratchBuffer,
                                MemoryBuffer **pixels)
{
    gl::Extents extents       = framebufferAttachment.getSize();
    GLenum binding            = framebufferAttachment.getBinding();
    gl::InternalFormat format = *framebufferAttachment.getFormat().info;
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        format = framebuffer->getImplementation()->getImplementationColorReadFormat(context);
    }
    ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                         scratchBuffer->getInitialized(
                             format.pixelBytes * extents.width * extents.height, pixels, 0));
    ANGLE_TRY(framebuffer->readPixels(context, gl::Rectangle{0, 0, extents.width, extents.height},
                                      format.format, format.type, gl::PixelPackState{}, nullptr,
                                      (*pixels)->data()));
    return Result::Continue;
}
void SerializeImageIndex(JsonSerializer *json, const gl::ImageIndex &imageIndex)
{
    GroupScope group(json, "Image");
    json->addString("ImageType", ToString(imageIndex.getType()));
    json->addScalar("LevelIndex", imageIndex.getLevelIndex());
    json->addScalar("LayerIndex", imageIndex.getLayerIndex());
    json->addScalar("LayerCount", imageIndex.getLayerCount());
}

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      JsonSerializer *json,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment,
                                      gl::GLESEnum enumGroup)
{
    if (framebufferAttachment.type() == GL_TEXTURE ||
        framebufferAttachment.type() == GL_RENDERBUFFER)
    {
        json->addScalar("AttachedResourceID", framebufferAttachment.id());
    }
    json->addCString(
        "Type", gl::GLenumToString(gl::GLESEnum::ObjectIdentifier, framebufferAttachment.type()));
    // serialize target variable
    json->addString("Binding", gl::GLenumToString(enumGroup, framebufferAttachment.getBinding()));
    if (framebufferAttachment.type() == GL_TEXTURE)
    {
        SerializeImageIndex(json, framebufferAttachment.getTextureImageIndex());
    }
    json->addScalar("NumViews", framebufferAttachment.getNumViews());
    json->addScalar("Multiview", framebufferAttachment.isMultiview());
    json->addScalar("ViewIndex", framebufferAttachment.getBaseViewIndex());
    json->addScalar("Samples", framebufferAttachment.getRenderToTextureSamples());

    {
        GroupScope extentsGroup(json, "Extents");
        SerializeExtents(json, framebufferAttachment.getSize());
    }

    if (framebufferAttachment.type() != GL_TEXTURE &&
        framebufferAttachment.type() != GL_RENDERBUFFER)
    {
        GLenum prevReadBufferState = framebuffer->getReadBufferState();
        GLenum binding             = framebufferAttachment.getBinding();
        if (IsValidColorAttachmentBinding(binding,
                                          framebuffer->getState().getColorAttachments().size()))
        {
            framebuffer->setReadBuffer(framebufferAttachment.getBinding());
            ANGLE_TRY(framebuffer->syncState(context, GL_FRAMEBUFFER, gl::Command::Other));
        }

        if (framebufferAttachment.initState() == gl::InitState::Initialized)
        {
            MemoryBuffer *pixelsPtr = nullptr;
            ANGLE_TRY(ReadPixelsFromAttachment(context, framebuffer, framebufferAttachment,
                                               scratchBuffer, &pixelsPtr));
            json->addBlob("Data", pixelsPtr->data(), pixelsPtr->size());
        }
        else
        {
            json->addCString("Data", "Not initialized");
        }
        // Reset framebuffer state
        framebuffer->setReadBuffer(prevReadBufferState);
    }
    return Result::Continue;
}

Result SerializeFramebufferState(const gl::Context *context,
                                 JsonSerializer *json,
                                 ScratchBuffer *scratchBuffer,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState)
{
    GroupScope group(json, "Framebuffer", framebufferState.id().value);

    json->addString("Label", framebufferState.getLabel());
    json->addVector("DrawStates", framebufferState.getDrawBufferStates());
    json->addScalar("ReadBufferState", framebufferState.getReadBufferState());
    json->addScalar("DefaultWidth", framebufferState.getDefaultWidth());
    json->addScalar("DefaultHeight", framebufferState.getDefaultHeight());
    json->addScalar("DefaultSamples", framebufferState.getDefaultSamples());
    json->addScalar("DefaultFixedSampleLocation",
                    framebufferState.getDefaultFixedSampleLocations());
    json->addScalar("DefaultLayers", framebufferState.getDefaultLayers());
    json->addScalar("FlipY", framebufferState.getFlipY());

    {
        GroupScope attachmentsGroup(json, "Attachments");
        const gl::DrawBuffersVector<gl::FramebufferAttachment> &colorAttachments =
            framebufferState.getColorAttachments();
        for (size_t attachmentIndex = 0; attachmentIndex < colorAttachments.size();
             ++attachmentIndex)
        {
            const gl::FramebufferAttachment &colorAttachment = colorAttachments[attachmentIndex];
            if (colorAttachment.isAttached())
            {
                GroupScope colorAttachmentgroup(json, "ColorAttachment",
                                                static_cast<int>(attachmentIndex));
                ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                         colorAttachment,
                                                         gl::GLESEnum::ColorBuffer));
            }
        }
        if (framebuffer->getDepthStencilAttachment())
        {
            GroupScope dsAttachmentgroup(json, "DepthStencilAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                     *framebuffer->getDepthStencilAttachment(),
                                                     gl::GLESEnum::AllEnums));
        }
        else
        {
            if (framebuffer->getDepthAttachment())
            {
                GroupScope depthAttachmentgroup(json, "DepthAttachment");
                ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                         *framebuffer->getDepthAttachment(),
                                                         gl::GLESEnum::FramebufferAttachment));
            }
            if (framebuffer->getStencilAttachment())
            {
                GroupScope stencilAttachmengroup(json, "StencilAttachment");
                ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                         *framebuffer->getStencilAttachment(),
                                                         gl::GLESEnum::AllEnums));
            }
        }
    }
    return Result::Continue;
}

Result SerializeFramebuffer(const gl::Context *context,
                            JsonSerializer *json,
                            ScratchBuffer *scratchBuffer,
                            gl::Framebuffer *framebuffer)
{
    return SerializeFramebufferState(context, json, scratchBuffer, framebuffer,
                                     framebuffer->getState());
}

void SerializeRasterizerState(JsonSerializer *json, const gl::RasterizerState &rasterizerState)
{
    GroupScope group(json, "Rasterizer");
    json->addScalar("CullFace", rasterizerState.cullFace);
    json->addString("CullMode", ToString(rasterizerState.cullMode));
    json->addScalar("FrontFace", rasterizerState.frontFace);
    json->addString("PolygonMode", ToString(rasterizerState.polygonMode));
    json->addScalar("PolygonOffsetPoint", rasterizerState.polygonOffsetPoint);
    json->addScalar("PolygonOffsetLine", rasterizerState.polygonOffsetLine);
    json->addScalar("PolygonOffsetFill", rasterizerState.polygonOffsetFill);
    json->addScalar("PolygonOffsetFactor", rasterizerState.polygonOffsetFactor);
    json->addScalar("PolygonOffsetUnits", rasterizerState.polygonOffsetUnits);
    json->addScalar("PolygonOffsetClamp", rasterizerState.polygonOffsetClamp);
    json->addScalar("DepthClamp", rasterizerState.depthClamp);
    json->addScalar("PointDrawMode", rasterizerState.pointDrawMode);
    json->addScalar("MultiSample", rasterizerState.multiSample);
    json->addScalar("RasterizerDiscard", rasterizerState.rasterizerDiscard);
    json->addScalar("Dither", rasterizerState.dither);
}

void SerializeRectangle(JsonSerializer *json,
                        const std::string &name,
                        const gl::Rectangle &rectangle)
{
    GroupScope group(json, name);
    json->addScalar("x", rectangle.x);
    json->addScalar("y", rectangle.y);
    json->addScalar("w", rectangle.width);
    json->addScalar("h", rectangle.height);
}

void SerializeBlendStateExt(JsonSerializer *json, const gl::BlendStateExt &blendStateExt)
{
    GroupScope group(json, "BlendStateExt");
    json->addScalar("DrawBufferCount", blendStateExt.getDrawBufferCount());
    json->addScalar("EnableMask", blendStateExt.getEnabledMask().bits());
    json->addScalar("DstColor", blendStateExt.getDstColorBits());
    json->addScalar("DstAlpha", blendStateExt.getDstAlphaBits());
    json->addScalar("SrcColor", blendStateExt.getSrcColorBits());
    json->addScalar("SrcAlpha", blendStateExt.getSrcAlphaBits());
    json->addScalar("EquationColor", blendStateExt.getEquationColorBits());
    json->addScalar("EquationAlpha", blendStateExt.getEquationAlphaBits());
    json->addScalar("ColorMask", blendStateExt.getColorMaskBits());
}

void SerializeDepthStencilState(JsonSerializer *json,
                                const gl::DepthStencilState &depthStencilState)
{
    GroupScope group(json, "DepthStencilState");
    json->addScalar("DepthTest", depthStencilState.depthTest);
    json->addScalar("DepthFunc", depthStencilState.depthFunc);
    json->addScalar("DepthMask", depthStencilState.depthMask);
    json->addScalar("StencilTest", depthStencilState.stencilTest);
    json->addScalar("StencilFunc", depthStencilState.stencilFunc);
    json->addScalar("StencilMask", depthStencilState.stencilMask);
    json->addScalar("StencilFail", depthStencilState.stencilFail);
    json->addScalar("StencilPassDepthFail", depthStencilState.stencilPassDepthFail);
    json->addScalar("StencilPassDepthPass", depthStencilState.stencilPassDepthPass);
    json->addScalar("StencilWritemask", depthStencilState.stencilWritemask);
    json->addScalar("StencilBackFunc", depthStencilState.stencilBackFunc);
    json->addScalar("StencilBackMask", depthStencilState.stencilBackMask);
    json->addScalar("StencilBackFail", depthStencilState.stencilBackFail);
    json->addScalar("StencilBackPassDepthFail", depthStencilState.stencilBackPassDepthFail);
    json->addScalar("StencilBackPassDepthPass", depthStencilState.stencilBackPassDepthPass);
    json->addScalar("StencilBackWritemask", depthStencilState.stencilBackWritemask);
}

void SerializeVertexAttribCurrentValueData(
    JsonSerializer *json,
    const gl::VertexAttribCurrentValueData &vertexAttribCurrentValueData)
{
    ASSERT(vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::UnsignedInt);
    if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float)
    {
        json->addScalar("0", vertexAttribCurrentValueData.Values.FloatValues[0]);
        json->addScalar("1", vertexAttribCurrentValueData.Values.FloatValues[1]);
        json->addScalar("2", vertexAttribCurrentValueData.Values.FloatValues[2]);
        json->addScalar("3", vertexAttribCurrentValueData.Values.FloatValues[3]);
    }
    else if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int)
    {
        json->addScalar("0", vertexAttribCurrentValueData.Values.IntValues[0]);
        json->addScalar("1", vertexAttribCurrentValueData.Values.IntValues[1]);
        json->addScalar("2", vertexAttribCurrentValueData.Values.IntValues[2]);
        json->addScalar("3", vertexAttribCurrentValueData.Values.IntValues[3]);
    }
    else
    {
        json->addScalar("0", vertexAttribCurrentValueData.Values.UnsignedIntValues[0]);
        json->addScalar("1", vertexAttribCurrentValueData.Values.UnsignedIntValues[1]);
        json->addScalar("2", vertexAttribCurrentValueData.Values.UnsignedIntValues[2]);
        json->addScalar("3", vertexAttribCurrentValueData.Values.UnsignedIntValues[3]);
    }
}

void SerializePixelPackState(JsonSerializer *json, const gl::PixelPackState &pixelPackState)
{
    GroupScope group(json, "PixelPackState");
    json->addScalar("Alignment", pixelPackState.alignment);
    json->addScalar("RowLength", pixelPackState.rowLength);
    json->addScalar("SkipRows", pixelPackState.skipRows);
    json->addScalar("SkipPixels", pixelPackState.skipPixels);
    json->addScalar("ImageHeight", pixelPackState.imageHeight);
    json->addScalar("SkipImages", pixelPackState.skipImages);
    json->addScalar("ReverseRowOrder", pixelPackState.reverseRowOrder);
}

void SerializePixelUnpackState(JsonSerializer *json, const gl::PixelUnpackState &pixelUnpackState)
{
    GroupScope group(json, "PixelUnpackState");
    json->addScalar("Alignment", pixelUnpackState.alignment);
    json->addScalar("RowLength", pixelUnpackState.rowLength);
    json->addScalar("SkipRows", pixelUnpackState.skipRows);
    json->addScalar("SkipPixels", pixelUnpackState.skipPixels);
    json->addScalar("ImageHeight", pixelUnpackState.imageHeight);
    json->addScalar("SkipImages", pixelUnpackState.skipImages);
}

void SerializeImageUnit(JsonSerializer *json, const gl::ImageUnit &imageUnit, int imageUnitIndex)
{
    GroupScope group(json, "ImageUnit", imageUnitIndex);
    json->addScalar("Level", imageUnit.level);
    json->addScalar("Layered", imageUnit.layered);
    json->addScalar("Layer", imageUnit.layer);
    json->addScalar("Access", imageUnit.access);
    json->addCString("Format", gl::GLinternalFormatToString(imageUnit.format));
    json->addScalar("TextureID", imageUnit.texture.id().value);
}

template <typename ResourceType>
void SerializeResourceID(JsonSerializer *json, const char *name, const ResourceType *resource)
{
    json->addScalar(name, resource ? resource->id().value : 0);
}

void SerializeContextState(JsonSerializer *json, const gl::State &state)
{
    GroupScope group(json, "ContextState");
    json->addScalar("Priority", state.getContextPriority());
    json->addScalar("Major", state.getClientMajorVersion());
    json->addScalar("Minor", state.getClientMinorVersion());
    SerializeColorFWithGroup(json, "ColorClearValue", state.getColorClearValue());
    json->addScalar("DepthClearValue", state.getDepthClearValue());
    json->addScalar("StencilClearValue", state.getStencilClearValue());
    SerializeRasterizerState(json, state.getRasterizerState());
    json->addScalar("ScissorTestEnabled", state.isScissorTestEnabled());
    SerializeRectangle(json, "Scissors", state.getScissor());
    SerializeBlendStateExt(json, state.getBlendStateExt());
    SerializeColorFWithGroup(json, "BlendColor", state.getBlendColor());
    json->addScalar("SampleAlphaToCoverageEnabled", state.isSampleAlphaToCoverageEnabled());
    json->addScalar("SampleCoverageEnabled", state.isSampleCoverageEnabled());
    json->addScalar("SampleCoverageValue", state.getSampleCoverageValue());
    json->addScalar("SampleCoverageInvert", state.getSampleCoverageInvert());
    json->addScalar("SampleMaskEnabled", state.isSampleMaskEnabled());
    json->addScalar("MaxSampleMaskWords", state.getMaxSampleMaskWords());
    {
        const auto &sampleMaskValues = state.getSampleMaskValues();
        GroupScope maskGroup(json, "SampleMaskValues");
        for (size_t i = 0; i < sampleMaskValues.size(); i++)
        {
            json->addScalar(ToString(i), sampleMaskValues[i]);
        }
    }
    SerializeDepthStencilState(json, state.getDepthStencilState());
    json->addScalar("StencilRef", state.getStencilRef());
    json->addScalar("StencilBackRef", state.getStencilBackRef());
    json->addScalar("LineWidth", state.getLineWidth());
    json->addScalar("GenerateMipmapHint", state.getGenerateMipmapHint());
    json->addScalar("FragmentShaderDerivativeHint", state.getFragmentShaderDerivativeHint());
    json->addScalar("BindGeneratesResourceEnabled", state.isBindGeneratesResourceEnabled());
    json->addScalar("ClientArraysEnabled", state.areClientArraysEnabled());
    SerializeRectangle(json, "Viewport", state.getViewport());
    json->addScalar("Near", state.getNearPlane());
    json->addScalar("Far", state.getFarPlane());
    json->addString("ClipOrigin", ToString(state.getClipOrigin()));
    json->addString("ClipDepthMode", ToString(state.getClipDepthMode()));
    SerializeResourceID(json, "ReadFramebufferID", state.getReadFramebuffer());
    SerializeResourceID(json, "DrawFramebufferID", state.getDrawFramebuffer());
    json->addScalar("RenderbufferID", state.getRenderbufferId().value);
    SerializeResourceID(json, "CurrentProgramID", state.getProgram());
    SerializeResourceID(json, "CurrentProgramPipelineID", state.getProgramPipeline());
    json->addString("ProvokingVertex", ToString(state.getProvokingVertex()));
    const std::vector<gl::VertexAttribCurrentValueData> &vertexAttribCurrentValues =
        state.getVertexAttribCurrentValues();
    for (size_t i = 0; i < vertexAttribCurrentValues.size(); i++)
    {
        GroupScope vagroup(json, "VertexAttribCurrentValue", static_cast<int>(i));
        SerializeVertexAttribCurrentValueData(json, vertexAttribCurrentValues[i]);
    }
    ASSERT(state.getVertexArray());
    json->addScalar("VertexArrayID", state.getVertexArray()->id().value);
    json->addScalar("CurrentValuesTypeMask", state.getCurrentValuesTypeMask().to_ulong());
    json->addScalar("ActiveSampler", state.getActiveSampler());
    {
        GroupScope boundTexturesGroup(json, "BoundTextures");
        const gl::TextureBindingMap &boundTexturesMap = state.getBoundTexturesForCapture();
        for (gl::TextureType textureType : AllEnums<gl::TextureType>())
        {
            const gl::TextureBindingVector &textures = boundTexturesMap[textureType];
            GroupScope texturesGroup(json, ToString(textureType));
            SerializeBindingPointerVector<gl::Texture>(json, textures);
        }
    }
    json->addScalar("TexturesIncompatibleWithSamplers",
                    state.getTexturesIncompatibleWithSamplers().to_ulong());

    {
        GroupScope texturesCacheGroup(json, "ActiveTexturesCache");

        const gl::ActiveTexturesCache &texturesCache = state.getActiveTexturesCache();
        for (GLuint textureIndex = 0; textureIndex < texturesCache.size(); ++textureIndex)
        {
            const gl::Texture *tex = texturesCache[textureIndex];
            std::stringstream strstr;
            strstr << "Tex " << std::setfill('0') << std::setw(2) << textureIndex;
            json->addScalar(strstr.str(), tex ? tex->id().value : 0);
        }
    }

    {
        GroupScope samplersGroupScope(json, "Samplers");
        SerializeBindingPointerVector<gl::Sampler>(json, state.getSamplers());
    }

    {
        GroupScope imageUnitsGroup(json, "BoundImageUnits");

        const std::vector<gl::ImageUnit> &imageUnits = state.getImageUnits();
        for (size_t imageUnitIndex = 0; imageUnitIndex < imageUnits.size(); ++imageUnitIndex)
        {
            const gl::ImageUnit &imageUnit = imageUnits[imageUnitIndex];
            SerializeImageUnit(json, imageUnit, static_cast<int>(imageUnitIndex));
        }
    }

    {
        const gl::ActiveQueryMap &activeQueries = state.getActiveQueriesForCapture();
        GroupScope activeQueriesGroup(json, "ActiveQueries");
        for (gl::QueryType queryType : AllEnums<gl::QueryType>())
        {
            const gl::BindingPointer<gl::Query> &query = activeQueries[queryType];
            std::stringstream strstr;
            strstr << queryType;
            json->addScalar(strstr.str(), query.id().value);
        }
    }

    {
        const gl::BoundBufferMap &boundBuffers = state.getBoundBuffersForCapture();
        GroupScope boundBuffersGroup(json, "BoundBuffers");
        for (gl::BufferBinding bufferBinding : AllEnums<gl::BufferBinding>())
        {
            const gl::BindingPointer<gl::Buffer> &buffer = boundBuffers[bufferBinding];
            std::stringstream strstr;
            strstr << bufferBinding;
            json->addScalar(strstr.str(), buffer.id().value);
        }
    }

    SerializeOffsetBindingPointerVector<gl::Buffer>(json, "UniformBufferBindings",
                                                    state.getOffsetBindingPointerUniformBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        json, "AtomicCounterBufferBindings", state.getOffsetBindingPointerAtomicCounterBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        json, "ShaderStorageBufferBindings", state.getOffsetBindingPointerShaderStorageBuffers());
    if (state.getCurrentTransformFeedback())
    {
        json->addScalar("CurrentTransformFeedback",
                        state.getCurrentTransformFeedback()->id().value);
    }
    SerializePixelUnpackState(json, state.getUnpackState());
    SerializePixelPackState(json, state.getPackState());
    json->addScalar("PrimitiveRestartEnabled", state.isPrimitiveRestartEnabled());
    json->addScalar("MultisamplingEnabled", state.isMultisamplingEnabled());
    json->addScalar("SampleAlphaToOneEnabled", state.isSampleAlphaToOneEnabled());
    json->addScalar("CoverageModulation", state.getCoverageModulation());
    json->addScalar("FramebufferSRGB", state.getFramebufferSRGB());
    json->addScalar("RobustResourceInitEnabled", state.isRobustResourceInitEnabled());
    json->addScalar("ProgramBinaryCacheEnabled", state.isProgramBinaryCacheEnabled());
    json->addScalar("TextureRectangleEnabled", state.isTextureRectangleEnabled());
    json->addScalar("MaxShaderCompilerThreads", state.getMaxShaderCompilerThreads());
    json->addScalar("EnabledClipDistances", state.getEnabledClipDistances().to_ulong());
    json->addScalar("BlendFuncConstantAlphaDrawBuffers",
                    state.getBlendFuncConstantAlphaDrawBuffers().to_ulong());
    json->addScalar("BlendFuncConstantColorDrawBuffers",
                    state.getBlendFuncConstantColorDrawBuffers().to_ulong());
    json->addScalar("SimultaneousConstantColorAndAlphaBlendFunc",
                    state.noSimultaneousConstantColorAndAlphaBlendFunc());
}

void SerializeBufferState(JsonSerializer *json, const gl::BufferState &bufferState)
{
    json->addString("Label", bufferState.getLabel());
    json->addString("Usage", ToString(bufferState.getUsage()));
    json->addScalar("Size", bufferState.getSize());
    json->addScalar("AccessFlags", bufferState.getAccessFlags());
    json->addScalar("Access", bufferState.getAccess());
    json->addScalar("Mapped", bufferState.isMapped());
    json->addScalar("MapOffset", bufferState.getMapOffset());
    json->addScalar("MapLength", bufferState.getMapLength());
}

Result SerializeBuffer(const gl::Context *context,
                       JsonSerializer *json,
                       ScratchBuffer *scratchBuffer,
                       gl::Buffer *buffer)
{
    GroupScope group(json, "Buffer", buffer->id().value);
    SerializeBufferState(json, buffer->getState());
    if (buffer->getSize() > 0)
    {
        MemoryBuffer *dataPtr = nullptr;
        ANGLE_CHECK_GL_ALLOC(
            const_cast<gl::Context *>(context),
            scratchBuffer->getInitialized(static_cast<size_t>(buffer->getSize()), &dataPtr, 0));
        ANGLE_TRY(buffer->getSubData(context, 0, dataPtr->size(), dataPtr->data()));
        json->addBlob("data", dataPtr->data(), dataPtr->size());
    }
    else
    {
        json->addCString("data", "null");
    }
    return Result::Continue;
}

void SerializeColorGeneric(JsonSerializer *json,
                           const std::string &name,
                           const ColorGeneric &colorGeneric)
{
    GroupScope group(json, name);
    ASSERT(colorGeneric.type == ColorGeneric::Type::Float ||
           colorGeneric.type == ColorGeneric::Type::Int ||
           colorGeneric.type == ColorGeneric::Type::UInt);
    json->addCString("Type", ColorGenericTypeToString(colorGeneric.type));
    if (colorGeneric.type == ColorGeneric::Type::Float)
    {
        SerializeColorF(json, colorGeneric.colorF);
    }
    else if (colorGeneric.type == ColorGeneric::Type::Int)
    {
        SerializeColorI(json, colorGeneric.colorI);
    }
    else
    {
        SerializeColorUI(json, colorGeneric.colorUI);
    }
}

void SerializeSamplerState(JsonSerializer *json, const gl::SamplerState &samplerState)
{
    json->addScalar("MinFilter", samplerState.getMinFilter());
    json->addScalar("MagFilter", samplerState.getMagFilter());
    json->addScalar("WrapS", samplerState.getWrapS());
    json->addScalar("WrapT", samplerState.getWrapT());
    json->addScalar("WrapR", samplerState.getWrapR());
    json->addScalar("MaxAnisotropy", samplerState.getMaxAnisotropy());
    json->addScalar("MinLod", samplerState.getMinLod());
    json->addScalar("MaxLod", samplerState.getMaxLod());
    json->addScalar("CompareMode", samplerState.getCompareMode());
    json->addScalar("CompareFunc", samplerState.getCompareFunc());
    json->addScalar("SRGBDecode", samplerState.getSRGBDecode());
    SerializeColorGeneric(json, "BorderColor", samplerState.getBorderColor());
}

void SerializeSampler(JsonSerializer *json, gl::Sampler *sampler)
{
    GroupScope group(json, "Sampler", sampler->id().value);
    json->addString("Label", sampler->getLabel());
    SerializeSamplerState(json, sampler->getSamplerState());
}

void SerializeSwizzleState(JsonSerializer *json, const gl::SwizzleState &swizzleState)
{
    json->addScalar("SwizzleRed", swizzleState.swizzleRed);
    json->addScalar("SwizzleGreen", swizzleState.swizzleGreen);
    json->addScalar("SwizzleBlue", swizzleState.swizzleBlue);
    json->addScalar("SwizzleAlpha", swizzleState.swizzleAlpha);
}

void SerializeRenderbufferState(JsonSerializer *json,
                                const gl::RenderbufferState &renderbufferState)
{
    GroupScope wg(json, "State");
    json->addScalar("Width", renderbufferState.getWidth());
    json->addScalar("Height", renderbufferState.getHeight());
    SerializeGLFormat(json, renderbufferState.getFormat());
    json->addScalar("Samples", renderbufferState.getSamples());
    json->addCString("InitState", InitStateToString(renderbufferState.getInitState()));
}

Result SerializeRenderbuffer(const gl::Context *context,
                             JsonSerializer *json,
                             ScratchBuffer *scratchBuffer,
                             gl::Renderbuffer *renderbuffer)
{
    GroupScope wg(json, "Renderbuffer", renderbuffer->id().value);
    SerializeRenderbufferState(json, renderbuffer->getState());
    json->addString("Label", renderbuffer->getLabel());

    if (renderbuffer->initState(GL_NONE, gl::ImageIndex()) == gl::InitState::Initialized)
    {
        if (renderbuffer->getSamples() > 1 && renderbuffer->getFormat().info->depthBits > 0)
        {
            // Vulkan can't do resolve blits for multisampled depth attachemnts and
            // we don't implement an emulation, therefore we can't read back any useful
            // data here.
            json->addCString("Pixels", "multisampled depth buffer");
        }
        else if (renderbuffer->getWidth() * renderbuffer->getHeight() <= 0)
        {
            json->addCString("Pixels", "no pixels");
        }
        else
        {
            const gl::InternalFormat &format = *renderbuffer->getFormat().info;

            const gl::Extents size(renderbuffer->getWidth(), renderbuffer->getHeight(), 1);
            gl::PixelPackState packState;
            packState.alignment = 1;

            GLenum readFormat = renderbuffer->getImplementationColorReadFormat(context);
            GLenum readType   = renderbuffer->getImplementationColorReadType(context);

            GLuint bytes = 0;
            bool computeOK =
                format.computePackUnpackEndByte(readType, size, packState, false, &bytes);
            ASSERT(computeOK);

            MemoryBuffer *pixelsPtr = nullptr;
            ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                                 scratchBuffer->getInitialized(bytes, &pixelsPtr, 0));

            ANGLE_TRY(renderbuffer->getImplementation()->getRenderbufferImage(
                context, packState, nullptr, readFormat, readType, pixelsPtr->data()));
            json->addBlob("Pixels", pixelsPtr->data(), pixelsPtr->size());
        }
    }
    else
    {
        json->addCString("Pixels", "Not initialized");
    }
    return Result::Continue;
}

void SerializeWorkGroupSize(JsonSerializer *json, const sh::WorkGroupSize &workGroupSize)
{
    GroupScope wg(json, "workGroupSize");
    json->addScalar("x", workGroupSize[0]);
    json->addScalar("y", workGroupSize[1]);
    json->addScalar("z", workGroupSize[2]);
}

void SerializeUniformIndexToBufferBinding(JsonSerializer *json,
                                          const gl::ProgramUniformBlockArray<GLuint> &blockToBuffer)
{
    GroupScope wg(json, "uniformBlockIndexToBufferBinding");
    for (size_t blockIndex = 0; blockIndex < blockToBuffer.size(); ++blockIndex)
    {
        json->addScalar(ToString(blockIndex), blockToBuffer[blockIndex]);
    }
}

void SerializeShaderVariable(JsonSerializer *json, const sh::ShaderVariable &shaderVariable)
{
    GroupScope wg(json, "ShaderVariable");
    json->addScalar("Type", shaderVariable.type);
    json->addScalar("Precision", shaderVariable.precision);
    json->addString("Name", shaderVariable.name);
    json->addString("MappedName", shaderVariable.mappedName);
    json->addVector("ArraySizes", shaderVariable.arraySizes);
    json->addScalar("StaticUse", shaderVariable.staticUse);
    json->addScalar("Active", shaderVariable.active);
    for (const sh::ShaderVariable &field : shaderVariable.fields)
    {
        SerializeShaderVariable(json, field);
    }
    json->addString("StructOrBlockName", shaderVariable.structOrBlockName);
    json->addString("MappedStructOrBlockName", shaderVariable.mappedStructOrBlockName);
    json->addScalar("RowMajorLayout", shaderVariable.isRowMajorLayout);
    json->addScalar("Location", shaderVariable.location);
    json->addScalar("Binding", shaderVariable.binding);
    json->addScalar("ImageUnitFormat", shaderVariable.imageUnitFormat);
    json->addScalar("Offset", shaderVariable.offset);
    json->addScalar("Readonly", shaderVariable.readonly);
    json->addScalar("Writeonly", shaderVariable.writeonly);
    json->addScalar("Index", shaderVariable.index);
    json->addScalar("YUV", shaderVariable.yuv);
    json->addCString("Interpolation", InterpolationTypeToString(shaderVariable.interpolation));
    json->addScalar("Invariant", shaderVariable.isInvariant);
    json->addScalar("TexelFetchStaticUse", shaderVariable.texelFetchStaticUse);
}

void SerializeShaderVariablesVector(JsonSerializer *json,
                                    const std::vector<sh::ShaderVariable> &shaderVariables)
{
    for (const sh::ShaderVariable &shaderVariable : shaderVariables)
    {
        SerializeShaderVariable(json, shaderVariable);
    }
}

void SerializeInterfaceBlocksVector(JsonSerializer *json,
                                    const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    for (const sh::InterfaceBlock &interfaceBlock : interfaceBlocks)
    {
        GroupScope group(json, "Interface Block");
        json->addString("Name", interfaceBlock.name);
        json->addString("MappedName", interfaceBlock.mappedName);
        json->addString("InstanceName", interfaceBlock.instanceName);
        json->addScalar("ArraySize", interfaceBlock.arraySize);
        json->addCString("Layout", BlockLayoutTypeToString(interfaceBlock.layout));
        json->addScalar("Binding", interfaceBlock.binding);
        json->addScalar("StaticUse", interfaceBlock.staticUse);
        json->addScalar("Active", interfaceBlock.active);
        json->addCString("BlockType", BlockTypeToString(interfaceBlock.blockType));
        SerializeShaderVariablesVector(json, interfaceBlock.fields);
    }
}

void SerializeCompiledShaderState(JsonSerializer *json, const gl::SharedCompiledShaderState &state)
{
    json->addCString("Type", gl::ShaderTypeToString(state->shaderType));
    json->addScalar("Version", state->shaderVersion);
    json->addString("TranslatedSource", state->translatedSource);
    json->addVectorAsHash("CompiledBinary", state->compiledBinary);
    SerializeWorkGroupSize(json, state->localSize);
    SerializeShaderVariablesVector(json, state->inputVaryings);
    SerializeShaderVariablesVector(json, state->outputVaryings);
    SerializeShaderVariablesVector(json, state->uniforms);
    SerializeInterfaceBlocksVector(json, state->uniformBlocks);
    SerializeInterfaceBlocksVector(json, state->shaderStorageBlocks);
    SerializeShaderVariablesVector(json, state->allAttributes);
    SerializeShaderVariablesVector(json, state->activeAttributes);
    SerializeShaderVariablesVector(json, state->activeOutputVariables);
    json->addScalar("NumViews", state->numViews);
    json->addScalar("SpecConstUsageBits", state->specConstUsageBits.bits());
    json->addScalar("MetadataFlags", state->metadataFlags.bits());
    json->addScalar("AdvancedBlendEquations", state->advancedBlendEquations.bits());
    json->addString("GeometryShaderInputPrimitiveType",
                    ToString(state->geometryShaderInputPrimitiveType));
    json->addString("GeometryShaderOutputPrimitiveType",
                    ToString(state->geometryShaderOutputPrimitiveType));
    json->addScalar("GeometryShaderMaxVertices", state->geometryShaderMaxVertices);
    json->addScalar("GeometryShaderInvocations", state->geometryShaderInvocations);
    json->addScalar("TessControlShaderVertices", state->tessControlShaderVertices);
    json->addScalar("TessGenMode", state->tessGenMode);
    json->addScalar("TessGenSpacing", state->tessGenSpacing);
    json->addScalar("TessGenVertexOrder", state->tessGenVertexOrder);
    json->addScalar("TessGenPointMode", state->tessGenPointMode);
}

void SerializeShaderState(JsonSerializer *json, const gl::ShaderState &shaderState)
{
    GroupScope group(json, "ShaderState");
    json->addString("Label", shaderState.getLabel());
    json->addString("Source", shaderState.getSource());
    json->addCString("CompileStatus", CompileStatusToString(shaderState.getCompileStatus()));
}

void SerializeShader(const gl::Context *context,
                     JsonSerializer *json,
                     GLuint id,
                     gl::Shader *shader)
{
    // Ensure deterministic compilation.
    shader->resolveCompile(context);

    GroupScope group(json, "Shader", id);
    SerializeShaderState(json, shader->getState());
    SerializeCompiledShaderState(json, shader->getCompiledState());
    json->addScalar("Handle", shader->getHandle().value);
    // TODO: implement MEC context validation only after all contexts have been initialized
    // http://anglebug.com/42266488
    // json->addScalar("RefCount", shader->getRefCount());
    json->addScalar("FlaggedForDeletion", shader->isFlaggedForDeletion());
    // Do not serialize mType because it is already serialized in SerializeCompiledShaderState.
    json->addString("InfoLogString", shader->getInfoLogString());
    // Do not serialize compiler resources string because it can vary between test modes.
}

void SerializeVariableLocationsVector(JsonSerializer *json,
                                      const std::string &group_name,
                                      const std::vector<gl::VariableLocation> &variableLocations)
{
    GroupScope group(json, group_name);
    for (size_t locIndex = 0; locIndex < variableLocations.size(); ++locIndex)
    {
        const gl::VariableLocation &variableLocation = variableLocations[locIndex];
        GroupScope vargroup(json, "Location", static_cast<int>(locIndex));
        json->addScalar("ArrayIndex", variableLocation.arrayIndex);
        json->addScalar("Index", variableLocation.index);
        json->addScalar("Ignored", variableLocation.ignored);
    }
}

void SerializeBlockMemberInfo(JsonSerializer *json, const sh::BlockMemberInfo &blockMemberInfo)
{
    GroupScope group(json, "BlockMemberInfo");
    json->addScalar("Offset", blockMemberInfo.offset);
    json->addScalar("Stride", blockMemberInfo.arrayStride);
    json->addScalar("MatrixStride", blockMemberInfo.matrixStride);
    json->addScalar("IsRowMajorMatrix", blockMemberInfo.isRowMajorMatrix);
    json->addScalar("TopLevelArrayStride", blockMemberInfo.topLevelArrayStride);
}

void SerializeBufferVariablesVector(JsonSerializer *json,
                                    const std::vector<gl::BufferVariable> &bufferVariables)
{
    for (const gl::BufferVariable &bufferVariable : bufferVariables)
    {
        GroupScope group(json, "BufferVariable");
        json->addString("Name", bufferVariable.name);
        json->addString("MappedName", bufferVariable.mappedName);

        json->addScalar("Type", bufferVariable.pod.type);
        json->addScalar("Precision", bufferVariable.pod.precision);
        json->addScalar("activeUseBits", bufferVariable.activeShaders().to_ulong());
        for (const gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            json->addScalar(
                gl::ShaderTypeToString(shaderType),
                bufferVariable.isActive(shaderType) ? bufferVariable.getId(shaderType) : 0);
        }

        json->addScalar("BufferIndex", bufferVariable.pod.bufferIndex);
        SerializeBlockMemberInfo(json, bufferVariable.pod.blockInfo);

        json->addScalar("TopLevelArraySize", bufferVariable.pod.topLevelArraySize);
        json->addScalar("basicTypeElementCount", bufferVariable.pod.basicTypeElementCount);
        json->addScalar("isArray", bufferVariable.pod.isArray);
    }
}

void SerializeProgramAliasedBindings(JsonSerializer *json,
                                     const gl::ProgramAliasedBindings &programAliasedBindings)
{
    for (const auto &programAliasedBinding : programAliasedBindings)
    {
        GroupScope group(json, programAliasedBinding.first);
        json->addScalar("Location", programAliasedBinding.second.location);
        json->addScalar("Aliased", programAliasedBinding.second.aliased);
    }
}

void SerializeProgramState(JsonSerializer *json, const gl::ProgramState &programState)
{
    json->addString("Label", programState.getLabel());
    json->addVectorOfStrings("TransformFeedbackVaryingNames",
                             programState.getTransformFeedbackVaryingNames());
    json->addScalar("BinaryRetrieveableHint", programState.hasBinaryRetrieveableHint());
    json->addScalar("Separable", programState.isSeparable());
    SerializeProgramAliasedBindings(json, programState.getUniformLocationBindings());

    const gl::ProgramExecutable &executable = programState.getExecutable();

    SerializeWorkGroupSize(json, executable.getComputeShaderLocalSize());
    SerializeUniformIndexToBufferBinding(
        json, executable.getUniformBlockIndexToBufferBindingForCapture());
    SerializeVariableLocationsVector(json, "UniformLocations", executable.getUniformLocations());
    SerializeBufferVariablesVector(json, executable.getBufferVariables());
    SerializeRange(json, executable.getAtomicCounterUniformRange());
    SerializeVariableLocationsVector(json, "SecondaryOutputLocations",
                                     executable.getSecondaryOutputLocations());
    json->addScalar("NumViews", executable.getNumViews());
    json->addScalar("DrawIDLocation", executable.getDrawIDLocation());
    json->addScalar("BaseVertexLocation", executable.getBaseVertexLocation());
    json->addScalar("BaseInstanceLocation", executable.getBaseInstanceLocation());
}

void SerializeProgramBindings(JsonSerializer *json, const gl::ProgramBindings &programBindings)
{
    for (const auto &programBinding : programBindings)
    {
        json->addScalar(programBinding.first, programBinding.second);
    }
}

template <typename T>
void SerializeUniformData(JsonSerializer *json,
                          const gl::Context *context,
                          const gl::ProgramExecutable &executable,
                          gl::UniformLocation loc,
                          GLenum type,
                          GLint size,
                          void (gl::ProgramExecutable::*getFunc)(const gl::Context *,
                                                                 gl::UniformLocation,
                                                                 T *) const)
{
    std::vector<T> uniformData(gl::VariableComponentCount(type) * size, 0);
    (executable.*getFunc)(context, loc, uniformData.data());
    json->addVector("Data", uniformData);
}

void SerializeProgram(JsonSerializer *json,
                      const gl::Context *context,
                      GLuint id,
                      gl::Program *program)
{
    // Ensure deterministic link.
    program->resolveLink(context);

    GroupScope group(json, "Program", id);

    std::vector<GLint> shaderHandles;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Shader *shader = program->getAttachedShader(shaderType);
        shaderHandles.push_back(shader ? shader->getHandle().value : 0);
    }
    json->addVector("Handle", shaderHandles);

    SerializeProgramState(json, program->getState());
    json->addScalar("IsValidated", program->isValidated());
    SerializeProgramBindings(json, program->getAttributeBindings());
    SerializeProgramAliasedBindings(json, program->getFragmentOutputLocations());
    SerializeProgramAliasedBindings(json, program->getFragmentOutputIndexes());
    json->addScalar("IsLinked", program->isLinked());
    json->addScalar("IsFlaggedForDeletion", program->isFlaggedForDeletion());
    // TODO: implement MEC context validation only after all contexts have been initialized
    // http://anglebug.com/42266488
    // json->addScalar("RefCount", program->getRefCount());
    json->addScalar("ID", program->id().value);

    const gl::ProgramExecutable &executable = program->getExecutable();

    // Serialize uniforms.
    {
        GroupScope uniformsGroup(json, "Uniforms");
        GLint uniformCount = static_cast<GLint>(executable.getUniforms().size());
        for (int uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
        {
            GroupScope uniformGroup(json, "Uniform", uniformIndex);

            constexpr GLsizei kMaxUniformNameLen = 1024;
            char uniformName[kMaxUniformNameLen] = {};
            GLint size                           = 0;
            GLenum type                          = GL_NONE;
            executable.getActiveUniform(uniformIndex, kMaxUniformNameLen, nullptr, &size, &type,
                                        uniformName);

            json->addCString("Name", uniformName);
            json->addScalar("Size", size);
            json->addCString("Type", gl::GLenumToString(gl::GLESEnum::AttributeType, type));

            const gl::UniformLocation loc = executable.getUniformLocation(uniformName);

            if (loc.value == -1)
            {
                continue;
            }

            switch (gl::VariableComponentType(type))
            {
                case GL_FLOAT:
                {
                    SerializeUniformData<GLfloat>(json, context, executable, loc, type, size,
                                                  &gl::ProgramExecutable::getUniformfv);
                    break;
                }
                case GL_BOOL:
                case GL_INT:
                {
                    SerializeUniformData<GLint>(json, context, executable, loc, type, size,
                                                &gl::ProgramExecutable::getUniformiv);
                    break;
                }
                case GL_UNSIGNED_INT:
                {
                    SerializeUniformData<GLuint>(json, context, executable, loc, type, size,
                                                 &gl::ProgramExecutable::getUniformuiv);
                    break;
                }
                default:
                    UNREACHABLE();
                    break;
            }
        }
    }
}

void SerializeImageDesc(JsonSerializer *json, size_t descIndex, const gl::ImageDesc &imageDesc)
{
    // Skip serializing unspecified image levels.
    if (imageDesc.size.empty())
    {
        return;
    }

    GroupScope group(json, "ImageDesc", static_cast<int>(descIndex));
    SerializeExtents(json, imageDesc.size);
    SerializeGLFormat(json, imageDesc.format);
    json->addScalar("Samples", imageDesc.samples);
    json->addScalar("FixesSampleLocations", imageDesc.fixedSampleLocations);
    json->addCString("InitState", InitStateToString(imageDesc.initState));
}

void SerializeTextureState(JsonSerializer *json, const gl::TextureState &textureState)
{
    json->addString("Type", ToString(textureState.getType()));
    SerializeSwizzleState(json, textureState.getSwizzleState());
    {
        GroupScope samplerStateGroup(json, "SamplerState");
        SerializeSamplerState(json, textureState.getSamplerState());
    }
    json->addCString("SRGB", SrgbOverrideToString(textureState.getSRGBOverride()));
    json->addScalar("BaseLevel", textureState.getBaseLevel());
    json->addScalar("MaxLevel", textureState.getMaxLevel());
    json->addScalar("DepthStencilTextureMode", textureState.getDepthStencilTextureMode());
    json->addScalar("BeenBoundAsImage", textureState.hasBeenBoundAsImage());
    json->addScalar("ImmutableFormat", textureState.getImmutableFormat());
    json->addScalar("ImmutableLevels", textureState.getImmutableLevels());
    json->addScalar("Usage", textureState.getUsage());
    SerializeRectangle(json, "Crop", textureState.getCrop());
    json->addScalar("GenerateMipmapHint", textureState.getGenerateMipmapHint());
    json->addCString("InitState", InitStateToString(textureState.getInitState()));
    json->addScalar("BoundBufferID", textureState.getBuffer().id().value);

    {
        GroupScope descGroup(json, "ImageDescs");
        const std::vector<gl::ImageDesc> &imageDescs = textureState.getImageDescs();
        for (size_t descIndex = 0; descIndex < imageDescs.size(); ++descIndex)
        {
            SerializeImageDesc(json, descIndex, imageDescs[descIndex]);
        }
    }
}

Result SerializeTextureData(JsonSerializer *json,
                            const gl::Context *context,
                            gl::Texture *texture,
                            ScratchBuffer *scratchBuffer)
{
    gl::ImageIndexIterator imageIter = gl::ImageIndexIterator::MakeGeneric(
        texture->getType(), texture->getBaseLevel(), texture->getMipmapMaxLevel() + 1,
        gl::ImageIndex::kEntireLevel, gl::ImageIndex::kEntireLevel);
    while (imageIter.hasNext())
    {
        gl::ImageIndex index = imageIter.next();

        // Skip serializing level data if the level index is out of range
        GLuint levelIndex = index.getLevelIndex();
        if (levelIndex > texture->getMipmapMaxLevel() || levelIndex < texture->getBaseLevel())
            continue;

        const gl::ImageDesc &desc = texture->getTextureState().getImageDesc(index);

        if (desc.size.empty())
            continue;

        const gl::InternalFormat &format = *desc.format.info;

        // Check for supported textures
        ASSERT(index.getType() == gl::TextureType::_2D || index.getType() == gl::TextureType::_3D ||
               index.getType() == gl::TextureType::_2DArray ||
               index.getType() == gl::TextureType::CubeMap ||
               index.getType() == gl::TextureType::CubeMapArray ||
               index.getType() == gl::TextureType::_2DMultisampleArray ||
               index.getType() == gl::TextureType::_2DMultisample ||
               index.getType() == gl::TextureType::External);

        GLenum glFormat = format.format;
        GLenum glType   = format.type;

        const gl::Extents size(desc.size.width, desc.size.height, desc.size.depth);
        gl::PixelPackState packState;
        packState.alignment = 1;

        GLuint endByte  = 0;
        bool unpackSize = format.computePackUnpackEndByte(glType, size, packState, true, &endByte);
        ASSERT(unpackSize);
        MemoryBuffer *texelsPtr = nullptr;
        ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                             scratchBuffer->getInitialized(endByte, &texelsPtr, 0));

        std::stringstream label;

        label << "Texels-Level" << index.getLevelIndex();
        if (imageIter.current().hasLayer())
        {
            label << "-Layer" << imageIter.current().getLayerIndex();
        }

        if (texture->getState().getInitState() == gl::InitState::Initialized)
        {
            if (format.compressed)
            {
                // TODO: Read back compressed data. http://anglebug.com/42264702
                json->addCString(label.str(), "compressed texel data");
            }
            else
            {
                ANGLE_TRY(texture->getTexImage(context, packState, nullptr, index.getTarget(),
                                               index.getLevelIndex(), glFormat, glType,
                                               texelsPtr->data()));
                json->addBlob(label.str(), texelsPtr->data(), texelsPtr->size());
            }
        }
        else
        {
            json->addCString(label.str(), "not initialized");
        }
    }
    return Result::Continue;
}

Result SerializeTexture(const gl::Context *context,
                        JsonSerializer *json,
                        ScratchBuffer *scratchBuffer,
                        gl::Texture *texture)
{
    GroupScope group(json, "Texture", texture->getId());

    // We serialize texture data first, to force the texture state to be initialized.
    if (texture->getType() != gl::TextureType::Buffer)
    {
        ANGLE_TRY(SerializeTextureData(json, context, texture, scratchBuffer));
    }

    SerializeTextureState(json, texture->getState());
    json->addString("Label", texture->getLabel());
    // FrameCapture can not serialize mBoundSurface and mBoundStream
    // because they are likely to change with each run
    return Result::Continue;
}

void SerializeVertexAttributeVector(JsonSerializer *json,
                                    const std::vector<gl::VertexAttribute> &vertexAttributes)
{
    for (size_t attribIndex = 0; attribIndex < vertexAttributes.size(); ++attribIndex)
    {
        GroupScope group(json, "VertexAttribute", static_cast<int>(attribIndex));
        const gl::VertexAttribute &vertexAttribute = vertexAttributes[attribIndex];
        json->addScalar("BindingIndex", vertexAttribute.bindingIndex);
        json->addScalar("Enabled", vertexAttribute.enabled);
        ASSERT(vertexAttribute.format);
        SerializeANGLEFormat(json, vertexAttribute.format);
        json->addScalar("RelativeOffset", vertexAttribute.relativeOffset);
        json->addScalar("VertexAttribArrayStride", vertexAttribute.vertexAttribArrayStride);
    }
}

void SerializeVertexBindingsVector(JsonSerializer *json,
                                   const std::vector<gl::VertexBinding> &vertexBindings)
{
    for (size_t bindingIndex = 0; bindingIndex < vertexBindings.size(); ++bindingIndex)
    {
        GroupScope group(json, "VertexBinding", static_cast<int>(bindingIndex));
        const gl::VertexBinding &vertexBinding = vertexBindings[bindingIndex];
        json->addScalar("Stride", vertexBinding.getStride());
        json->addScalar("Divisor", vertexBinding.getDivisor());
        json->addScalar("Offset", vertexBinding.getOffset());
        json->addScalar("BufferID", vertexBinding.getBuffer().id().value);
        json->addScalar("BoundAttributesMask", vertexBinding.getBoundAttributesMask().to_ulong());
    }
}

void SerializeVertexArrayState(JsonSerializer *json, const gl::VertexArrayState &vertexArrayState)
{
    json->addString("Label", vertexArrayState.getLabel());
    SerializeVertexAttributeVector(json, vertexArrayState.getVertexAttributes());
    if (vertexArrayState.getElementArrayBuffer())
    {
        json->addScalar("ElementArrayBufferID",
                        vertexArrayState.getElementArrayBuffer()->id().value);
    }
    else
    {
        json->addScalar("ElementArrayBufferID", 0);
    }
    SerializeVertexBindingsVector(json, vertexArrayState.getVertexBindings());
    json->addScalar("EnabledAttributesMask",
                    vertexArrayState.getEnabledAttributesMask().to_ulong());
    json->addScalar("VertexAttributesTypeMask",
                    vertexArrayState.getVertexAttributesTypeMask().to_ulong());
    json->addScalar("ClientMemoryAttribsMask",
                    vertexArrayState.getClientMemoryAttribsMask().to_ulong());
    json->addScalar("NullPointerClientMemoryAttribsMask",
                    vertexArrayState.getNullPointerClientMemoryAttribsMask().to_ulong());
}

void SerializeVertexArray(JsonSerializer *json, gl::VertexArray *vertexArray)
{
    GroupScope group(json, "VertexArray", vertexArray->id().value);
    SerializeVertexArrayState(json, vertexArray->getState());
    json->addScalar("BufferAccessValidationEnabled",
                    vertexArray->isBufferAccessValidationEnabled());
}

}  // namespace

Result SerializeContextToString(const gl::Context *context, std::string *stringOut)
{
    JsonSerializer json;
    json.startGroup("Context");

    SerializeContextState(&json, context->getState());
    ScratchBuffer scratchBuffer(1);
    {
        const gl::FramebufferManager &framebufferManager =
            context->getState().getFramebufferManagerForCapture();
        GroupScope framebufferGroup(&json, "FramebufferManager");
        for (const auto &framebuffer :
             gl::UnsafeResourceMapIter(framebufferManager.getResourcesForCapture()))
        {
            gl::Framebuffer *framebufferPtr = framebuffer.second;
            ANGLE_TRY(SerializeFramebuffer(context, &json, &scratchBuffer, framebufferPtr));
        }
    }
    {
        const gl::BufferManager &bufferManager = context->getState().getBufferManagerForCapture();
        GroupScope framebufferGroup(&json, "BufferManager");
        for (const auto &buffer : gl::UnsafeResourceMapIter(bufferManager.getResourcesForCapture()))
        {
            gl::Buffer *bufferPtr = buffer.second;
            ANGLE_TRY(SerializeBuffer(context, &json, &scratchBuffer, bufferPtr));
        }
    }
    {
        const gl::SamplerManager &samplerManager =
            context->getState().getSamplerManagerForCapture();
        GroupScope samplerGroup(&json, "SamplerManager");
        for (const auto &sampler :
             gl::UnsafeResourceMapIter(samplerManager.getResourcesForCapture()))
        {
            gl::Sampler *samplerPtr = sampler.second;
            SerializeSampler(&json, samplerPtr);
        }
    }
    {
        const gl::RenderbufferManager &renderbufferManager =
            context->getState().getRenderbufferManagerForCapture();
        GroupScope renderbufferGroup(&json, "RenderbufferManager");
        for (const auto &renderbuffer :
             gl::UnsafeResourceMapIter(renderbufferManager.getResourcesForCapture()))
        {
            gl::Renderbuffer *renderbufferPtr = renderbuffer.second;
            ANGLE_TRY(SerializeRenderbuffer(context, &json, &scratchBuffer, renderbufferPtr));
        }
    }
    const gl::ShaderProgramManager &shaderProgramManager =
        context->getState().getShaderProgramManagerForCapture();
    {
        const gl::ResourceMap<gl::Shader, gl::ShaderProgramID> &shaderManager =
            shaderProgramManager.getShadersForCapture();
        GroupScope shaderGroup(&json, "ShaderManager");
        for (const auto &shader : gl::UnsafeResourceMapIter(shaderManager))
        {
            GLuint id             = shader.first;
            gl::Shader *shaderPtr = shader.second;
            SerializeShader(context, &json, id, shaderPtr);
        }
    }
    {
        const gl::ResourceMap<gl::Program, gl::ShaderProgramID> &programManager =
            shaderProgramManager.getProgramsForCaptureAndPerf();
        GroupScope shaderGroup(&json, "ProgramManager");
        for (const auto &program : gl::UnsafeResourceMapIter(programManager))
        {
            GLuint id               = program.first;
            gl::Program *programPtr = program.second;
            SerializeProgram(&json, context, id, programPtr);
        }
    }
    {
        const gl::TextureManager &textureManager =
            context->getState().getTextureManagerForCapture();
        GroupScope shaderGroup(&json, "TextureManager");
        for (const auto &texture :
             gl::UnsafeResourceMapIter(textureManager.getResourcesForCapture()))
        {
            gl::Texture *texturePtr = texture.second;
            ANGLE_TRY(SerializeTexture(context, &json, &scratchBuffer, texturePtr));
        }
    }
    {
        const gl::VertexArrayMap &vertexArrayMap = context->getVertexArraysForCapture();
        GroupScope shaderGroup(&json, "VertexArrayMap");
        for (const auto &vertexArray : gl::UnsafeResourceMapIter(vertexArrayMap))
        {
            gl::VertexArray *vertexArrayPtr = vertexArray.second;
            SerializeVertexArray(&json, vertexArrayPtr);
        }
    }
    json.endGroup();

    *stringOut = json.data();

    scratchBuffer.clear();
    return Result::Continue;
}

}  // namespace angle
