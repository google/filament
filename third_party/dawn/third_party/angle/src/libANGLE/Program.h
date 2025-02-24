//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Program.h: Defines the gl::Program class. Implements GL program objects
// and related functionality. [OpenGL ES 2.0.24] section 2.10.3 page 28.

#ifndef LIBANGLE_PROGRAM_H_
#define LIBANGLE_PROGRAM_H_

#include <GLES2/gl2.h>
#include <GLSLANG/ShaderVars.h>

#include <array>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "common/Optional.h"
#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "common/hash_containers.h"
#include "common/mathutil.h"
#include "common/utilities.h"

#include "libANGLE/Constants.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/InfoLog.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/angletypes.h"

namespace rx
{
class GLImplFactory;
class ProgramImpl;
class LinkSubTask;
struct TranslatedAttribute;
}  // namespace rx

namespace gl
{
class Buffer;
class BinaryInputStream;
class BinaryOutputStream;
struct Caps;
class Context;
struct Extensions;
class Framebuffer;
class ProgramExecutable;
class ShaderProgramManager;
class State;
struct UnusedUniform;
struct Version;

extern const char *const g_fakepath;

enum class LinkMismatchError
{
    // Shared
    NO_MISMATCH,
    TYPE_MISMATCH,
    ARRAYNESS_MISMATCH,
    ARRAY_SIZE_MISMATCH,
    PRECISION_MISMATCH,
    STRUCT_NAME_MISMATCH,
    FIELD_NUMBER_MISMATCH,
    FIELD_NAME_MISMATCH,

    // Varying specific
    INTERPOLATION_TYPE_MISMATCH,
    INVARIANCE_MISMATCH,

    // Uniform specific
    BINDING_MISMATCH,
    LOCATION_MISMATCH,
    OFFSET_MISMATCH,
    INSTANCE_NAME_MISMATCH,
    FORMAT_MISMATCH,

    // Interface block specific
    LAYOUT_QUALIFIER_MISMATCH,
    MATRIX_PACKING_MISMATCH,

