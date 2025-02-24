//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramExecutableD3D.h: Implementation of ProgramExecutableImpl.

#ifndef LIBANGLE_RENDERER_D3D_PROGRAMEXECUTABLED3D_H_
#define LIBANGLE_RENDERER_D3D_PROGRAMEXECUTABLED3D_H_

#include "compiler/translator/hlsl/blocklayoutHLSL.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ProgramExecutableImpl.h"
#include "libANGLE/renderer/d3d/DynamicHLSL.h"

namespace rx
{
class RendererD3D;
class UniformStorageD3D;
class ShaderExecutableD3D;

#if !defined(ANGLE_COMPILE_OPTIMIZATION_LEVEL)
// WARNING: D3DCOMPILE_OPTIMIZATION_LEVEL3 may lead to a DX9 shader compiler hang.
// It should only be used selectively to work around specific bugs.
#    define ANGLE_COMPILE_OPTIMIZATION_LEVEL D3DCOMPILE_OPTIMIZATION_LEVEL1
#endif

enum class HLSLRegisterType : uint8_t
{
    None                = 0,
    Texture             = 1,
    UnorderedAccessView = 2
};

// Helper struct representing a single shader uniform
// TODO(jmadill): Make uniform blocks shared between all programs, so we don't need separate
// register indices.
struct D3DUniform : private angle::NonCopyable
{
    D3DUniform(GLenum type,
               HLSLRegisterType reg,
               const std::string &nameIn,
               const std::vector<unsigned int> &arraySizesIn,
               bool defaultBlock);
    ~D3DUniform();

    bool isSampler() const;
    bool isImage() const;
    bool isImage2D() const;
    bool isArray() const { return !arraySizes.empty(); }
    unsigned int getArraySizeProduct() const;
    bool isReferencedByShader(gl::ShaderType shaderType) const;

    const uint8_t *firstNonNullData() const;
    const uint8_t *getDataPtrToElement(size_t elementIndex) const;

    // Duplicated from the GL layer
    const gl::UniformTypeInfo &typeInfo;
    std::string name;  // Names of arrays don't include [0], unlike at the GL layer.
    std::vector<unsigned int> arraySizes;

    // Pointer to a system copies of the data. Separate pointers for each uniform storage type.
    gl::ShaderMap<uint8_t *> mShaderData;

    // Register information.
    HLSLRegisterType regType;
    gl::ShaderMap<unsigned int> mShaderRegisterIndexes;
    unsigned int registerCount;

    // Register "elements" are used for uniform structs in ES3, to appropriately identify single
    // uniforms
    // inside aggregate types, which are packed according C-like structure rules.
    unsigned int registerElement;

    // Special buffer for sampler values.
    std::vector<GLint> mSamplerData;
};

struct D3DInterfaceBlock
{
    D3DInterfaceBlock();
    D3DInterfaceBlock(const D3DInterfaceBlock &other);

    bool activeInShader(gl::ShaderType shaderType) const
    {
        return mShaderRegisterIndexes[shaderType] != GL_INVALID_INDEX;
    }

    gl::ShaderMap<unsigned int> mShaderRegisterIndexes;
};

struct D3DUniformBlock : D3DInterfaceBlock
{
    D3DUniformBlock();
    D3DUniformBlock(const D3DUniformBlock &other);

    gl::ShaderMap<bool> mUseStructuredBuffers;
    gl::ShaderMap<unsigned int> mByteWidths;
    gl::ShaderMap<unsigned int> mStructureByteStrides;
};

struct ShaderStorageBlock
{
    std::string name;
    unsigned int arraySize     = 0;
    unsigned int registerIndex = 0;
};

struct D3DUBOCache
{
    unsigned int registerIndex;
    int binding;
};

struct D3DUBOCacheUseSB : D3DUBOCache
{
    unsigned int byteWidth;
    unsigned int structureByteStride;
};

struct D3DVarying final
{
    D3DVarying();
    D3DVarying(const std::string &semanticNameIn,
               unsigned int semanticIndexIn,
               unsigned int componentCountIn,
               unsigned int outputSlotIn);

