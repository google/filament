
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererD3D.h: Defines a back-end specific class for the DirectX renderer.

#ifndef LIBANGLE_RENDERER_D3D_RENDERERD3D_H_
#define LIBANGLE_RENDERER_D3D_RENDERERD3D_H_

#include <array>

#include "common/Color.h"
#include "common/MemoryBuffer.h"
#include "common/debug.h"
#include "libANGLE/Device.h"
#include "libANGLE/State.h"
#include "libANGLE/Version.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/serial_utils.h"
#include "platform/autogen/FeaturesD3D_autogen.h"

namespace egl
{
class ConfigSet;
}

namespace gl
{
class ErrorSet;
class FramebufferState;
class InfoLog;
class Texture;
struct LinkedVarying;
}  // namespace gl

namespace rx
{
class ContextImpl;
struct D3DUniform;
struct D3DVarying;
class EGLImageD3D;
class FramebufferImpl;
class ImageD3D;
class IndexBuffer;
class NativeWindowD3D;
class ProgramD3D;
class ProgramExecutableD3D;
class RenderTargetD3D;
class ShaderExecutableD3D;
class SwapChainD3D;
class TextureStorage;
struct TranslatedIndexData;
class UniformStorageD3D;
class VertexBuffer;

struct DeviceIdentifier
{
    UINT VendorId;
    UINT DeviceId;
    UINT SubSysId;
    UINT Revision;
    UINT FeatureLevel;
};

enum RendererClass
{
    RENDERER_D3D11,
    RENDERER_D3D9
};

struct BindFlags
{
    bool renderTarget    = false;
    bool unorderedAccess = false;
    static BindFlags RenderTarget()
    {
        BindFlags flags;
        flags.renderTarget = true;
        return flags;
    }
    static BindFlags UnorderedAccess()
    {
        BindFlags flags;
        flags.unorderedAccess = true;
        return flags;
    }
};

// A d3d::Context wraps error handling.
namespace d3d
{
class Context : angle::NonCopyable
{
  public:
    Context() {}
    virtual ~Context() {}

    virtual void handleResult(HRESULT hr,
                              const char *message,
                              const char *file,
                              const char *function,
                              unsigned int line) = 0;
};
}  // namespace d3d

// ANGLE_TRY for HRESULT errors.
#define ANGLE_TRY_HR(CONTEXT, EXPR, MESSAGE)                                                     \
    do                                                                                           \
    {                                                                                            \
        auto ANGLE_LOCAL_VAR = (EXPR);                                                           \
        if (ANGLE_UNLIKELY(FAILED(ANGLE_LOCAL_VAR)))                                             \
        {                                                                                        \
            CONTEXT->handleResult(ANGLE_LOCAL_VAR, MESSAGE, __FILE__, ANGLE_FUNCTION, __LINE__); \
            return angle::Result::Stop;                                                          \
        }                                                                                        \
    } while (0)

#define ANGLE_CHECK_HR(CONTEXT, EXPR, MESSAGE, ERROR)                                  \
    do                                                                                 \
    {                                                                                  \
        if (ANGLE_UNLIKELY(!(EXPR)))                                                   \
        {                                                                              \
            CONTEXT->handleResult(ERROR, MESSAGE, __FILE__, ANGLE_FUNCTION, __LINE__); \
            return angle::Result::Stop;                                                \
        }                                                                              \
    } while (0)

#define ANGLE_HR_UNREACHABLE(context) \
    UNREACHABLE();                    \
    ANGLE_CHECK_HR(context, false, "Unreachble code reached.", E_FAIL)

// Check if the device is lost every 10 failures to get the query data
constexpr unsigned int kPollingD3DDeviceLostCheckFrequency = 10;

// Useful for unit testing
class BufferFactoryD3D : angle::NonCopyable
{
  public:
    BufferFactoryD3D() {}
    virtual ~BufferFactoryD3D() {}

    virtual VertexBuffer *createVertexBuffer() = 0;
    virtual IndexBuffer *createIndexBuffer()   = 0;

    // TODO(jmadill): add VertexFormatCaps
    virtual VertexConversionType getVertexConversionType(angle::FormatID vertexFormatID) const = 0;
    virtual GLenum getVertexComponentType(angle::FormatID vertexFormatID) const                = 0;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using this
    // function.
    virtual angle::Result getVertexSpaceRequired(const gl::Context *context,
                                                 const gl::VertexAttribute &attrib,
                                                 const gl::VertexBinding &binding,
                                                 size_t count,
                                                 GLsizei instances,
                                                 GLuint baseInstance,
                                                 unsigned int *bytesRequiredOut) const = 0;
};

using AttribIndexArray = gl::AttribArray<int>;

class RendererD3D : public BufferFactoryD3D
{
  public:
    explicit RendererD3D(egl::Display *display);
    ~RendererD3D() override;

