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

#include <cstring>

#include <filament/MaterialEnums.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <filamat/MaterialBuilder.h>

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

bool GLSLTools::analyzeFragmentShader(const std::string& shaderCode, ShaderModel model,
        MaterialBuilder::MaterialDomain materialDomain,
        MaterialBuilder::TargetApi targetApi) const noexcept {

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangFragment);
    tShader.setStrings(&shaderCString, 1);

    auto getMaterialFunctionName = [](MaterialBuilder::MaterialDomain domain) {
        switch(domain) {
            case MaterialBuilder::MaterialDomain::SURFACE:
                return "material";

            case MaterialBuilder::MaterialDomain::POST_PROCESS:
                return "postProcess";
        }
    };
    const char* materialFunctionName = getMaterialFunctionName(materialDomain);

    GLSLangCleaner cleaner;
    int version = glslangVersionFromShaderModel(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse fragment shader:" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::flush;
        return false;
    }

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

    // Check there is a prepareMaterial function defintion in this shader.
    TIntermAggregate* prepareMaterialNode = ASTUtils::getFunctionByNameOnly("prepareMaterial",
            *root);
    if (prepareMaterialNode == nullptr) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find prepareMaterial() function" << utils::io::endl;
        return false;
    }

    std::string prepareMaterialSignature = prepareMaterialNode->getName().c_str();
    bool prepareMaterialCalled = isFunctionCalled(prepareMaterialSignature,
            *materialFctNode, *root);
    if (!prepareMaterialCalled) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: prepareMaterial() is not called" << utils::io::endl;
        return false;
    }

    return true;
}

bool GLSLTools::analyzeVertexShader(const std::string& shaderCode, ShaderModel model,
        MaterialBuilder::MaterialDomain materialDomain,
        MaterialBuilder::TargetApi targetApi) const noexcept {

    // TODO: After implementing post-process vertex shaders, properly analyze them here.
    if (materialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return true;
    }

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangVertex);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    int version = glslangVersionFromShaderModel(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi);
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
        filament::backend::ShaderType type,
        const std::string& shaderCode,
        MaterialBuilder::PropertyList& properties,
        MaterialBuilder::TargetApi targetApi,
        ShaderModel model) const noexcept {
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(type == FRAGMENT ?
            EShLanguage::EShLangFragment : EShLanguage::EShLangVertex);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    int version = glslangVersionFromShaderModel(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi);
    const TBuiltInResource* builtins = &DefaultTBuiltInResource;
    bool ok = tShader.parse(builtins, version, false, msg);
    if (!ok) {
        // Even with all properties set the shader doesn't build. This is likely a syntax error
        // with user provided code.
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return false;
    }

    TIntermNode* rootNode = tShader.getIntermediate()->getTreeRoot();

    std::string mainFunction(type == FRAGMENT ?
            "material" : "materialVertex");

    TIntermAggregate* functionMaterialDef = ASTUtils::getFunctionByNameOnly(mainFunction, *rootNode);
    std::string materialFullyQualifiedName = functionMaterialDef->getName().c_str();
    return findPropertyWritesOperations(materialFullyQualifiedName, 0, rootNode, properties);
}

bool GLSLTools::findPropertyWritesOperations(const std::string& functionName, size_t parameterIdx,
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
    bool ok = findSymbolsUsage(functionName, *rootNode, symbols);
    if (!ok) {
        utils::slog.e << "Unable to trace usage of symbols in function '" << functionName
                << utils::io::endl;
        return false;
    }

    // Iterate over symbols to see if the parameter we are interested in what written.
    std::string parameterName = functionMaterialParameters.at(parameterIdx).name;
    for (Symbol symbol: symbols) {
        // This is not the symbol we are interested in.
        if (symbol.getName() != parameterName) {
            continue;
        }

        // This is a direct assignment of the variable. X =
        if (symbol.getAccesses().size() == 0) {
            continue;
        }

        scanSymbolForProperty(symbol, rootNode, properties);
    }
    return true;
}

void GLSLTools::scanSymbolForProperty(Symbol& symbol,
        TIntermNode* rootNode,
        MaterialBuilder::PropertyList& properties) const noexcept {
    for (Access access : symbol.getAccesses()) {
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
                    MaterialBuilder::Property p =
                            Enums::toEnum<Property>(symbol.getDirectIndexStructName());
                    properties[size_t(p)] = true;
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

bool GLSLTools::findSymbolsUsage(const std::string& functionSignature, TIntermNode& root,
        std::deque<Symbol>& symbols) const noexcept {
    TIntermNode* functionAST = ASTUtils::getFunctionBySignature(functionSignature, root);
    ASTUtils::traceSymbols(*functionAST, symbols);
    return true;
}

int GLSLTools::glslangVersionFromShaderModel(ShaderModel model) {
    int version = 110;
    switch (model) {
        case ShaderModel::UNKNOWN:
            break;
        case ShaderModel::GL_ES_30:
            version = 100;
            break;
        case ShaderModel::GL_CORE_41:
            break;
    }
    return version;
}

EShMessages GLSLTools::glslangFlagsFromTargetApi(MaterialBuilder::TargetApi targetApi) {
    EShMessages msg = EShMessages::EShMsgDefault;
    if (targetApi == MaterialBuilder::TargetApi::VULKAN) {
        msg = (EShMessages) (EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules);
    }
    return msg;
}

void GLSLTools::prepareShaderParser(glslang::TShader& shader, EShLanguage language,
        int version, filamat::MaterialBuilder::Optimization optimization) {
    // We must only setup the SPIRV environment when we actually need to output SPIRV
    if (optimization == filamat::MaterialBuilder::Optimization::SIZE ||
            optimization == filamat::MaterialBuilder::Optimization::PERFORMANCE) {
        shader.setAutoMapBindings(true);
        shader.setEnvInput(EShSourceGlsl, language, EShClientVulkan, version);
        shader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_1);
        shader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_3);
    }
}

} // namespace filamat