    D3DVarying(const D3DVarying &)            = default;
    D3DVarying &operator=(const D3DVarying &) = default;

    std::string semanticName;
    unsigned int semanticIndex;
    unsigned int componentCount;
    unsigned int outputSlot;
};

using D3DUniformMap = std::map<std::string, D3DUniform *>;

class D3DVertexExecutable
{
  public:
    enum HLSLAttribType
    {
        FLOAT,
        UNSIGNED_INT,
        SIGNED_INT,
    };

    typedef std::vector<HLSLAttribType> Signature;

    D3DVertexExecutable(const gl::InputLayout &inputLayout,
                        const Signature &signature,
                        ShaderExecutableD3D *shaderExecutable);
    ~D3DVertexExecutable();

    bool matchesSignature(const Signature &signature) const;
    static void getSignature(RendererD3D *renderer,
                             const gl::InputLayout &inputLayout,
                             Signature *signatureOut);

    const gl::InputLayout &inputs() const { return mInputs; }
    const Signature &signature() const { return mSignature; }
    ShaderExecutableD3D *shaderExecutable() const { return mShaderExecutable; }

  private:
    static HLSLAttribType GetAttribType(GLenum type);

    gl::InputLayout mInputs;
    Signature mSignature;
    ShaderExecutableD3D *mShaderExecutable;
};

class D3DPixelExecutable
{
  public:
    D3DPixelExecutable(const std::vector<GLenum> &outputSignature,
                       const gl::ImageUnitTextureTypeMap &image2DSignature,
                       ShaderExecutableD3D *shaderExecutable);
    ~D3DPixelExecutable();

    bool matchesSignature(const std::vector<GLenum> &outputSignature,
                          const gl::ImageUnitTextureTypeMap &image2DSignature) const
    {
        return mOutputSignature == outputSignature && mImage2DSignature == image2DSignature;
    }

    const std::vector<GLenum> &outputSignature() const { return mOutputSignature; }

    const gl::ImageUnitTextureTypeMap &image2DSignature() const { return mImage2DSignature; }

    ShaderExecutableD3D *shaderExecutable() const { return mShaderExecutable; }

  private:
    const std::vector<GLenum> mOutputSignature;
    const gl::ImageUnitTextureTypeMap mImage2DSignature;
    ShaderExecutableD3D *mShaderExecutable;
};

class D3DComputeExecutable
{
  public:
    D3DComputeExecutable(const gl::ImageUnitTextureTypeMap &signature,
                         std::unique_ptr<ShaderExecutableD3D> shaderExecutable);
    ~D3DComputeExecutable();

    bool matchesSignature(const gl::ImageUnitTextureTypeMap &signature) const
    {
        return mSignature == signature;
    }

    const gl::ImageUnitTextureTypeMap &signature() const { return mSignature; }
    ShaderExecutableD3D *shaderExecutable() const { return mShaderExecutable.get(); }

  private:
    gl::ImageUnitTextureTypeMap mSignature;
    std::unique_ptr<ShaderExecutableD3D> mShaderExecutable;
};

struct D3DSampler
{
    D3DSampler();

    bool active;
    GLint logicalTextureUnit;
    gl::TextureType textureType;
};

struct D3DImage
{
    D3DImage();
    bool active;
    GLint logicalImageUnit;
};

class ProgramExecutableD3D : public ProgramExecutableImpl
{
  public:
    ProgramExecutableD3D(const gl::ProgramExecutable *executable);
    ~ProgramExecutableD3D() override;

    void destroy(const gl::Context *context) override;

    void setUniform1fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform2fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform3fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform4fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform1iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform2iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform3iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform4iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform1uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform2uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform3uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform4uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniformMatrix2fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix3fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix4fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix2x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix2x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;

