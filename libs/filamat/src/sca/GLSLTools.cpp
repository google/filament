/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GLSLTools.h"

#include <filament/MaterialEnums.h>
#include <filamat/MaterialBuilder.h>
#include "../shaders/MaterialInfo.h"

#include <utils/Log.h>

#include "filamat/Enums.h"

#include "ASTHelpers.h"

// GLSLANG headers
#include <InfoSink.h>
#include <localintermediate.h>

#include "builtinResource.h"

using namespace utils;
using namespace glslang;
using namespace ASTUtils;
using namespace filament::backend;

namespace filamat {

GLSLangCleaner::GLSLangCleaner() {
    mAllocator = &GetThreadPoolAllocator();
}

GLSLangCleaner::~GLSLangCleaner() {
    GetThreadPoolAllocator().pop();
    SetThreadPoolAllocator(mAllocator);
}

static std::string_view getMaterialFunctionName(MaterialBuilder::MaterialDomain domain) noexcept {
    switch (domain) {
        case MaterialBuilder::MaterialDomain::SURFACE:
            return "material";
        case MaterialBuilder::MaterialDomain::POST_PROCESS:
            return "postProcess";
        case MaterialBuilder::MaterialDomain::COMPUTE:
            return "compute";
    }
};

bool GLSLTools::analyzeComputeShader(const std::string& shaderCode,
        filament::backend::ShaderModel model, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& info) noexcept {

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangCompute);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse compute shader:" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::flush;
        return false;
    }

    auto materialFunctionName = getMaterialFunctionName(filament::MaterialDomain::COMPUTE);

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTUtils::getFunctionByNameOnly(materialFunctionName, *root);
    if (materialFctNode == nullptr) {
        utils::slog.e << "ERROR: Invalid compute shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find " << materialFunctionName << "() function" << utils::io::endl;
        return false;
    }

    return true;
}

bool GLSLTools::analyzeFragmentShader(const std::string& shaderCode,
        filament::backend::ShaderModel model, MaterialBuilder::MaterialDomain materialDomain,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        bool hasCustomSurfaceShading, MaterialInfo const& info) noexcept {

    assert_invariant(materialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangFragment);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse fragment shader:" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::flush;
        return false;
    }

    auto materialFunctionName = getMaterialFunctionName(materialDomain);

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTUtils::getFunctionByNameOnly(materialFunctionName, *root);
    if (materialFctNode == nullptr) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find " << materialFunctionName << "() function" << utils::io::endl;
        return false;
    }

    // If this is a post-process material, at this point we've successfully met all the
    // requirements.
    if (materialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return true;
    }

    // Check there is a prepareMaterial function definition in this shader.
    TIntermAggregate* prepareMaterialNode =
            ASTUtils::getFunctionByNameOnly("prepareMaterial", *root);
    if (prepareMaterialNode == nullptr) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find prepareMaterial() function" << utils::io::endl;
        return false;
    }

    std::string_view prepareMaterialSignature = prepareMaterialNode->getName();
    bool prepareMaterialCalled = isFunctionCalled(prepareMaterialSignature,
            *materialFctNode, *root);
    if (!prepareMaterialCalled) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: prepareMaterial() is not called" << utils::io::endl;
        return false;
    }

    if (hasCustomSurfaceShading) {
        materialFctNode = ASTUtils::getFunctionByNameOnly("surfaceShading", *root);
        if (materialFctNode == nullptr) {
            utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
            utils::slog.e << "ERROR: Unable to find surfaceShading() function"
                          << utils::io::endl;
            return false;
        }
    }

    return true;
}

bool GLSLTools::analyzeVertexShader(const std::string& shaderCode,
        filament::backend::ShaderModel model,
        MaterialBuilder::MaterialDomain materialDomain, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& info) noexcept {

    assert_invariant(materialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    // TODO: After implementing post-process vertex shaders, properly analyze them here.
    if (materialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return true;
    }

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangVertex);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse vertex shader" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::flush;
        return false;
    }

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTUtils::getFunctionByNameOnly("materialVertex", *root);
    if (materialFctNode == nullptr) {
        utils::slog.e << "ERROR: Invalid vertex shader" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find materialVertex() function" << utils::io::endl;
        return false;
    }

    return true;
}

void GLSLTools::init() {
    // Each call to InitializeProcess must be matched with a call to FinalizeProcess.
    InitializeProcess();
}

void GLSLTools::shutdown() {
    FinalizeProcess();
}