    virtual egl::Error initialize() = 0;

    virtual egl::ConfigSet generateConfigs()                                            = 0;
    virtual void generateDisplayExtensions(egl::DisplayExtensions *outExtensions) const = 0;

    virtual ContextImpl *createContext(const gl::State &state, gl::ErrorSet *errorSet) = 0;

    virtual std::string getRendererDescription() const                  = 0;
    virtual std::string getVendorString() const                         = 0;
    virtual std::string getVersionString(bool includeFullVersion) const = 0;

    virtual int getMinorShaderModel() const          = 0;
    virtual std::string getShaderModelSuffix() const = 0;

    // Direct3D Specific methods
    virtual DeviceIdentifier getAdapterIdentifier() const = 0;

    virtual bool isValidNativeWindow(EGLNativeWindowType window) const                  = 0;
    virtual NativeWindowD3D *createNativeWindow(EGLNativeWindowType window,
                                                const egl::Config *config,
                                                const egl::AttributeMap &attribs) const = 0;

    virtual SwapChainD3D *createSwapChain(NativeWindowD3D *nativeWindow,
                                          HANDLE shareHandle,
                                          IUnknown *d3dTexture,
                                          GLenum backBufferFormat,
                                          GLenum depthBufferFormat,
                                          EGLint orientation,
                                          EGLint samples)                          = 0;
    virtual egl::Error getD3DTextureInfo(const egl::Config *configuration,
                                         IUnknown *d3dTexture,
                                         const egl::AttributeMap &attribs,
                                         EGLint *width,
                                         EGLint *height,
                                         GLsizei *samples,
                                         gl::Format *glFormat,
                                         const angle::Format **angleFormat,
                                         UINT *arraySlice) const                   = 0;
    virtual egl::Error validateShareHandle(const egl::Config *config,
                                           HANDLE shareHandle,
                                           const egl::AttributeMap &attribs) const = 0;

    virtual int getMajorShaderModel() const = 0;

    virtual void setGlobalDebugAnnotator() = 0;

    const angle::FeaturesD3D &getFeatures() const;

    // Pixel operations
    virtual angle::Result copyImage2D(const gl::Context *context,
                                      const gl::Framebuffer *framebuffer,
                                      const gl::Rectangle &sourceRect,
                                      GLenum destFormat,
                                      const gl::Offset &destOffset,
                                      TextureStorage *storage,
                                      GLint level)      = 0;
    virtual angle::Result copyImageCube(const gl::Context *context,
                                        const gl::Framebuffer *framebuffer,
                                        const gl::Rectangle &sourceRect,
                                        GLenum destFormat,
                                        const gl::Offset &destOffset,
                                        TextureStorage *storage,
                                        gl::TextureTarget target,
                                        GLint level)    = 0;
    virtual angle::Result copyImage3D(const gl::Context *context,
                                      const gl::Framebuffer *framebuffer,
                                      const gl::Rectangle &sourceRect,
                                      GLenum destFormat,
                                      const gl::Offset &destOffset,
                                      TextureStorage *storage,
                                      GLint level)      = 0;
    virtual angle::Result copyImage2DArray(const gl::Context *context,
                                           const gl::Framebuffer *framebuffer,
                                           const gl::Rectangle &sourceRect,
                                           GLenum destFormat,
                                           const gl::Offset &destOffset,
                                           TextureStorage *storage,
                                           GLint level) = 0;

    virtual angle::Result copyTexture(const gl::Context *context,
                                      const gl::Texture *source,
                                      GLint sourceLevel,
                                      gl::TextureTarget srcTarget,
                                      const gl::Box &sourceBox,
                                      GLenum destFormat,
                                      GLenum destType,
                                      const gl::Offset &destOffset,
                                      TextureStorage *storage,
                                      gl::TextureTarget destTarget,
                                      GLint destLevel,
                                      bool unpackFlipY,
                                      bool unpackPremultiplyAlpha,
                                      bool unpackUnmultiplyAlpha) = 0;
    virtual angle::Result copyCompressedTexture(const gl::Context *context,
                                                const gl::Texture *source,
                                                GLint sourceLevel,
                                                TextureStorage *storage,
                                                GLint destLevel)  = 0;