    void getUniformfv(const gl::Context *context, GLint location, GLfloat *params) const override;
    void getUniformiv(const gl::Context *context, GLint location, GLint *params) const override;
    void getUniformuiv(const gl::Context *context, GLint location, GLuint *params) const override;

    void updateCachedInputLayoutFromShader(RendererD3D *renderer,
                                           const gl::SharedCompiledShaderState &vertexShader);
    void updateCachedOutputLayoutFromShader();
    void updateCachedImage2DBindLayoutFromShader(gl::ShaderType shaderType);

    void updateCachedInputLayout(RendererD3D *renderer,
                                 UniqueSerial associatedSerial,
                                 const gl::State &state);
    void updateCachedOutputLayout(const gl::Context *context, const gl::Framebuffer *framebuffer);
    void updateCachedImage2DBindLayout(const gl::Context *context, const gl::ShaderType shaderType);
    void updateUniformBufferCache(const gl::Caps &caps);

    // Checks if we need to recompile certain shaders.
    bool hasVertexExecutableForCachedInputLayout();
    bool hasGeometryExecutableForPrimitiveType(RendererD3D *renderer,
                                               const gl::State &state,
                                               gl::PrimitiveMode drawMode);
    bool hasPixelExecutableForCachedOutputLayout();
    bool hasComputeExecutableForCachedImage2DBindLayout();

    bool anyShaderUniformsDirty() const { return mShaderUniformsDirty.any(); }
    bool areShaderUniformsDirty(gl::ShaderType shaderType) const
    {
        return mShaderUniformsDirty[shaderType];
    }
    void dirtyAllUniforms();
    void markUniformsClean();

    const std::vector<PixelShaderOutputVariable> &getPixelShaderKey() { return mPixelShaderKey; }

    void assignImage2DRegisters(gl::ShaderType shaderType,
                                unsigned int startImageIndex,
                                int startLogicalImageUnit,
                                bool readonly);

    const std::vector<D3DUniform *> &getD3DUniforms() const { return mD3DUniforms; }
    UniformStorageD3D *getShaderUniformStorage(gl::ShaderType shaderType) const
    {
        return mShaderUniformStorages[shaderType].get();
    }
    const AttribIndexArray &getAttribLocationToD3DSemantics() const
    {
        return mAttribLocationToD3DSemantic;
    }
    unsigned int getAtomicCounterBufferRegisterIndex(GLuint binding,
                                                     gl::ShaderType shaderType) const;
    unsigned int getShaderStorageBufferRegisterIndex(GLuint blockIndex,
                                                     gl::ShaderType shaderType) const;
    const std::vector<D3DUBOCache> &getShaderUniformBufferCache(gl::ShaderType shaderType) const;
    const std::vector<D3DUBOCacheUseSB> &getShaderUniformBufferCacheUseSB(
        gl::ShaderType shaderType) const;
    GLint getSamplerMapping(gl::ShaderType type,
                            unsigned int samplerIndex,
                            const gl::Caps &caps) const;
    gl::TextureType getSamplerTextureType(gl::ShaderType type, unsigned int samplerIndex) const;
    gl::RangeUI getUsedSamplerRange(gl::ShaderType type) const;

    bool isSamplerMappingDirty() const { return mDirtySamplerMapping; }
    void updateSamplerMapping();

    GLint getImageMapping(gl::ShaderType type,
                          unsigned int imageIndex,
                          bool readonly,
                          const gl::Caps &caps) const;
    gl::RangeUI getUsedImageRange(gl::ShaderType type, bool readonly) const;

    bool usesPointSize() const { return mUsesPointSize; }
    bool usesPointSpriteEmulation(RendererD3D *renderer) const;
    bool usesGeometryShader(RendererD3D *renderer,
                            const gl::ProvokingVertexConvention provokingVertex,
                            gl::PrimitiveMode drawMode) const;
    bool usesGeometryShaderForPointSpriteEmulation(RendererD3D *renderer) const;

