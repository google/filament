//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramExecutableMtl.h: Implementation of ProgramExecutableImpl.

#ifndef LIBANGLE_RENDERER_MTL_PROGRAMEXECUTABLEMTL_H_
#define LIBANGLE_RENDERER_MTL_PROGRAMEXECUTABLEMTL_H_

#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/renderer/ProgramExecutableImpl.h"
#include "libANGLE/renderer/metal/mtl_buffer_pool.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_common.h"
#include "libANGLE/renderer/metal/mtl_msl_utils.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"

namespace rx
{
class ContextMtl;

struct UBOConversionInfo
{

    UBOConversionInfo(const std::vector<sh::BlockMemberInfo> &stdInfo,
                      const std::vector<sh::BlockMemberInfo> &metalInfo,
                      size_t stdSize,
                      size_t metalSize)
        : _stdInfo(stdInfo), _metalInfo(metalInfo), _stdSize(stdSize), _metalSize(metalSize)
    {
        _needsConversion = _calculateNeedsConversion();
    }
    const std::vector<sh::BlockMemberInfo> &stdInfo() const { return _stdInfo; }
    const std::vector<sh::BlockMemberInfo> &metalInfo() const { return _metalInfo; }
    size_t stdSize() const { return _stdSize; }
    size_t metalSize() const { return _metalSize; }

    bool needsConversion() const { return _needsConversion; }

  private:
    std::vector<sh::BlockMemberInfo> _stdInfo, _metalInfo;
    size_t _stdSize, _metalSize;
    bool _needsConversion;

    bool _calculateNeedsConversion()
    {
        // If we have a different number of fields then we need conversion
        if (_stdInfo.size() != _metalInfo.size())
        {
            return true;
        }
        for (size_t i = 0; i < _stdInfo.size(); ++i)
        {
            // If the matrix is transposed
            if (_stdInfo[i].isRowMajorMatrix)
            {
                return true;
            }
            // If we have a bool
            if (gl::VariableComponentType(_stdInfo[i].type) == GL_BOOL)
            {
                return true;
            }
            // If any offset information is different
            if (!(_stdInfo[i] == _metalInfo[i]))
            {
                return true;
            }
        }
        return false;
    }
};

struct ProgramArgumentBufferEncoderMtl
{
    void reset(ContextMtl *contextMtl);

    mtl::AutoObjCPtr<id<MTLArgumentEncoder>> metalArgBufferEncoder;
    mtl::BufferPool bufferPool;
};

constexpr size_t kFragmentShaderVariants = 4;

// Represents a specialized shader variant. For example, a shader variant with fragment coverage
// mask enabled and a shader variant without.
struct ProgramShaderObjVariantMtl
{
    void reset(ContextMtl *contextMtl);

    mtl::AutoObjCPtr<id<MTLFunction>> metalShader;
    // UBO's argument buffer encoder. Used when number of UBOs used exceeds number of allowed
    // discrete slots, and thus needs to encode all into one argument buffer.
    ProgramArgumentBufferEncoderMtl uboArgBufferEncoder;

    // Store reference to the TranslatedShaderInfo to easy querying mapped textures/UBO/XFB
    // bindings.
    const mtl::TranslatedShaderInfo *translatedSrcInfo;
};

// State for the default uniform blocks.
struct DefaultUniformBlockMtl final : private angle::NonCopyable
{
    DefaultUniformBlockMtl();
    ~DefaultUniformBlockMtl();

    // Shadow copies of the shader uniform data.
    angle::MemoryBuffer uniformData;

    // Since the default blocks are laid out in std140, this tells us where to write on a call
    // to a setUniform method. They are arranged in uniform location order.
    std::vector<sh::BlockMemberInfo> uniformLayout;
};

class ProgramExecutableMtl : public ProgramExecutableImpl
{
  public:
    ProgramExecutableMtl(const gl::ProgramExecutable *executable);
    ~ProgramExecutableMtl() override;

    void destroy(const gl::Context *context) override;

    angle::Result load(ContextMtl *contextMtl, gl::BinaryInputStream *stream);
    void save(gl::BinaryOutputStream *stream);

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

    bool hasFlatAttribute() const { return mProgramHasFlatAttributes; }

    // Calls this before drawing, changedPipelineDesc is passed when vertex attributes desc and/or
    // shader program changed.
    angle::Result setupDraw(const gl::Context *glContext,
                            mtl::RenderCommandEncoder *cmdEncoder,
                            const mtl::RenderPipelineDesc &pipelineDesc,
                            bool pipelineDescChanged,
                            bool forceTexturesSetting,
                            bool uniformBuffersDirty);

  private:
    friend class ProgramMtl;

    void reset(ContextMtl *context);

    template <int cols, int rows>
    void setUniformMatrixfv(GLint location,
                            GLsizei count,
                            GLboolean transpose,
                            const GLfloat *value);
    template <class T>
    void getUniformImpl(GLint location, T *v, GLenum entryPointType) const;

    template <typename T>
    void setUniformImpl(GLint location, GLsizei count, const T *v, GLenum entryPointType);