    // RenderTarget creation
    virtual angle::Result createRenderTarget(const gl::Context *context,
                                             int width,
                                             int height,
                                             GLenum format,
                                             GLsizei samples,
                                             RenderTargetD3D **outRT)     = 0;
    virtual angle::Result createRenderTargetCopy(const gl::Context *context,
                                                 RenderTargetD3D *source,
                                                 RenderTargetD3D **outRT) = 0;

    // Shader operations
    virtual angle::Result loadExecutable(d3d::Context *context,
                                         const uint8_t *function,
                                         size_t length,
                                         gl::ShaderType type,
                                         const std::vector<D3DVarying> &streamOutVaryings,
                                         bool separatedOutputBuffers,
                                         ShaderExecutableD3D **outExecutable)      = 0;
    virtual angle::Result compileToExecutable(d3d::Context *context,
                                              gl::InfoLog &infoLog,
                                              const std::string &shaderHLSL,
                                              gl::ShaderType type,
                                              const std::vector<D3DVarying> &streamOutVaryings,
                                              bool separatedOutputBuffers,
                                              const CompilerWorkaroundsD3D &workarounds,
                                              ShaderExecutableD3D **outExectuable) = 0;
    virtual angle::Result ensureHLSLCompilerInitialized(d3d::Context *context)     = 0;

    virtual UniformStorageD3D *createUniformStorage(size_t storageSize) = 0;

    // Image operations
    virtual ImageD3D *createImage() = 0;
    virtual ExternalImageSiblingImpl *createExternalImageSibling(
        const gl::Context *context,
        EGLenum target,
        EGLClientBuffer buffer,
        const egl::AttributeMap &attribs)                                              = 0;
    virtual angle::Result generateMipmap(const gl::Context *context,
                                         ImageD3D *dest,
                                         ImageD3D *source)                             = 0;
    virtual angle::Result generateMipmapUsingD3D(const gl::Context *context,
                                                 TextureStorage *storage,
                                                 const gl::TextureState &textureState) = 0;
    virtual angle::Result copyImage(const gl::Context *context,
                                    ImageD3D *dest,
                                    ImageD3D *source,
                                    const gl::Box &sourceBox,
                                    const gl::Offset &destOffset,
                                    bool unpackFlipY,
                                    bool unpackPremultiplyAlpha,
                                    bool unpackUnmultiplyAlpha)                        = 0;
    virtual TextureStorage *createTextureStorage2D(SwapChainD3D *swapChain,
                                                   const std::string &label)           = 0;
    virtual TextureStorage *createTextureStorageEGLImage(EGLImageD3D *eglImage,
                                                         RenderTargetD3D *renderTargetD3D,
                                                         const std::string &label)     = 0;
    virtual TextureStorage *createTextureStorageBuffer(
        const gl::OffsetBindingPointer<gl::Buffer> &buffer,
        GLenum internalFormat,
        const std::string &label) = 0;
    virtual TextureStorage *createTextureStorageExternal(
        egl::Stream *stream,
        const egl::Stream::GLTextureDescription &desc,
        const std::string &label)                                                            = 0;
    virtual TextureStorage *createTextureStorage2D(GLenum internalformat,
                                                   BindFlags bindFlags,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   int levels,
                                                   const std::string &label,
                                                   bool hintLevelZeroOnly)                   = 0;
    virtual TextureStorage *createTextureStorageCube(GLenum internalformat,
                                                     BindFlags bindFlags,
                                                     int size,
                                                     int levels,
                                                     bool hintLevelZeroOnly,
                                                     const std::string &label)               = 0;
    virtual TextureStorage *createTextureStorage3D(GLenum internalformat,
                                                   BindFlags bindFlags,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLsizei depth,
                                                   int levels,
                                                   const std::string &label)                 = 0;
    virtual TextureStorage *createTextureStorage2DArray(GLenum internalformat,
                                                        BindFlags bindFlags,
                                                        GLsizei width,
                                                        GLsizei height,
                                                        GLsizei depth,
                                                        int levels,
                                                        const std::string &label)            = 0;
    virtual TextureStorage *createTextureStorage2DMultisample(GLenum internalformat,
                                                              GLsizei width,
                                                              GLsizei height,
                                                              int levels,
                                                              int samples,
                                                              bool fixedSampleLocations,
                                                              const std::string &label)      = 0;
    virtual TextureStorage *createTextureStorage2DMultisampleArray(GLenum internalformat,
                                                                   GLsizei width,
                                                                   GLsizei height,
                                                                   GLsizei depth,
                                                                   int levels,
                                                                   int samples,
                                                                   bool fixedSampleLocations,
                                                                   const std::string &label) = 0;

