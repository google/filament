//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramLinkedResources.h: implements link-time checks for default block uniforms, and generates
// uniform locations. Populates data structures related to uniforms so that they can be stored in
// program state.

#ifndef LIBANGLE_UNIFORMLINKER_H_
#define LIBANGLE_UNIFORMLINKER_H_

#include <GLSLANG/ShaderVars.h>

#include "angle_gl.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VaryingPacking.h"

#include <functional>

namespace sh
{
class BlockLayoutEncoder;
struct BlockMemberInfo;
struct InterfaceBlock;
struct ShaderVariable;
class BlockEncoderVisitor;
class ShaderVariableVisitor;
struct ShaderVariable;
}  // namespace sh

namespace gl
{
struct BufferVariable;
struct Caps;
class Context;
class InfoLog;
struct InterfaceBlock;
enum class LinkMismatchError;
class ProgramState;
class ProgramPipelineState;
class ProgramBindings;
class ProgramAliasedBindings;
class Shader;
struct AtomicCounterBuffer;
struct VariableLocation;
struct Version;

using ShaderUniform = std::pair<ShaderType, const sh::ShaderVariable *>;

// The link operation is responsible for finishing the link of uniform and interface blocks.
// This way it can filter out unreferenced resources and still have access to the info.
// TODO(jmadill): Integrate uniform linking/filtering as well as interface blocks.
struct UnusedUniform
{
    UnusedUniform(std::string name,
                  bool isSampler,
                  bool isImage,
                  bool isAtomicCounter,
                  bool isFragmentInOut)
    {
        this->name            = name;
        this->isSampler       = isSampler;
        this->isImage         = isImage;
        this->isAtomicCounter = isAtomicCounter;
        this->isFragmentInOut = isFragmentInOut;
    }

    std::string name;
    bool isSampler;
    bool isImage;
    bool isAtomicCounter;
    bool isFragmentInOut;
};

struct UsedUniform : public sh::ShaderVariable
{
    UsedUniform();
    UsedUniform(GLenum type,
                GLenum precision,
                const std::string &name,
                const std::vector<unsigned int> &arraySizes,
                const int binding,
                const int offset,
                const int location,
                const int bufferIndex,
                const sh::BlockMemberInfo &blockInfo);
    UsedUniform(const UsedUniform &other);
    UsedUniform &operator=(const UsedUniform &other);
    ~UsedUniform();

    bool isSampler() const { return typeInfo->isSampler; }
    bool isImage() const { return typeInfo->isImageType; }
    bool isAtomicCounter() const { return IsAtomicCounterType(type); }
    size_t getElementSize() const { return typeInfo->externalSize; }

    void setActive(ShaderType shaderType, bool used, uint32_t _id)
    {
        activeVariable.setActive(shaderType, used, _id);
    }

    ActiveVariable activeVariable;
    const UniformTypeInfo *typeInfo;

    // Identifies the containing buffer backed resource -- interface block or atomic counter buffer.
    int bufferIndex;
    sh::BlockMemberInfo blockInfo;
    std::vector<unsigned int> outerArraySizes;
    unsigned int outerArrayOffset;
};

class UniformLinker final : angle::NonCopyable
{
  public:
    UniformLinker(const ShaderBitSet &activeShaderStages,
                  const ShaderMap<std::vector<sh::ShaderVariable>> &shaderUniforms);
    ~UniformLinker();

    bool link(const Caps &caps,
              InfoLog &infoLog,
              const ProgramAliasedBindings &uniformLocationBindings);

    void getResults(std::vector<LinkedUniform> *uniforms,
                    std::vector<std::string> *uniformNames,
                    std::vector<std::string> *uniformMappedNames,
                    std::vector<UnusedUniform> *unusedUniformsOutOrNull,
                    std::vector<VariableLocation> *uniformLocationsOutOrNull);