    // I/O block specific
    FIELD_LOCATION_MISMATCH,
    FIELD_STRUCT_NAME_MISMATCH,
};

void LogLinkMismatch(InfoLog &infoLog,
                     const std::string &variableName,
                     const char *variableType,
                     LinkMismatchError linkError,
                     const std::string &mismatchedStructOrBlockFieldName,
                     ShaderType shaderType1,
                     ShaderType shaderType2);

bool IsActiveInterfaceBlock(const sh::InterfaceBlock &interfaceBlock);

// Struct used for correlating uniforms/elements of uniform arrays to handles
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct VariableLocation
{
    static constexpr unsigned int kUnused = GL_INVALID_INDEX;

    VariableLocation();
    VariableLocation(unsigned int arrayIndex, unsigned int index);

    // If used is false, it means this location is only used to fill an empty space in an array,
    // and there is no corresponding uniform variable for this location. It can also mean the
    // uniform was optimized out by the implementation.
    bool used() const { return (index != kUnused); }
    void markUnused() { index = kUnused; }
    void markIgnored() { ignored = true; }

    bool operator==(const VariableLocation &other) const
    {
        return arrayIndex == other.arrayIndex && index == other.index;
    }

    // "index" is an index of the variable. The variable contains the indices for other than the
    // innermost GLSL arrays.
    uint32_t index;

    // "arrayIndex" stores the index of the innermost GLSL array. It's zero for non-arrays.
    uint32_t arrayIndex : 31;
    // If this location was bound to an unreferenced uniform.  Setting data on this uniform is a
    // no-op.
    uint32_t ignored : 1;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

// Information about a variable binding.
// Currently used by CHROMIUM_path_rendering
struct BindingInfo
{
    // The type of binding, for example GL_FLOAT_VEC3.
    // This can be GL_NONE if the variable is optimized away.
    GLenum type;

    // This is the name of the variable in
    // the translated shader program. Note that
    // this can be empty in the case where the
    // variable has been optimized away.
    std::string name;

    // True if the binding is valid, otherwise false.
    bool valid;
};

struct ProgramBinding
{
    ProgramBinding() : location(GL_INVALID_INDEX), aliased(false) {}
    ProgramBinding(GLuint index) : location(index), aliased(false) {}

    GLuint location;
    // Whether another binding was set that may potentially alias this.
    bool aliased;
};

class ProgramBindings final : angle::NonCopyable
{
  public:
    ProgramBindings();
    ~ProgramBindings();

    void bindLocation(GLuint index, const std::string &name);
    int getBindingByName(const std::string &name) const;
    template <typename T>
    int getBinding(const T &variable) const;

    using const_iterator = angle::HashMap<std::string, GLuint>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    std::map<std::string, GLuint> getStableIterationMap() const;

  private:
    angle::HashMap<std::string, GLuint> mBindings;
};

// Uniforms and Fragment Outputs require special treatment due to array notation (e.g., "[0]")
class ProgramAliasedBindings final : angle::NonCopyable
{
  public:
    ProgramAliasedBindings();
    ~ProgramAliasedBindings();

    void bindLocation(GLuint index, const std::string &name);
    int getBindingByName(const std::string &name) const;
    int getBindingByLocation(GLuint location) const;
    template <typename T>
    int getBinding(const T &variable) const;

    using const_iterator = angle::HashMap<std::string, ProgramBinding>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    std::map<std::string, ProgramBinding> getStableIterationMap() const;

  private:
    angle::HashMap<std::string, ProgramBinding> mBindings;
};

class ProgramState final : angle::NonCopyable
{
  public:
    ProgramState(rx::GLImplFactory *factory);
    ~ProgramState();

    const std::string &getLabel();

    SharedCompiledShaderState getAttachedShader(ShaderType shaderType) const;
    const ShaderMap<SharedCompiledShaderState> &getAttachedShaders() const
    {
        return mAttachedShaders;
    }
    const std::vector<std::string> &getTransformFeedbackVaryingNames() const
    {
        return mTransformFeedbackVaryingNames;
    }
    GLint getTransformFeedbackBufferMode() const { return mTransformFeedbackBufferMode; }

    bool hasAnyAttachedShader() const;

    const ProgramBindings &getAttributeBindings() const { return mAttributeBindings; }
    const ProgramAliasedBindings &getUniformLocationBindings() const
    {
        return mUniformLocationBindings;
    }
    const ProgramAliasedBindings &getFragmentOutputLocations() const
    {
        return mFragmentOutputLocations;
    }
    const ProgramAliasedBindings &getFragmentOutputIndexes() const
    {
        return mFragmentOutputIndexes;
    }

    const ProgramExecutable &getExecutable() const
    {
        ASSERT(mExecutable);
        return *mExecutable;
    }
    ProgramExecutable &getExecutable()
    {
        ASSERT(mExecutable);
        return *mExecutable;
    }

    const SharedProgramExecutable &getSharedExecutable() const
    {
        ASSERT(mExecutable);
        return mExecutable;
    }

    const std::string &getLabel() const { return mLabel; }

    bool hasBinaryRetrieveableHint() const { return mBinaryRetrieveableHint; }

    bool isSeparable() const { return mSeparable; }

    ShaderType getAttachedTransformFeedbackStage() const;

  private:
    friend class MemoryProgramCache;
    friend class Program;

    void updateActiveSamplers();
    void updateProgramInterfaceInputs();
    void updateProgramInterfaceOutputs();

    // Scans the sampler bindings for type conflicts with sampler 'textureUnitIndex'.
    void setSamplerUniformTextureTypeAndFormat(size_t textureUnitIndex);

    std::string mLabel;

    ShaderMap<SharedCompileJob> mShaderCompileJobs;
    ShaderMap<SharedCompiledShaderState> mAttachedShaders;

    std::vector<std::string> mTransformFeedbackVaryingNames;
    GLenum mTransformFeedbackBufferMode;

    bool mBinaryRetrieveableHint;
    bool mSeparable;

    ProgramBindings mAttributeBindings;

    // Note that this has nothing to do with binding layout qualifiers that can be set for some
    // uniforms in GLES3.1+. It is used to pre-set the location of uniforms.
    ProgramAliasedBindings mUniformLocationBindings;

    // EXT_blend_func_extended
    ProgramAliasedBindings mFragmentOutputLocations;
    ProgramAliasedBindings mFragmentOutputIndexes;

    InfoLog mInfoLog;

    // The result of the link.  State that is not the link output should remain in ProgramState,
    // while the link output should be placed in ProgramExecutable.
    //
    // This is a shared_ptr because it can be "installed" in the context as part of the rendering
    // context.  Similarly, it can be installed in a program pipeline.  Once the executable is
    // installed, the actual Program should not be referenced; it may have been unsuccessfully
    // relinked and its executable in an unusable state.
    SharedProgramExecutable mExecutable;
};

struct ProgramVaryingRef
{
    const sh::ShaderVariable *get(ShaderType stage) const
    {
        ASSERT(stage == frontShaderStage || stage == backShaderStage);
        const sh::ShaderVariable *ref = stage == frontShaderStage ? frontShader : backShader;
        ASSERT(ref);
        return ref;
    }

    const sh::ShaderVariable *frontShader = nullptr;
    const sh::ShaderVariable *backShader  = nullptr;
    ShaderType frontShaderStage           = ShaderType::InvalidEnum;
    ShaderType backShaderStage            = ShaderType::InvalidEnum;
};

using ProgramMergedVaryings = std::vector<ProgramVaryingRef>;

class Program final : public LabeledObject, public angle::Subject
{
  public:
    Program(rx::GLImplFactory *factory, ShaderProgramManager *manager, ShaderProgramID handle);
    void onDestroy(const Context *context);

    ShaderProgramID id() const;

    angle::Result setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    ANGLE_INLINE rx::ProgramImpl *getImplementation() const
    {
        ASSERT(!mLinkingState);
        return mProgram;
    }

    void attachShader(const Context *context, Shader *shader);
    void detachShader(const Context *context, Shader *shader);
    int getAttachedShadersCount() const;

    Shader *getAttachedShader(ShaderType shaderType) const;

    void bindAttributeLocation(const Context *context, GLuint index, const char *name);
    void bindUniformLocation(const Context *context, UniformLocation location, const char *name);

    // EXT_blend_func_extended
    void bindFragmentOutputLocation(const Context *context, GLuint index, const char *name);
    void bindFragmentOutputIndex(const Context *context, GLuint index, const char *name);

    // KHR_parallel_shader_compile
    // Try to link the program asynchronously. As a result, background threads may be launched to
    // execute the linking tasks concurrently.
    angle::Result link(const Context *context, angle::JobResultExpectancy resultExpectancy);

    // Peek whether there is any running linking tasks.
    bool isLinking() const;
    bool hasLinkingState() const { return mLinkingState != nullptr; }

    bool isLinked() const
    {
        ASSERT(!mLinkingState);
        return mLinked;
    }
    bool isBinaryReady(const Context *context);
    ANGLE_INLINE void cacheProgramBinaryIfNecessary(const Context *context)
    {
        // This function helps ensure the program binary is cached, even if the backend waits for
        // post-link tasks without the knowledge of the front-end.
        if (!mIsBinaryCached && !mState.mBinaryRetrieveableHint &&
            mState.mExecutable->mPostLinkSubTasks.empty())
        {
            cacheProgramBinaryIfNotAlready(context);
        }
    }

    angle::Result setBinary(const Context *context,
                            GLenum binaryFormat,
                            const void *binary,
                            GLsizei length);
    angle::Result getBinary(Context *context,
                            GLenum *binaryFormat,
                            void *binary,
                            GLsizei bufSize,
                            GLsizei *length);
    GLint getBinaryLength(Context *context);
    void setBinaryRetrievableHint(bool retrievable);
    bool getBinaryRetrievableHint() const;

    angle::Result loadBinary(const Context *context,
                             const void *binary,
                             GLsizei length,
                             egl::CacheGetResult *resultOut);

    InfoLog &getInfoLog() { return mState.mInfoLog; }
    int getInfoLogLength() const;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const;

    void setSeparable(const Context *context, bool separable);
    bool isSeparable() const { return mState.mSeparable; }

    void getAttachedShaders(GLsizei maxCount, GLsizei *count, ShaderProgramID *shaders) const;

    void bindUniformBlock(UniformBlockIndex uniformBlockIndex, GLuint uniformBlockBinding);

    void setTransformFeedbackVaryings(const Context *context,
                                      GLsizei count,
                                      const GLchar *const *varyings,
                                      GLenum bufferMode);
    GLenum getTransformFeedbackBufferMode() const { return mState.mTransformFeedbackBufferMode; }

    ANGLE_INLINE void addRef() { mRefCount++; }

    ANGLE_INLINE void release(const Context *context)
    {
        mRefCount--;

        if (mRefCount == 0 && mDeleteStatus)
        {
            deleteSelf(context);
        }
    }

    unsigned int getRefCount() const;
    bool isInUse() const { return getRefCount() != 0; }
    void flagForDeletion();
    bool isFlaggedForDeletion() const;

    void validate(const Caps &caps);
    bool isValidated() const;

    const ProgramState &getState() const { return mState; }

    const ProgramBindings &getAttributeBindings() const { return mState.getAttributeBindings(); }
    const ProgramAliasedBindings &getUniformLocationBindings() const
    {
        return mState.getUniformLocationBindings();
    }
    const ProgramAliasedBindings &getFragmentOutputLocations() const
    {
        return mState.getFragmentOutputLocations();
    }
    const ProgramAliasedBindings &getFragmentOutputIndexes() const
    {
        return mState.getFragmentOutputIndexes();
    }

    // Try to resolve linking. Inlined to make sure its overhead is as low as possible.
    void resolveLink(const Context *context)
    {
        if (ANGLE_UNLIKELY(mLinkingState))
        {
            resolveLinkImpl(context);
        }
    }

    // Writes a program's binary to |mBinary|.
    angle::Result serialize(const Context *context);
    const angle::MemoryBuffer &getSerializedBinary() const { return mBinary; }

    rx::UniqueSerial serial() const { return mSerial; }

    const ProgramExecutable &getExecutable() const { return mState.getExecutable(); }
    ProgramExecutable &getExecutable() { return mState.getExecutable(); }
    const SharedProgramExecutable &getSharedExecutable() const
    {
        return mState.getSharedExecutable();
    }

  private:
    class MainLinkLoadTask;
    class MainLoadTask;
    class MainLinkTask;
    class MainLinkLoadEvent;

    friend class ProgramPipeline;
    friend class MainLinkLoadTask;
    friend class MainLoadTask;
    friend class MainLinkTask;

    struct LinkingState;
    ~Program() override;

    // Loads program state according to the specified binary blob.  Returns true on success.
    bool deserialize(const Context *context, BinaryInputStream &stream);

    void unlink();
    void setupExecutableForLink(const Context *context);
    void deleteSelf(const Context *context);

    angle::Result linkJobImpl(const Caps &caps,
                              const Limitations &limitations,
                              const Version &clientVersion,
                              bool isWebGL,
                              LinkingVariables *linkingVariables,
                              ProgramLinkedResources *resources,
                              ProgramMergedVaryings *mergedVaryingsOut);

    void makeNewExecutable(const Context *context);

    bool linkValidateShaders();
    void linkShaders();
    bool linkAttributes(const Caps &caps, const Limitations &limitations, bool webglCompatibility);
    bool linkVaryings();

    bool linkUniforms(const Caps &caps,
                      const Version &clientVersion,
                      std::vector<UnusedUniform> *unusedUniformsOutOrNull,
                      GLuint *combinedImageUniformsOut);

    void updateLinkedShaderStages();

    // Block until linking is finished and resolve it.
    void resolveLinkImpl(const Context *context);
    // Block until post-link tasks are finished.
    void waitForPostLinkTasks(const Context *context);

    void postResolveLink(const Context *context);
    void cacheProgramBinaryIfNotAlready(const Context *context);

    void dumpProgramInfo(const Context *context) const;

    rx::UniqueSerial mSerial;
    ProgramState mState;
    rx::ProgramImpl *mProgram;

    bool mValidated;
    // Flag to indicate that the program can be deleted when no longer in use
    bool mDeleteStatus;
    // Whether the program binary is implicitly cached yet.  This is usually done in
    // |resolveLinkImpl|, but may be deferred in the presence of post-link tasks.  In that case,
    // |waitForPostLinkTasks| would cache the binary.  However, if the wait on the tasks is done by
    // the backend itself, this caching will not be done.  This flag is used to make sure the binary
    // is eventually cached at some point in the future.
    bool mIsBinaryCached;

    bool mLinked;
    std::unique_ptr<LinkingState> mLinkingState;

    egl::BlobCache::Key mProgramHash;

    unsigned int mRefCount;

    ShaderProgramManager *mResourceManager;
    const ShaderProgramID mHandle;

    // ProgramState::mAttachedShaders holds a reference to shaders' compiled state, which is all the
    // program and the backends require after link.  The actual shaders linked to the program are
    // stored here to support shader attach/detach and link without providing access to them in the
    // backends.
    ShaderMap<Shader *> mAttachedShaders;

    // A cache of the program binary, prepared by |serialize()|.  OpenGL requires the application to
    // query the length of the binary first (requiring a call to |serialize()|), and then get the
    // actual binary.  This cache ensures the second call does not need to call |serialize()| again.
    angle::MemoryBuffer mBinary;

    angle::SimpleMutex mHistogramMutex;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAM_H_