    angle::Result getVertexExecutableForCachedInputLayout(d3d::Context *context,
                                                          RendererD3D *renderer,
                                                          ShaderExecutableD3D **outExectuable,
                                                          gl::InfoLog *infoLog);
    angle::Result getGeometryExecutableForPrimitiveType(
        d3d::Context *errContext,
        RendererD3D *renderer,
        const gl::Caps &caps,
        gl::ProvokingVertexConvention provokingVertex,
        gl::PrimitiveMode drawMode,
        ShaderExecutableD3D **outExecutable,
        gl::InfoLog *infoLog);
    angle::Result getPixelExecutableForCachedOutputLayout(d3d::Context *context,
                                                          RendererD3D *renderer,
                                                          ShaderExecutableD3D **outExectuable,
                                                          gl::InfoLog *infoLog);
    angle::Result getComputeExecutableForImage2DBindLayout(d3d::Context *context,
                                                           RendererD3D *renderer,
                                                           ShaderExecutableD3D **outExecutable,
                                                           gl::InfoLog *infoLog);

    bool hasShaderStage(gl::ShaderType shaderType) const
    {
        return mExecutable->getLinkedShaderStages()[shaderType];
    }

    bool hasNamedUniform(const std::string &name);

    bool usesVertexID() const { return mUsesVertexID; }

    angle::Result loadBinaryShaderExecutables(d3d::Context *contextD3D,
                                              RendererD3D *renderer,
                                              gl::BinaryInputStream *stream);

    unsigned int getSerial() const { return mSerial; }

  private:
    friend class ProgramD3D;

    bool load(const gl::Context *context, RendererD3D *renderer, gl::BinaryInputStream *stream);
    void save(const gl::Context *context, RendererD3D *renderer, gl::BinaryOutputStream *stream);

    template <typename DestT>
    void getUniformInternal(GLint location, DestT *dataOut) const;

    template <typename T>
    void setUniformImpl(D3DUniform *targetUniform,
                        const gl::VariableLocation &locationInfo,
                        GLsizei count,
                        const T *v,
                        uint8_t *targetData,
                        GLenum uniformType);

    template <typename T>
    void setUniformInternal(GLint location, GLsizei count, const T *v, GLenum uniformType);

    template <int cols, int rows>
    void setUniformMatrixfvInternal(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value);

    void initAttribLocationsToD3DSemantic(const gl::SharedCompiledShaderState &vertexShader);

    void reset();
    void initializeUniformBlocks();
    void initializeShaderStorageBlocks(const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders);
    void initializeUniformStorage(RendererD3D *renderer,
                                  const gl::ShaderBitSet &availableShaderStages);

    void updateCachedVertexExecutableIndex();
    void updateCachedPixelExecutableIndex();
    void updateCachedComputeExecutableIndex();

    void defineUniformsAndAssignRegisters(
        RendererD3D *renderer,
        const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders);
    void defineUniformBase(gl::ShaderType shaderType,
                           const sh::ShaderVariable &uniform,
                           D3DUniformMap *uniformMap);
    void assignAllSamplerRegisters(const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders);
    void assignSamplerRegisters(const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders,
                                size_t uniformIndex);

    static void AssignSamplers(unsigned int startSamplerIndex,
                               const gl::UniformTypeInfo &typeInfo,
                               unsigned int samplerCount,
                               std::vector<D3DSampler> &outSamplers,
                               gl::RangeUI *outUsedRange);

    void assignAllImageRegisters();
    void assignAllAtomicCounterRegisters();
    void assignImageRegisters(size_t uniformIndex);
    static void AssignImages(unsigned int startImageIndex,
                             int startLogicalImageUnit,
                             unsigned int imageCount,
                             std::vector<D3DImage> &outImages,
                             gl::RangeUI *outUsedRange);

    void gatherTransformFeedbackVaryings(
        RendererD3D *renderer,
        const gl::VaryingPacking &varyings,
        const std::vector<std::string> &transformFeedbackVaryingNames,
        const BuiltinInfo &builtins);
    D3DUniform *getD3DUniformFromLocation(const gl::VariableLocation &locationInfo);
    const D3DUniform *getD3DUniformFromLocation(const gl::VariableLocation &locationInfo) const;

