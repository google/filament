//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_H_
#define COMPILER_TRANSLATOR_MSL_H_

#include "compiler/translator/Compiler.h"

namespace sh
{

constexpr const char kUniformsVar[]                    = "angleUniforms";
constexpr const char kUnassignedAttributeString[]      = " __unassigned_attribute__";
constexpr const char kUnassignedFragmentOutputString[] = "__unassigned_output__";

class DriverUniform;
class DriverUniformMetal;
class SpecConst;
class TOutputMSL;
class TranslatorMetalReflection;
typedef std::unordered_map<size_t, std::string> originalNamesMap;
typedef std::unordered_map<std::string, size_t> samplerBindingMap;
typedef std::unordered_map<std::string, size_t> textureBindingMap;
typedef std::unordered_map<int, int> rwTextureBindingMap;  // GLSL image -> mtl read_write texture.
typedef std::unordered_map<std::string, size_t> userUniformBufferBindingMap;
typedef std::pair<size_t, size_t> uboBindingInfo;
struct UBOBindingInfo
{
    size_t bindIndex = 0;
    size_t arraySize = 0;
};
typedef std::unordered_map<std::string, UBOBindingInfo> uniformBufferBindingMap;

namespace mtl
{
TranslatorMetalReflection *getTranslatorMetalReflection(const TCompiler *compiler);
}

class TranslatorMetalReflection
{
  public:
    TranslatorMetalReflection() : hasUBOs(false), hasFlatInput(false) {}
    ~TranslatorMetalReflection() {}

    void addOriginalName(const size_t id, const std::string &name)
    {
        originalNames.insert({id, name});
    }
    void addSamplerBinding(const std::string &name, size_t samplerBinding)
    {
        samplerBindings.insert({name, samplerBinding});
    }
    void addTextureBinding(const std::string &name, size_t textureBinding)
    {
        textureBindings.insert({name, textureBinding});
    }
    void addRWTextureBinding(int glslImageBinding, int mtlRWTextureBinding)
    {
        bool inserted = rwTextureBindings.insert({glslImageBinding, mtlRWTextureBinding}).second;
        if (!inserted)
        {
            // Shader images are currently only implemented enough to support pixel local storage,
            // which does not allow more than one image to be bound to the same index.
            //
            // NOTE: Pixel local storage also does not allow image bindings to change via
            // glUniform1i, which we do not currently account for in this backend.
            UNIMPLEMENTED();
        }
    }
    void addUserUniformBufferBinding(const std::string &name, size_t userUniformBufferBinding)
    {
        userUniformBufferBindings.insert({name, userUniformBufferBinding});
    }
    void addUniformBufferBinding(const std::string &name, UBOBindingInfo bindingInfo)
    {
        uniformBufferBindings.insert({name, bindingInfo});
    }
    std::string getOriginalName(const size_t id) { return originalNames.at(id); }
    samplerBindingMap getSamplerBindings() const { return samplerBindings; }
    textureBindingMap getTextureBindings() const { return textureBindings; }
    userUniformBufferBindingMap getUserUniformBufferBindings() const
    {
        return userUniformBufferBindings;
    }
    uniformBufferBindingMap getUniformBufferBindings() const { return uniformBufferBindings; }
    size_t getSamplerBinding(const std::string &name) const
    {
        auto it = samplerBindings.find(name);
        if (it != samplerBindings.end())
        {
            return it->second;
        }
        // If we can't find a matching sampler, assert out on Debug, and return an invalid value on
        // release.
        ASSERT(0);
        return std::numeric_limits<size_t>::max();
    }
    size_t getTextureBinding(const std::string &name) const
    {
        auto it = textureBindings.find(name);
        if (it != textureBindings.end())
        {
            return it->second;
        }
        // If we can't find a matching texture, assert out on Debug, and return an invalid value on
        // release.
        ASSERT(0);
        return std::numeric_limits<size_t>::max();
    }
    int getRWTextureBinding(int glslImageBinding) const
    {
        auto it = rwTextureBindings.find(glslImageBinding);
        if (it != rwTextureBindings.end())
        {
            return it->second;
        }
        // If there isn't a shader image bound to this slot, return -1. This signals to the program
        // that there is nothing here to bind.
        return -1;
    }
    size_t getUserUniformBufferBinding(const std::string &name) const
    {
        auto it = userUniformBufferBindings.find(name);
        if (it != userUniformBufferBindings.end())
        {
            return it->second;
        }
        // If we can't find a matching Uniform binding, assert out on Debug, and return an invalid
        // value.
        ASSERT(0);
        return std::numeric_limits<size_t>::max();
    }
    UBOBindingInfo getUniformBufferBinding(const std::string &name) const
    {
        auto it = uniformBufferBindings.find(name);
        if (it != uniformBufferBindings.end())
        {
            return it->second;
        }
        // If we can't find a matching UBO binding by name, assert out on Debug, and return an
        // invalid value.
        ASSERT(0);
        return {.bindIndex = std::numeric_limits<size_t>::max(),
                .arraySize = std::numeric_limits<size_t>::max()};
    }
    void reset()
    {
        hasUBOs              = false;
        hasFlatInput         = false;
        hasInvariance        = false;
        hasIsnanOrIsinf      = false;
        hasAttributeAliasing = false;
        originalNames.clear();
        samplerBindings.clear();
        textureBindings.clear();
        rwTextureBindings.clear();
        userUniformBufferBindings.clear();
        uniformBufferBindings.clear();
    }

    bool hasUBOs              = false;
    bool hasFlatInput         = false;
    bool hasInvariance        = false;
    bool hasIsnanOrIsinf      = false;
    bool hasAttributeAliasing = false;

  private:
    originalNamesMap originalNames;
    samplerBindingMap samplerBindings;
    textureBindingMap textureBindings;
    rwTextureBindingMap rwTextureBindings;
    userUniformBufferBindingMap userUniformBufferBindings;
    uniformBufferBindingMap uniformBufferBindings;
};

class TranslatorMSL : public TCompiler
{
  public:
    TranslatorMSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);

#ifdef ANGLE_ENABLE_METAL
    TranslatorMSL *getAsTranslatorMSL() override { return this; }
#endif

    TranslatorMetalReflection *getTranslatorMetalReflection() { return &translatorMetalReflection; }

  protected:
    bool translate(TIntermBlock *root,
                   const ShCompileOptions &compileOptions,
                   PerformanceDiagnostics *perfDiagnostics) override;

    [[nodiscard]] bool translateImpl(TInfoSinkBase &sink,
                                     TIntermBlock *root,
                                     const ShCompileOptions &compileOptions,
                                     PerformanceDiagnostics *perfDiagnostics,
                                     SpecConst *specConst,
                                     DriverUniformMetal *driverUniforms);

    [[nodiscard]] bool shouldFlattenPragmaStdglInvariantAll() override;

    [[nodiscard]] bool transformDepthBeforeCorrection(TIntermBlock *root,
                                                      const DriverUniformMetal *driverUniforms);

    [[nodiscard]] bool appendVertexShaderDepthCorrectionToMain(
        TIntermBlock *root,
        const DriverUniformMetal *driverUniforms);

    [[nodiscard]] bool insertSampleMaskWritingLogic(TIntermBlock &root,
                                                    DriverUniformMetal &driverUniforms);
    [[nodiscard]] bool insertRasterizationDiscardLogic(TIntermBlock &root);

    TranslatorMetalReflection translatorMetalReflection = {};
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_H_