bool GLSLTools::findProperties(
        filament::backend::ShaderStage type,
        const std::string& shaderCode,
        MaterialBuilder::PropertyList& properties,
        MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage,
        ShaderModel model) const noexcept {
    const char* shaderCString = shaderCode.c_str();

    auto getShaderStage = [](ShaderStage type) {
        switch (type) {
            case ShaderStage::VERTEX:   return EShLanguage::EShLangVertex;
            case ShaderStage::FRAGMENT: return EShLanguage::EShLangFragment;
            case ShaderStage::COMPUTE:  return EShLanguage::EShLangCompute;
        }
    };

    TShader tShader(getShaderStage(type));
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    const TBuiltInResource* builtins = &DefaultTBuiltInResource;
    bool ok = tShader.parse(builtins, version, false, msg);
    if (!ok) {
        // Even with all properties set the shader doesn't build. This is likely a syntax error
        // with user provided code.
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return false;
    }

    TIntermNode* rootNode = tShader.getIntermediate()->getTreeRoot();

    std::string_view mainFunction(type == ShaderStage::FRAGMENT ?
            "material" : "materialVertex");

    TIntermAggregate* functionMaterialDef = ASTUtils::getFunctionByNameOnly(mainFunction, *rootNode);
    std::string_view materialFullyQualifiedName = functionMaterialDef->getName();
    return findPropertyWritesOperations(materialFullyQualifiedName, 0, rootNode, properties);
}

bool GLSLTools::findPropertyWritesOperations(std::string_view functionName, size_t parameterIdx,
        TIntermNode* rootNode, MaterialBuilder::PropertyList& properties) const noexcept {

    glslang::TIntermAggregate* functionMaterialDef =
            ASTUtils::getFunctionBySignature(functionName, *rootNode);
    if (functionMaterialDef == nullptr) {
        utils::slog.e << "Unable to find function '" << functionName << "' definition."
                << utils::io::endl;
        return false;
    }

    std::vector<FunctionParameter> functionMaterialParameters;
    ASTUtils::getFunctionParameters(functionMaterialDef, functionMaterialParameters);

    if (functionMaterialParameters.size() <= parameterIdx) {
        utils::slog.e << "Unable to find function '" << functionName <<  "' parameterIndex: " <<
                parameterIdx << utils::io::endl;
        return false;
    }

    // The function has no instructions, it cannot write properties, let's skip all the work
    if (functionMaterialDef->getSequence().size() < 2) {
        return true;
    }

    // Make sure the parameter is either out or inout. Othwerise (const or in), there is no point
    // tracing its usage.
    FunctionParameter::Qualifier qualifier = functionMaterialParameters.at(parameterIdx).qualifier;
    if (qualifier == FunctionParameter::IN || qualifier == FunctionParameter::CONST) {
        return true;
    }

    std::deque<Symbol> symbols;
    findSymbolsUsage(functionName, *rootNode, symbols);

    // Iterate over symbols to see if the parameter we are interested in what written.
    std::string parameterName = functionMaterialParameters.at(parameterIdx).name;
    for (Symbol symbol: symbols) {
        // This is not the symbol we are interested in.
        if (symbol.getName() != parameterName) {
            continue;
        }

        // This is a direct assignment of the variable. X =
        if (symbol.getAccesses().empty()) {
            continue;
        }

        scanSymbolForProperty(symbol, rootNode, properties);
    }
    return true;
}

void GLSLTools::scanSymbolForProperty(Symbol& symbol,
        TIntermNode* rootNode,
        MaterialBuilder::PropertyList& properties) const noexcept {
    for (const Access& access : symbol.getAccesses()) {
        if (access.type == Access::Type::FunctionCall) {
            // Do NOT look into prepareMaterial call.
            if (access.string.find("prepareMaterial(struct") != std::string::npos) {
                continue;
            }
            // If the full symbol is passed, we need to look inside the function to known
            // how it is used. Otherwise, if a DirectIndexForStruct is passed, we can just check
            // if the parameter is out or inout.
            if (symbol.hasDirectIndexForStruct()) {
                TIntermAggregate* functionCall =
                        ASTUtils::getFunctionBySignature(access.string, *rootNode);
                std::vector<FunctionParameter> functionCallParameters;
                ASTUtils::getFunctionParameters(functionCall, functionCallParameters);

                FunctionParameter& parameter = functionCallParameters.at(access.parameterIdx);
                if (parameter.qualifier == FunctionParameter::OUT || parameter.qualifier ==
                        FunctionParameter::INOUT) {
                    const std::string& propName = symbol.getDirectIndexStructName();
                    if (Enums::isValid<Property>(propName)) {
                        MaterialBuilder::Property p = Enums::toEnum<Property>(propName);
                        properties[size_t(p)] = true;
                    }
                }
            } else {
                findPropertyWritesOperations(access.string, access.parameterIdx, rootNode,
                        properties);
            }
            return;
        }

        // If DirectIndexForStruct, issue the appropriate setProperty.
        if (access.type == Access::Type::DirectIndexForStruct) {
            if (Enums::isValid<Property>(access.string)) {
                MaterialBuilder::Property p = Enums::toEnum<Property>(access.string);
                properties[size_t(p)] = true;
            }
            return;
        }

        // Swizzling only happens at the end of the access chain and is ignored.
    }
}