  private:
    bool validateGraphicsUniforms(InfoLog &infoLog) const;
    bool validateGraphicsUniformsPerShader(ShaderType shaderToLink,
                                           bool extendLinkedUniforms,
                                           std::map<std::string, ShaderUniform> *linkedUniforms,
                                           InfoLog &infoLog) const;
    bool flattenUniformsAndCheckCapsForShader(ShaderType shaderType,
                                              const Caps &caps,
                                              std::vector<UsedUniform> &samplerUniforms,
                                              std::vector<UsedUniform> &imageUniforms,
                                              std::vector<UsedUniform> &atomicCounterUniforms,
                                              std::vector<UsedUniform> &inputAttachmentUniforms,
                                              std::vector<UnusedUniform> &unusedUniforms,
                                              InfoLog &infoLog);

    bool flattenUniformsAndCheckCaps(const Caps &caps, InfoLog &infoLog);
    bool checkMaxCombinedAtomicCounters(const Caps &caps, InfoLog &infoLog);

    bool indexUniforms(InfoLog &infoLog, const ProgramAliasedBindings &uniformLocationBindings);
    bool gatherUniformLocationsAndCheckConflicts(
        InfoLog &infoLog,
        const ProgramAliasedBindings &uniformLocationBindings,
        std::set<GLuint> *ignoredLocations,
        int *maxUniformLocation);
    void pruneUnusedUniforms();

    ShaderBitSet mActiveShaderStages;
    const ShaderMap<std::vector<sh::ShaderVariable>> &mShaderUniforms;
    std::vector<UsedUniform> mUniforms;
    std::vector<UnusedUniform> mUnusedUniforms;
    std::vector<VariableLocation> mUniformLocations;
};

using GetBlockSizeFunc = std::function<
    bool(const std::string &blockName, const std::string &blockMappedName, size_t *sizeOut)>;
using GetBlockMemberInfoFunc = std::function<
    bool(const std::string &name, const std::string &mappedName, sh::BlockMemberInfo *infoOut)>;

// This class is intended to be used during the link step to store interface block information.
// It is called by the Impl class during ProgramImpl::link so that it has access to the
// real block size and layout.
class InterfaceBlockLinker : angle::NonCopyable
{
  public:
    virtual ~InterfaceBlockLinker();

    // This is called once per shader stage. It stores a pointer to the block vector, so it's
    // important that this class does not persist longer than the duration of Program::link.
    void addShaderBlocks(ShaderType shader, const std::vector<sh::InterfaceBlock> *blocks);

    // This is called once during a link operation, after all shader blocks are added.
    void linkBlocks(const GetBlockSizeFunc &getBlockSize,
                    const GetBlockMemberInfoFunc &getMemberInfo) const;

    const std::vector<sh::InterfaceBlock> &getShaderBlocks(ShaderType shaderType) const
    {
        ASSERT(mShaderBlocks[shaderType]);
        return *mShaderBlocks[shaderType];
    }

  protected:
    InterfaceBlockLinker();
    void init(std::vector<InterfaceBlock> *blocksOut,
              std::vector<std::string> *unusedInterfaceBlocksOut);
    void defineInterfaceBlock(const GetBlockSizeFunc &getBlockSize,
                              const GetBlockMemberInfoFunc &getMemberInfo,
                              const sh::InterfaceBlock &interfaceBlock,
                              ShaderType shaderType) const;

    virtual size_t getCurrentBlockMemberIndex() const = 0;

    virtual sh::ShaderVariableVisitor *getVisitor(const GetBlockMemberInfoFunc &getMemberInfo,
                                                  const std::string &namePrefix,
                                                  const std::string &mappedNamePrefix,
                                                  ShaderType shaderType,
                                                  int blockIndex) const = 0;

    ShaderMap<const std::vector<sh::InterfaceBlock> *> mShaderBlocks = {};