    // Buffer-to-texture and Texture-to-buffer copies
    virtual bool supportsFastCopyBufferToTexture(GLenum internalFormat) const = 0;
    virtual angle::Result fastCopyBufferToTexture(const gl::Context *context,
                                                  const gl::PixelUnpackState &unpack,
                                                  gl::Buffer *unpackBuffer,
                                                  unsigned int offset,
                                                  RenderTargetD3D *destRenderTarget,
                                                  GLenum destinationFormat,
                                                  GLenum sourcePixelsType,
                                                  const gl::Box &destArea)    = 0;

    // Device lost
    gl::GraphicsResetStatus getResetStatus();
    void notifyDeviceLost();
    virtual bool resetDevice()          = 0;
    virtual bool testDeviceLost()       = 0;
    virtual bool testDeviceResettable() = 0;

    virtual RendererClass getRendererClass() const = 0;
    virtual void *getD3DDevice()                   = 0;

    GLint64 getTimestamp();

    virtual angle::Result clearRenderTarget(const gl::Context *context,
                                            RenderTargetD3D *renderTarget,
                                            const gl::ColorF &clearColorValue,
                                            const float clearDepthValue,
                                            const unsigned int clearStencilValue) = 0;

    virtual DeviceImpl *createEGLDevice() = 0;

    bool presentPathFastEnabled() const { return mPresentPathFastEnabled; }

    // Stream creation
    virtual StreamProducerImpl *createStreamProducerD3DTexture(
        egl::Stream::ConsumerType consumerType,
        const egl::AttributeMap &attribs) = 0;

    const gl::Caps &getNativeCaps() const;
    const gl::TextureCapsMap &getNativeTextureCaps() const;
    const gl::Extensions &getNativeExtensions() const;
    const gl::Limitations &getNativeLimitations() const;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const;
    virtual void initializeFrontendFeatures(angle::FrontendFeatures *features) const = 0;

    // Necessary hack for default framebuffers in D3D.
    virtual FramebufferImpl *createDefaultFramebuffer(const gl::FramebufferState &state) = 0;

    virtual gl::Version getMaxSupportedESVersion() const  = 0;
    virtual gl::Version getMaxConformantESVersion() const = 0;

    angle::Result initRenderTarget(const gl::Context *context, RenderTargetD3D *renderTarget);

    virtual angle::Result getIncompleteTexture(const gl::Context *context,
                                               gl::TextureType type,
                                               gl::Texture **textureOut) = 0;

    UniqueSerial generateSerial();

    virtual bool canSelectViewInVertexShader() const = 0;

    egl::Display *getDisplay() const { return mDisplay; }

  protected:
    virtual bool getLUID(LUID *adapterLuid) const                              = 0;
    virtual void generateCaps(gl::Caps *outCaps,
                              gl::TextureCapsMap *outTextureCaps,
                              gl::Extensions *outExtensions,
                              gl::Limitations *outLimitations,
                              ShPixelLocalStorageOptions *outPLSOptions) const = 0;

    bool skipDraw(const gl::State &glState, gl::PrimitiveMode drawMode);

    egl::Display *mDisplay;

    bool mPresentPathFastEnabled;

  private:
    void ensureCapsInitialized() const;

    virtual void initializeFeatures(angle::FeaturesD3D *features) const = 0;

    mutable bool mCapsInitialized;
    mutable gl::Caps mNativeCaps;
    mutable gl::TextureCapsMap mNativeTextureCaps;
    mutable gl::Extensions mNativeExtensions;
    mutable gl::Limitations mNativeLimitations;
    mutable ShPixelLocalStorageOptions mNativePLSOptions;

    mutable bool mFeaturesInitialized;
    mutable angle::FeaturesD3D mFeatures;

    bool mDeviceLost;

    UniqueSerialFactory mSerialFactory;
};

unsigned int GetBlendSampleMask(const gl::State &glState, int samples);
GLenum DefaultGLErrorCode(HRESULT hr);

// Define stubs so we don't need to include D3D9/D3D11 headers directly.
RendererD3D *CreateRenderer11(egl::Display *display);
RendererD3D *CreateRenderer9(egl::Display *display);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_RENDERERD3D_H_
