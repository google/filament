//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_utils.h:
//    Declares utilities functions that create Metal shaders, convert from angle enums
//    to Metal enums and so on.
//

#ifndef LIBANGLE_RENDERER_METAL_MTL_UTILS_H_
#define LIBANGLE_RENDERER_METAL_MTL_UTILS_H_

#import <Metal/Metal.h>

#include "angle_gl.h"
#include "common/MemoryBuffer.h"
#include "common/PackedEnums.h"
#include "libANGLE/Context.h"
#include "libANGLE/Texture.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"

namespace rx
{

class ContextMtl;

void StartFrameCapture(id<MTLDevice> metalDevice, id<MTLCommandQueue> metalCmdQueue);
void StartFrameCapture(ContextMtl *context);
void StopFrameCapture();

namespace mtl
{

enum class StagingPurpose
{
    Initialization,
    Upload
};

bool PreferStagedTextureUploads(const gl::Context *context,
                                const TextureRef &texture,
                                const Format &textureObjFormat,
                                const StagingPurpose purpose);

// Initialize texture content to black.
angle::Result InitializeTextureContents(const gl::Context *context,
                                        const TextureRef &texture,
                                        const Format &textureObjFormat,
                                        const ImageNativeIndex &index);
// Same as above but using GPU clear operation instead of CPU.forma
// - channelsToInit parameter controls which channels will get their content initialized.
angle::Result InitializeTextureContentsGPU(const gl::Context *context,
                                           const TextureRef &texture,
                                           const Format &textureObjFormat,
                                           const ImageNativeIndex &index,
                                           MTLColorWriteMask channelsToInit);

// Same as above but for a depth/stencil texture.
angle::Result InitializeDepthStencilTextureContentsGPU(const gl::Context *context,
                                                       const TextureRef &texture,
                                                       const Format &textureObjFormat,
                                                       const ImageNativeIndex &index);

// Unified texture's per slice/depth texel reading function
angle::Result ReadTexturePerSliceBytes(const gl::Context *context,
                                       const TextureRef &texture,
                                       size_t bytesPerRow,
                                       const gl::Rectangle &fromRegion,
                                       const MipmapNativeLevel &mipLevel,
                                       uint32_t sliceOrDepth,
                                       uint8_t *dataOut);

angle::Result ReadTexturePerSliceBytesToBuffer(const gl::Context *context,
                                               const TextureRef &texture,
                                               size_t bytesPerRow,
                                               const gl::Rectangle &fromRegion,
                                               const MipmapNativeLevel &mipLevel,
                                               uint32_t sliceOrDepth,
                                               uint32_t dstOffset,
                                               const BufferRef &dstBuffer);

MTLViewport GetViewport(const gl::Rectangle &rect, double znear = 0, double zfar = 1);
MTLViewport GetViewportFlipY(const gl::Rectangle &rect,
                             NSUInteger screenHeight,
                             double znear = 0,
                             double zfar  = 1);
MTLViewport GetViewport(const gl::Rectangle &rect,
                        NSUInteger screenHeight,
                        bool flipY,
                        double znear = 0,
                        double zfar  = 1);
MTLScissorRect GetScissorRect(const gl::Rectangle &rect,
                              NSUInteger screenHeight = 0,
                              bool flipY              = false);

uint32_t GetDeviceVendorId(id<MTLDevice> metalDevice);

AutoObjCPtr<id<MTLLibrary>> CreateShaderLibrary(
    id<MTLDevice> metalDevice,
    std::string_view source,
    const std::map<std::string, std::string> &substitutionDictionary,
    bool disableFastMath,
    bool usesInvariance,
    AutoObjCPtr<NSError *> *error);

AutoObjCPtr<id<MTLLibrary>> CreateShaderLibraryFromBinary(id<MTLDevice> metalDevice,
                                                          const uint8_t *data,
                                                          size_t length,
                                                          AutoObjCPtr<NSError *> *error);

AutoObjCPtr<id<MTLLibrary>> CreateShaderLibraryFromStaticBinary(id<MTLDevice> metalDevice,
                                                                const uint8_t *data,
                                                                size_t length,
                                                                AutoObjCPtr<NSError *> *error);

// Compiles a shader library into a metallib file, returning the path to it.
std::string CompileShaderLibraryToFile(const std::string &source,
                                       const std::map<std::string, std::string> &macros,
                                       bool disableFastMath,
                                       bool usesInvariance);

bool SupportsAppleGPUFamily(id<MTLDevice> device, uint8_t appleFamily);

bool SupportsMacGPUFamily(id<MTLDevice> device, uint8_t macFamily);

// Need to define invalid enum value since Metal doesn't define it
constexpr MTLTextureType MTLTextureTypeInvalid = static_cast<MTLTextureType>(NSUIntegerMax);
static_assert(sizeof(MTLTextureType) == sizeof(NSUInteger),
              "MTLTextureType is supposed to be based on NSUInteger");

constexpr MTLPrimitiveType MTLPrimitiveTypeInvalid = static_cast<MTLPrimitiveType>(NSUIntegerMax);
static_assert(sizeof(MTLPrimitiveType) == sizeof(NSUInteger),
              "MTLPrimitiveType is supposed to be based on NSUInteger");

constexpr MTLIndexType MTLIndexTypeInvalid = static_cast<MTLIndexType>(NSUIntegerMax);
static_assert(sizeof(MTLIndexType) == sizeof(NSUInteger),
              "MTLIndexType is supposed to be based on NSUInteger");

MTLTextureType GetTextureType(gl::TextureType glType);

MTLSamplerMinMagFilter GetFilter(GLenum filter);
MTLSamplerMipFilter GetMipmapFilter(GLenum filter);
MTLSamplerAddressMode GetSamplerAddressMode(GLenum wrap);

MTLBlendFactor GetBlendFactor(gl::BlendFactorType factor);
MTLBlendOperation GetBlendOp(gl::BlendEquationType op);

MTLCompareFunction GetCompareFunc(GLenum func);
MTLStencilOperation GetStencilOp(GLenum op);

MTLWinding GetFrontfaceWinding(GLenum frontFaceMode, bool invert);

MTLPrimitiveTopologyClass GetPrimitiveTopologyClass(gl::PrimitiveMode mode);
MTLPrimitiveType GetPrimitiveType(gl::PrimitiveMode mode);
MTLIndexType GetIndexType(gl::DrawElementsType type);

MTLTextureSwizzle GetTextureSwizzle(GLenum swizzle);

// Get color write mask for a specified format. Some formats such as RGB565 doesn't have alpha
// channel but is emulated by a RGBA8 format, we need to disable alpha write for this format.
// - emulatedChannelsOut: if the format is emulated, this pointer will store a true value.
MTLColorWriteMask GetEmulatedColorWriteMask(const mtl::Format &mtlFormat,
                                            bool *emulatedChannelsOut);
MTLColorWriteMask GetEmulatedColorWriteMask(const mtl::Format &mtlFormat);
bool IsFormatEmulated(const mtl::Format &mtlFormat);
size_t EstimateTextureSizeInBytes(const mtl::Format &mtlFormat,
                                  size_t width,
                                  size_t height,
                                  size_t depth,
                                  size_t sampleCount,
                                  size_t numMips);

NSUInteger GetMaxRenderTargetSizeForDeviceInBytes(const mtl::ContextDevice &device);
NSUInteger GetMaxNumberOfRenderTargetsForDevice(const mtl::ContextDevice &device);
bool DeviceHasMaximumRenderTargetSize(id<MTLDevice> device);

// Useful to set clear color for texture originally having no alpha in GL, but backend's format
// has alpha channel.
MTLClearColor EmulatedAlphaClearColor(MTLClearColor color, MTLColorWriteMask colorMask);

NSUInteger ComputeTotalSizeUsedForMTLRenderPassDescriptor(const mtl::RenderPassDesc &descriptor,
                                                          const Context *context,
                                                          const mtl::ContextDevice &device);

NSUInteger ComputeTotalSizeUsedForMTLRenderPipelineDescriptor(
    const MTLRenderPipelineDescriptor *descriptor,
    const Context *context,
    const mtl::ContextDevice &device);

gl::Box MTLRegionToGLBox(const MTLRegion &mtlRegion);

MipmapNativeLevel GetNativeMipLevel(GLuint level, GLuint base);
GLuint GetGLMipLevel(const MipmapNativeLevel &nativeLevel, GLuint base);

angle::Result TriangleFanBoundCheck(ContextMtl *context, size_t numTris);

angle::Result GetTriangleFanIndicesCount(ContextMtl *context,
                                         GLsizei vetexCount,
                                         uint32_t *numElemsOut);

angle::Result CreateMslShader(ContextMtl *context,
                              id<MTLLibrary> shaderLib,
                              NSString *shaderName,
                              MTLFunctionConstantValues *funcConstants,
                              AutoObjCPtr<id<MTLFunction>> *shaderOut);

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_MTL_UTILS_H_ */