    std::vector<InterfaceBlock> *mBlocksOut             = nullptr;
    std::vector<std::string> *mUnusedInterfaceBlocksOut = nullptr;
};

class UniformBlockLinker final : public InterfaceBlockLinker
{
  public:
    UniformBlockLinker();
    ~UniformBlockLinker() override;

    void init(std::vector<InterfaceBlock> *blocksOut,
              std::vector<LinkedUniform> *uniformsOut,
              std::vector<std::string> *uniformNamesOut,
              std::vector<std::string> *uniformMappedNamesOut,
              std::vector<std::string> *unusedInterfaceBlocksOut);

  private:
    size_t getCurrentBlockMemberIndex() const override;

    sh::ShaderVariableVisitor *getVisitor(const GetBlockMemberInfoFunc &getMemberInfo,
                                          const std::string &namePrefix,
                                          const std::string &mappedNamePrefix,
                                          ShaderType shaderType,
                                          int blockIndex) const override;

    std::vector<LinkedUniform> *mUniformsOut         = nullptr;
    std::vector<std::string> *mUniformNamesOut       = nullptr;
    std::vector<std::string> *mUniformMappedNamesOut = nullptr;
};

class ShaderStorageBlockLinker final : public InterfaceBlockLinker
{
  public:
    ShaderStorageBlockLinker();
    ~ShaderStorageBlockLinker() override;

    void init(std::vector<InterfaceBlock> *blocksOut,
              std::vector<BufferVariable> *bufferVariablesOut,
              std::vector<std::string> *unusedInterfaceBlocksOut);

  private:
    size_t getCurrentBlockMemberIndex() const override;

    sh::ShaderVariableVisitor *getVisitor(const GetBlockMemberInfoFunc &getMemberInfo,
                                          const std::string &namePrefix,
                                          const std::string &mappedNamePrefix,
                                          ShaderType shaderType,
                                          int blockIndex) const override;

    std::vector<BufferVariable> *mBufferVariablesOut = nullptr;
};

class AtomicCounterBufferLinker final : angle::NonCopyable
{
  public:
    AtomicCounterBufferLinker();
    ~AtomicCounterBufferLinker();

    void init(std::vector<AtomicCounterBuffer> *atomicCounterBuffersOut);
    void link(const std::map<int, unsigned int> &sizeMap) const;

  private:
    std::vector<AtomicCounterBuffer> *mAtomicCounterBuffersOut = nullptr;
};

class PixelLocalStorageLinker final : angle::NonCopyable
{
  public:
    PixelLocalStorageLinker();
    ~PixelLocalStorageLinker();

    void init(std::vector<ShPixelLocalStorageFormat> *pixelLocalStorageFormatsOut);
    void link(const std::vector<ShPixelLocalStorageFormat> &pixelLocalStorageFormats) const;

  private:
    std::vector<ShPixelLocalStorageFormat> *mPixelLocalStorageFormatsOut = nullptr;
};

struct ProgramLinkedResources
{
    ProgramLinkedResources();
    ~ProgramLinkedResources();

    void init(std::vector<InterfaceBlock> *uniformBlocksOut,
              std::vector<LinkedUniform> *uniformsOut,
              std::vector<std::string> *uniformNamesOut,
              std::vector<std::string> *uniformMappedNamesOut,
              std::vector<InterfaceBlock> *shaderStorageBlocksOut,
              std::vector<BufferVariable> *bufferVariablesOut,
              std::vector<AtomicCounterBuffer> *atomicCounterBuffersOut,
              std::vector<ShPixelLocalStorageFormat> *pixelLocalStorageFormatsOut);

    ProgramVaryingPacking varyingPacking;
    UniformBlockLinker uniformBlockLinker;
    ShaderStorageBlockLinker shaderStorageBlockLinker;
    AtomicCounterBufferLinker atomicCounterBufferLinker;
    PixelLocalStorageLinker pixelLocalStorageLinker;
    std::vector<UnusedUniform> unusedUniforms;
    std::vector<std::string> unusedInterfaceBlocks;
};

struct LinkingVariables final : private angle::NonCopyable
{
    LinkingVariables();
    ~LinkingVariables();