    gl::ShaderMap<SharedCompiledShaderStateD3D> mAttachedShaders;

    std::vector<std::unique_ptr<D3DVertexExecutable>> mVertexExecutables;
    std::vector<std::unique_ptr<D3DPixelExecutable>> mPixelExecutables;
    angle::PackedEnumMap<gl::PrimitiveMode, std::unique_ptr<ShaderExecutableD3D>>
        mGeometryExecutables;
    std::vector<std::unique_ptr<D3DComputeExecutable>> mComputeExecutables;

    gl::ShaderMap<std::string> mShaderHLSL;
    gl::ShaderMap<CompilerWorkaroundsD3D> mShaderWorkarounds;

    FragDepthUsage mFragDepthUsage;
    bool mUsesSampleMask;
    bool mHasMultiviewEnabled;
    bool mUsesVertexID;
    bool mUsesViewID;
    std::vector<PixelShaderOutputVariable> mPixelShaderKey;

    // Common code for all dynamic geometry shaders. Consists mainly of the GS input and output
    // structures, built from the linked varying info. We store the string itself instead of the
    // packed varyings for simplicity.
    std::string mGeometryShaderPreamble;

    bool mUsesPointSize;
    bool mUsesFlatInterpolation;

    gl::ShaderMap<std::unique_ptr<UniformStorageD3D>> mShaderUniformStorages;

    gl::ShaderMap<std::vector<D3DSampler>> mShaderSamplers;
    gl::ShaderMap<gl::RangeUI> mUsedShaderSamplerRanges;
    bool mDirtySamplerMapping;

    gl::ShaderMap<std::vector<D3DImage>> mImages;
    gl::ShaderMap<std::vector<D3DImage>> mReadonlyImages;
    gl::ShaderMap<gl::RangeUI> mUsedImageRange;
    gl::ShaderMap<gl::RangeUI> mUsedReadonlyImageRange;
    gl::ShaderMap<gl::RangeUI> mUsedAtomicCounterRange;

    // Cache for pixel shader output layout to save reallocations.
    std::vector<GLenum> mPixelShaderOutputLayoutCache;
    Optional<size_t> mCachedPixelExecutableIndex;

    AttribIndexArray mAttribLocationToD3DSemantic;

    gl::ShaderMap<std::vector<D3DUBOCache>> mShaderUBOCaches;
    gl::ShaderMap<std::vector<D3DUBOCacheUseSB>> mShaderUBOCachesUseSB;
    D3DVertexExecutable::Signature mCachedVertexSignature;
    gl::InputLayout mCachedInputLayout;
    Optional<size_t> mCachedVertexExecutableIndex;

    std::vector<D3DVarying> mStreamOutVaryings;
    std::vector<D3DUniform *> mD3DUniforms;
    std::map<std::string, int> mImageBindingMap;
    std::map<std::string, int> mAtomicBindingMap;
    std::vector<D3DUniformBlock> mD3DUniformBlocks;
    std::vector<D3DInterfaceBlock> mD3DShaderStorageBlocks;
    gl::ShaderMap<std::vector<ShaderStorageBlock>> mShaderStorageBlocks;
    std::array<unsigned int, gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS>
        mComputeAtomicCounterBufferRegisterIndices;

    gl::ShaderMap<std::vector<sh::ShaderVariable>> mImage2DUniforms;
    gl::ShaderMap<gl::ImageUnitTextureTypeMap> mImage2DBindLayoutCache;
    Optional<size_t> mCachedComputeExecutableIndex;

    gl::ShaderBitSet mShaderUniformsDirty;

    UniqueSerial mCurrentVertexArrayStateSerial;

    unsigned int mSerial;

    static unsigned int issueSerial();
    static unsigned int mCurrentSerial;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_PROGRAMEXECUTABLED3D_H_