bool GLSLTools::findSymbolsUsage(std::string_view functionSignature, TIntermNode& root,
        std::deque<Symbol>& symbols) noexcept {
    TIntermNode* functionAST = ASTUtils::getFunctionBySignature(functionSignature, root);
    ASTUtils::traceSymbols(*functionAST, symbols);
    return true;
}

// use 100 for ES environment, 110 for desktop; this is the GLSL version, not SPIR-V or Vulkan
// this is intended to be used with glslang's parse() method, which will figure out the actual
// version.
int GLSLTools::getGlslDefaultVersion(ShaderModel model) {
        switch (model) {
        case ShaderModel::MOBILE:
            return 100;
        case ShaderModel::DESKTOP:
            return 110;
    }
}

// The shading language version. Corresponds to #version $VALUE.
std::pair<int, bool> GLSLTools::getShadingLanguageVersion(ShaderModel model,
        filament::backend::FeatureLevel featureLevel) {
    using FeatureLevel = filament::backend::FeatureLevel;
    switch (model) {
        case ShaderModel::MOBILE:
            return { featureLevel >= FeatureLevel::FEATURE_LEVEL_2 ? 310 : 300, true };
        case ShaderModel::DESKTOP:
            return { featureLevel >= FeatureLevel::FEATURE_LEVEL_2 ? 430 : 410, false };
    }
}

EShMessages GLSLTools::glslangFlagsFromTargetApi(
        MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage) {
    using TargetApi = MaterialBuilder::TargetApi;
    using TargetLanguage = MaterialBuilder::TargetLanguage;

    switch (targetLanguage) {
        case TargetLanguage::GLSL:
            assert_invariant(targetApi == TargetApi::OPENGL);
            return EShMessages::EShMsgDefault;

        case TargetLanguage::SPIRV:
            // issue messages for SPIR-V generation
            using Type = std::underlying_type_t<EShMessages>;
            auto msg = (Type)EShMessages::EShMsgSpvRules;
            if (targetApi == TargetApi::VULKAN) {
                // issue messages for Vulkan-requirements of GLSL for SPIR-V
                msg |= (Type)EShMessages::EShMsgVulkanRules;
            }
            if (targetApi == TargetApi::METAL) {
                // FIXME: We have to use EShMsgVulkanRules for metal, otherwise compilation will
                //        choke on gl_VertexIndex.
                msg |= (Type)EShMessages::EShMsgVulkanRules;
            }
            return (EShMessages)msg;
    }
}

void GLSLTools::prepareShaderParser(MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, glslang::TShader& shader,
        EShLanguage stage, int version) {
    // We must only set up the SPIRV environment when we actually need to output SPIRV
    if (targetLanguage == MaterialBuilder::TargetLanguage::SPIRV) {
        shader.setAutoMapBindings(true);
        switch (targetApi) {
            case MaterialBuilderBase::TargetApi::OPENGL:
                shader.setEnvInput(EShSourceGlsl, stage, EShClientOpenGL, version);
                shader.setEnvClient(EShClientOpenGL, EShTargetOpenGL_450);
                break;
            case MaterialBuilderBase::TargetApi::VULKAN:
            case MaterialBuilderBase::TargetApi::METAL:
                shader.setEnvInput(EShSourceGlsl, stage, EShClientVulkan, version);
                shader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_0);
                break;
            case MaterialBuilderBase::TargetApi::ALL:
                // can't happen
                break;
        }
        shader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_0);
    }
}

void GLSLTools::textureLodBias(TShader& shader) {
    TIntermediate* intermediate = shader.getIntermediate();
    TIntermNode* root = intermediate->getTreeRoot();
    ASTUtils::textureLodBias(intermediate, root,
            "material(struct-MaterialInputs",
            "filament_lodBias");
}

} // namespace filamat