    void initForProgram(const ProgramState &state);
    void initForProgramPipeline(const ProgramPipelineState &state);

    ShaderMap<std::vector<sh::ShaderVariable>> outputVaryings;
    ShaderMap<std::vector<sh::ShaderVariable>> inputVaryings;
    ShaderMap<std::vector<sh::ShaderVariable>> uniforms;
    ShaderMap<std::vector<sh::InterfaceBlock>> uniformBlocks;
    ShaderBitSet isShaderStageUsedBitset;
};

class CustomBlockLayoutEncoderFactory : angle::NonCopyable
{
  public:
    virtual ~CustomBlockLayoutEncoderFactory() {}

    virtual sh::BlockLayoutEncoder *makeEncoder() = 0;
};

// Used by the backends in Program*::linkResources to parse interface blocks and provide
// information to ProgramLinkedResources' linkers.
class ProgramLinkedResourcesLinker final : angle::NonCopyable
{
  public:
    ProgramLinkedResourcesLinker(CustomBlockLayoutEncoderFactory *customEncoderFactory)
        : mCustomEncoderFactory(customEncoderFactory)
    {}

    void linkResources(const ProgramState &programState,
                       const ProgramLinkedResources &resources) const;

  private:
    void getAtomicCounterBufferSizeMap(const ProgramExecutable &executable,
                                       std::map<int, unsigned int> &sizeMapOut) const;

    CustomBlockLayoutEncoderFactory *mCustomEncoderFactory;
};

using ShaderInterfaceBlock = std::pair<ShaderType, const sh::InterfaceBlock *>;
using InterfaceBlockMap    = std::map<std::string, ShaderInterfaceBlock>;

bool LinkValidateProgramGlobalNames(InfoLog &infoLog,
                                    const ProgramExecutable &executable,
                                    const LinkingVariables &linkingVariables);
bool LinkValidateShaderInterfaceMatching(const std::vector<sh::ShaderVariable> &outputVaryings,
                                         const std::vector<sh::ShaderVariable> &inputVaryings,
                                         ShaderType frontShaderType,
                                         ShaderType backShaderType,
                                         int frontShaderVersion,
                                         int backShaderVersion,
                                         bool isSeparable,
                                         InfoLog &infoLog);
bool LinkValidateBuiltInVaryingsInvariant(const std::vector<sh::ShaderVariable> &vertexVaryings,
                                          const std::vector<sh::ShaderVariable> &fragmentVaryings,
                                          int vertexShaderVersion,
                                          InfoLog &infoLog);
bool LinkValidateBuiltInVaryings(const std::vector<sh::ShaderVariable> &inputVaryings,
                                 const std::vector<sh::ShaderVariable> &outputVaryings,
                                 ShaderType inputShaderType,
                                 ShaderType outputShaderType,
                                 int inputShaderVersion,
                                 int outputShaderVersion,
                                 InfoLog &infoLog);
LinkMismatchError LinkValidateProgramVariables(const sh::ShaderVariable &variable1,
                                               const sh::ShaderVariable &variable2,
                                               bool validatePrecision,
                                               bool treatVariable1AsNonArray,
                                               bool treatVariable2AsNonArray,
                                               std::string *mismatchedStructOrBlockMemberName);
void AddProgramVariableParentPrefix(const std::string &parentName,
                                    std::string *mismatchedFieldName);
bool LinkValidateProgramInterfaceBlocks(const Caps &caps,
                                        const Version &clientVersion,
                                        bool webglCompatibility,
                                        ShaderBitSet activeProgramStages,
                                        const ProgramLinkedResources &resources,
                                        InfoLog &infoLog,
                                        GLuint *combinedShaderStorageBlocksCountOut);
}  // namespace gl

#endif  // LIBANGLE_UNIFORMLINKER_H_