    void saveTranslatedShaders(gl::BinaryOutputStream *stream);
    void loadTranslatedShaders(gl::BinaryInputStream *stream);

    void saveShaderInternalInfo(gl::BinaryOutputStream *stream);
    void loadShaderInternalInfo(gl::BinaryInputStream *stream);

    void saveInterfaceBlockInfo(gl::BinaryOutputStream *stream);
    angle::Result loadInterfaceBlockInfo(gl::BinaryInputStream *stream);

    void saveDefaultUniformBlocksInfo(gl::BinaryOutputStream *stream);
    angle::Result loadDefaultUniformBlocksInfo(mtl::Context *context,
                                               gl::BinaryInputStream *stream);

    void linkUpdateHasFlatAttributes(const gl::SharedCompiledShaderState &vertexShader);

    angle::Result initDefaultUniformBlocks(
        mtl::Context *context,
        const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders);
    angle::Result resizeDefaultUniformBlocksMemory(mtl::Context *context,
                                                   const gl::ShaderMap<size_t> &requiredBufferSize);
    void initUniformBlocksRemapper(const gl::SharedCompiledShaderState &shader);

    mtl::BufferPool *getBufferPool(ContextMtl *context, gl::ShaderType shaderType);

    angle::Result getSpecializedShader(ContextMtl *context,
                                       gl::ShaderType shaderType,
                                       const mtl::RenderPipelineDesc &renderPipelineDesc,
                                       id<MTLFunction> *shaderOut);

    angle::Result commitUniforms(ContextMtl *context, mtl::RenderCommandEncoder *cmdEncoder);
    angle::Result updateTextures(const gl::Context *glContext,
                                 mtl::RenderCommandEncoder *cmdEncoder,
                                 bool forceUpdate);

    angle::Result updateUniformBuffers(ContextMtl *context,
                                       mtl::RenderCommandEncoder *cmdEncoder,
                                       const mtl::RenderPipelineDesc &pipelineDesc);
    angle::Result updateXfbBuffers(ContextMtl *context,
                                   mtl::RenderCommandEncoder *cmdEncoder,
                                   const mtl::RenderPipelineDesc &pipelineDesc);
    angle::Result legalizeUniformBufferOffsets(ContextMtl *context);
    angle::Result bindUniformBuffersToDiscreteSlots(ContextMtl *context,
                                                    mtl::RenderCommandEncoder *cmdEncoder,
                                                    gl::ShaderType shaderType);

    angle::Result encodeUniformBuffersInfoArgumentBuffer(ContextMtl *context,
                                                         mtl::RenderCommandEncoder *cmdEncoder,
                                                         gl::ShaderType shaderType);

    bool mProgramHasFlatAttributes;

    gl::ShaderMap<DefaultUniformBlockMtl> mDefaultUniformBlocks;
    std::unordered_map<std::string, UBOConversionInfo> mUniformBlockConversions;

    // Translated metal shaders:
    gl::ShaderMap<mtl::TranslatedShaderInfo> mMslShaderTranslateInfo;

    // Translated metal version for transform feedback only vertex shader:
    // - Metal doesn't allow vertex shader to write to both buffers and to stage output
    // (gl_Position). Need a special version of vertex shader that only writes to transform feedback
    // buffers.
    mtl::TranslatedShaderInfo mMslXfbOnlyVertexShaderInfo;

    // Compiled native shader object variants:
    // - Vertex shader: One with emulated rasterization discard, one with true rasterization
    // discard, one without.
    mtl::RenderPipelineRasterStateMap<ProgramShaderObjVariantMtl> mVertexShaderVariants;
    // - Fragment shader: Combinations of sample coverage mask and depth write enabled states.
    std::array<ProgramShaderObjVariantMtl, kFragmentShaderVariants> mFragmentShaderVariants;

    // Cached references of current shader variants.
    gl::ShaderMap<ProgramShaderObjVariantMtl *> mCurrentShaderVariants;

    gl::ShaderBitSet mDefaultUniformBlocksDirty;
    gl::ShaderBitSet mSamplerBindingsDirty;

    // Scratch data:
    // Legalized buffers and their offsets. For example, uniform buffer's offset=1 is not a valid
    // offset, it will be converted to legal offset and the result is stored in this array.
    std::vector<std::pair<mtl::BufferRef, uint32_t>> mLegalizedOffsetedUniformBuffers;
    // Stores the render stages usage of each uniform buffer. Only used if the buffers are encoded
    // into an argument buffer.
    std::vector<uint32_t> mArgumentBufferRenderStageUsages;

    uint32_t mShadowCompareModes[mtl::kMaxShaderSamplers];

    gl::ShaderMap<std::unique_ptr<mtl::BufferPool>> mDefaultUniformBufferPools;
};

angle::Result CreateMslShaderLib(mtl::Context *context,
                                 gl::InfoLog &infoLog,
                                 mtl::TranslatedShaderInfo *translatedMslInfo,
                                 const std::map<std::string, std::string> &substitutionMacros);
}  // namespace rx

#endif  // LIBANGLE_RENDERER_MTL_PROGRAMEXECUTABLEMTL_H_
