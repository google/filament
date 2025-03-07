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

#include <filamat/Enums.h>
#include <filament/MaterialEnums.h>
#include <filamat/MaterialBuilder.h>

#include <utils/Log.h>

#include "ASTHelpers.h"

// GLSLANG headers
#include <InfoSink.h>
#include <localintermediate.h>

#include "builtinResource.h"

using namespace utils;
using namespace glslang;
using namespace filament::backend;

namespace filamat {

GLSLangCleaner::GLSLangCleaner() {
    mAllocator = &GetThreadPoolAllocator();
}

GLSLangCleaner::~GLSLangCleaner() {
    GetThreadPoolAllocator().pop();
    SetThreadPoolAllocator(mAllocator);
}

// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

static const TIntermTyped* findLValueBase(const TIntermTyped* node, Symbol& symbol);

class SymbolsTracer : public TIntermTraverser {
public:
    explicit SymbolsTracer(std::deque<Symbol>& events) : mEvents(events) {
    }

    // Function call site.
    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() != EOpFunctionCall) {
            return true;
        }

        // Find function name.
        std::string const functionName = node->getName().c_str();

        // Iterate on function parameters.
        for (size_t parameterIdx = 0; parameterIdx < node->getSequence().size(); parameterIdx++) {
            TIntermNode* parameter = node->getSequence().at(parameterIdx);
            // Parameter is not a pure symbol. It is indexed or swizzled.
            if (parameter->getAsBinaryNode()) {
                Symbol symbol;
                std::vector<Symbol> events;
                const TIntermTyped* n = findLValueBase(parameter->getAsBinaryNode(), symbol);
                if (n != nullptr && n->getAsSymbolNode() != nullptr) {
                    const TString& symbolTString = n->getAsSymbolNode()->getName();
                    symbol.setName(symbolTString.c_str());
                    events.push_back(symbol);
                }

                for (Symbol symbol : events) {
                    Access const fCall = { Access::FunctionCall, functionName, parameterIdx };
                    symbol.add(fCall);
                    mEvents.push_back(symbol);
                }
            }
            // Parameter is a pure symbol.
            if (parameter->getAsSymbolNode()) {
                Symbol s(parameter->getAsSymbolNode()->getName().c_str());
                Access const fCall = {Access::FunctionCall, functionName, parameterIdx};
                s.add(fCall);
                mEvents.push_back(s);
            }
        }

        return true;
    }

    // Assign operations
    bool visitBinary(TVisit, TIntermBinary* node) override {
        TOperator const op = node->getOp();
        Symbol symbol;
        if (op == EOpAssign || op == EOpAddAssign || op == EOpDivAssign || op == EOpSubAssign
            || op == EOpMulAssign ) {
            const TIntermTyped* n = findLValueBase(node->getLeft(), symbol);
            if (n != nullptr && n->getAsSymbolNode() != nullptr) {
                const TString& symbolTString = n->getAsSymbolNode()->getName();
                symbol.setName(symbolTString.c_str());
                mEvents.push_back(symbol);
                return false; // Don't visit subtree since we just traced it with findLValueBase()
            }
        }
        return true;
    }

private:
    std::deque<Symbol>& mEvents;
};

// Meant to explore the Lvalue in an assignment. Depth traverse the left child of an assignment
// binary node to find out the symbol and all access applied on it.
static const TIntermTyped* findLValueBase(const TIntermTyped* node, Symbol& symbol) {
    do {
        // Make sure we have a binary node
        const TIntermBinary* binary = node->getAsBinaryNode();
        if (binary == nullptr) {
            return node;
        }

        // Check Operator
        TOperator const op = binary->getOp();
        if (op != EOpIndexDirect && op != EOpIndexIndirect && op != EOpIndexDirectStruct
            && op != EOpVectorSwizzle && op != EOpMatrixSwizzle) {
            return nullptr;
        }
        Access access;
        if (op == EOpIndexDirectStruct) {
            access.string = ASTHelpers::getIndexDirectStructString(*binary);
            access.type = Access::DirectIndexForStruct;
        } else {
            access.string = ASTHelpers::to_string(op) ;
            access.type = Access::Swizzling;
        }
        symbol.add(access);
        node = node->getAsBinaryNode()->getLeft();
    } while (true);
}

// ------------------------------------------------------------------------------------------------

bool GLSLTools::analyzeComputeShader(const std::string& shaderCode,
        filament::backend::ShaderModel model, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage) noexcept {

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangCompute);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner const cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages const msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool const ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse compute shader:" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return false;
    }

    auto materialFunctionName = getMaterialFunctionName(filament::MaterialDomain::COMPUTE);

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTHelpers::getFunctionByNameOnly(materialFunctionName, *root);
    if (materialFctNode == nullptr) {
        utils::slog.e << "ERROR: Invalid compute shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find " << materialFunctionName << "() function" << utils::io::endl;
        return false;
    }

    return true;
}

std::optional<GLSLTools::FragmentShaderInfo> GLSLTools::analyzeFragmentShader(
        const std::string& shaderCode,
        filament::backend::ShaderModel model, MaterialBuilder::MaterialDomain materialDomain,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        bool hasCustomSurfaceShading) noexcept {

    assert_invariant(materialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangFragment);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner const cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages const msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool const ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse fragment shader:" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return std::nullopt;
    }

    auto materialFunctionName = getMaterialFunctionName(materialDomain);

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTHelpers::getFunctionByNameOnly(materialFunctionName, *root);
    if (materialFctNode == nullptr) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find " << materialFunctionName << "() function" << utils::io::endl;
        return std::nullopt;
    }

    FragmentShaderInfo result {
        .userMaterialHasCustomDepth = GLSLTools::hasCustomDepth(root, materialFctNode)
    };

    // If this is a post-process material, at this point we've successfully met all the
    // requirements.
    if (materialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return result;
    }

    // Check there is a prepareMaterial function definition in this shader.
    TIntermAggregate* prepareMaterialNode =
            ASTHelpers::getFunctionByNameOnly("prepareMaterial", *root);
    if (prepareMaterialNode == nullptr) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: Unable to find prepareMaterial() function" << utils::io::endl;
        return std::nullopt;
    }

    std::string_view const prepareMaterialSignature = prepareMaterialNode->getName();
    bool const prepareMaterialCalled = ASTHelpers::isFunctionCalled(
            prepareMaterialSignature, *materialFctNode, *root);
    if (!prepareMaterialCalled) {
        utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
        utils::slog.e << "ERROR: prepareMaterial() is not called" << utils::io::endl;
        return std::nullopt;
    }

    if (hasCustomSurfaceShading) {
        materialFctNode = ASTHelpers::getFunctionByNameOnly("surfaceShading", *root);
        if (materialFctNode == nullptr) {
            utils::slog.e << "ERROR: Invalid fragment shader:" << utils::io::endl;
            utils::slog.e << "ERROR: Unable to find surfaceShading() function"
                          << utils::io::endl;
            return std::nullopt;
        }
    }

    return result;
}

bool GLSLTools::analyzeVertexShader(const std::string& shaderCode,
        filament::backend::ShaderModel model,
        MaterialBuilder::MaterialDomain materialDomain, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage) noexcept {

    assert_invariant(materialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    // TODO: After implementing post-process vertex shaders, properly analyze them here.
    if (materialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return true;
    }

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    TShader tShader(EShLanguage::EShLangVertex);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner const cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages const msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    bool const ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        utils::slog.e << "ERROR: Unable to parse vertex shader" << utils::io::endl;
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return false;
    }

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTHelpers::getFunctionByNameOnly("materialVertex", *root);
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

    GLSLangCleaner const cleaner;
    const int version = getGlslDefaultVersion(model);
    EShMessages const msg = glslangFlagsFromTargetApi(targetApi, targetLanguage);
    const TBuiltInResource* builtins = &DefaultTBuiltInResource;
    bool const ok = tShader.parse(builtins, version, false, msg);
    if (!ok) {
        // Even with all properties set the shader doesn't build. This is likely a syntax error
        // with user provided code.
        utils::slog.e << tShader.getInfoLog() << utils::io::endl;
        return false;
    }

    TIntermNode* rootNode = tShader.getIntermediate()->getTreeRoot();

    std::string_view const mainFunction(type == ShaderStage::FRAGMENT ?
            "material" : "materialVertex");

    TIntermAggregate* functionMaterialDef = ASTHelpers::getFunctionByNameOnly(mainFunction, *rootNode);
    std::string_view const materialFullyQualifiedName = functionMaterialDef->getName();
    return findPropertyWritesOperations(materialFullyQualifiedName, 0, rootNode, properties);
}

bool GLSLTools::findPropertyWritesOperations(std::string_view functionName, size_t parameterIdx,
        TIntermNode* rootNode, MaterialBuilder::PropertyList& properties) const noexcept {

    glslang::TIntermAggregate* functionMaterialDef =
            ASTHelpers::getFunctionBySignature(functionName, *rootNode);
    if (functionMaterialDef == nullptr) {
        utils::slog.e << "Unable to find function '" << functionName << "' definition."
                << utils::io::endl;
        return false;
    }

    std::vector<ASTHelpers::FunctionParameter> functionMaterialParameters;
    ASTHelpers::getFunctionParameters(functionMaterialDef, functionMaterialParameters);

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
    ASTHelpers::FunctionParameter::Qualifier const qualifier =
            functionMaterialParameters.at(parameterIdx).qualifier;
    if (qualifier == ASTHelpers::FunctionParameter::IN ||
        qualifier == ASTHelpers::FunctionParameter::CONST) {
        return true;
    }

    std::deque<Symbol> symbols;
    findSymbolsUsage(functionName, *rootNode, symbols);

    // Iterate over symbols to see if the parameter we are interested in what written.
    std::string const parameterName = functionMaterialParameters.at(parameterIdx).name;
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
                        ASTHelpers::getFunctionBySignature(access.string, *rootNode);
                std::vector<ASTHelpers::FunctionParameter> functionCallParameters;
                ASTHelpers::getFunctionParameters(functionCall, functionCallParameters);

                ASTHelpers::FunctionParameter const& parameter =
                        functionCallParameters.at(access.parameterIdx);
                if (parameter.qualifier == ASTHelpers::FunctionParameter::OUT ||
                    parameter.qualifier == ASTHelpers::FunctionParameter::INOUT) {
                    const std::string& propName = symbol.getDirectIndexStructName();
                    if (Enums::isValid<Property>(propName)) {
                        MaterialBuilder::Property const p = Enums::toEnum<Property>(propName);
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
                MaterialBuilder::Property const p = Enums::toEnum<Property>(access.string);
                properties[size_t(p)] = true;
            }
            return;
        }

        // Swizzling only happens at the end of the access chain and is ignored.
    }
}

bool GLSLTools::findSymbolsUsage(std::string_view functionSignature, TIntermNode& root,
        std::deque<Symbol>& symbols) noexcept {
    TIntermNode* functionAST = ASTHelpers::getFunctionBySignature(functionSignature, root);
    SymbolsTracer variableTracer(symbols);
    functionAST->traverse(&variableTracer);
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
            switch (featureLevel) {
                case FeatureLevel::FEATURE_LEVEL_0:     return { 100, true };
                case FeatureLevel::FEATURE_LEVEL_1:     return { 300, true };
                case FeatureLevel::FEATURE_LEVEL_2:     return { 310, true };
                case FeatureLevel::FEATURE_LEVEL_3:     return { 310, true };
            }
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
            if (targetApi == TargetApi::WEBGPU) {
                // FIXME: We have to use EShMsgVulkanRules for WEBGPU, otherwise compilation will
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
            case MaterialBuilderBase::TargetApi::WEBGPU:
            case MaterialBuilderBase::TargetApi::VULKAN:
            case MaterialBuilderBase::TargetApi::METAL:
                shader.setEnvInput(EShSourceGlsl, stage, EShClientVulkan, version);
                shader.setEnvClient(EShClientVulkan, EShTargetVulkan_1_1);
                break;
            // TODO: Handle webgpu here
            case MaterialBuilderBase::TargetApi::ALL:
                // can't happen
                break;
        }
        shader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_3);
    }
}

void GLSLTools::textureLodBias(TShader& shader) {
    TIntermediate* intermediate = shader.getIntermediate();
    TIntermNode* root = intermediate->getTreeRoot();
    textureLodBias(intermediate, root,
            "material(struct-MaterialInputs",
            "filament_lodBias");
}

template<typename F>
class AggregateTraverserAdapter : public glslang::TIntermTraverser {
    F closure;
public:
    explicit AggregateTraverserAdapter(F closure)
            : TIntermTraverser(true, false, false, false),
              closure(closure) { }

    bool visitAggregate(glslang::TVisit visit, glslang::TIntermAggregate* node) override {
        return closure(visit, node);
    }
};

template<typename F>
void traverseAggregate(TIntermNode* root, F&& closure) {
    AggregateTraverserAdapter adapter(std::forward<std::decay_t<F>>(closure));
    root->traverse(&adapter);
}

void GLSLTools::textureLodBias(TIntermediate* intermediate, TIntermNode* root,
        const char* entryPointSignatureish, const char* lodBiasSymbolName) noexcept {

    // First, find the "lodBias" symbol and entry point
    const std::string functionName{ entryPointSignatureish };
    TIntermSymbol* pIntermSymbolLodBias = nullptr;
    TIntermNode* pEntryPointRoot = nullptr;
    traverseAggregate(root,
            [&](TVisit, TIntermAggregate* node) {
                if (node->getOp() == glslang::EOpSequence) {
                    return true;
                }
                if (node->getOp() == glslang::EOpFunction) {
                    if (node->getName().rfind(functionName, 0) == 0) {
                        pEntryPointRoot = node;
                    }
                    return false;
                }
                if (node->getOp() == glslang::EOpLinkerObjects) {
                    for (TIntermNode* item: node->getSequence()) {
                        TIntermSymbol* symbol = item->getAsSymbolNode();
                        if (symbol && symbol->getBasicType() == TBasicType::EbtFloat) {
                            if (symbol->getName() == lodBiasSymbolName) {
                                pIntermSymbolLodBias = symbol;
                                break;
                            }
                        }
                    }
                }
                return true;
            });

    if (!pEntryPointRoot) {
        // This can happen if the material doesn't have user defined code,
        // e.g. with the depth material. We just do nothing then.
        return;
    }

    if (!pIntermSymbolLodBias) {
        // something went wrong
        utils::slog.e << "lod bias ignored because \"" << lodBiasSymbolName << "\" was not found!"
                      << utils::io::endl;
        return;
    }

    // add lod bias to texture calls
    // we need to run this only from the user's main entry point
    traverseAggregate(pEntryPointRoot,
            [&](TVisit, TIntermAggregate* node) {
                // skip everything that's not a texture() call
                if (node->getOp() != glslang::EOpTexture) {
                    return true;
                }

                TIntermSequence& sequence = node->getSequence();

                // first check that we have the correct sampler
                TIntermTyped* pTyped = sequence[0]->getAsTyped();
                if (!pTyped) {
                    return false;
                }

                TSampler const& sampler = pTyped->getType().getSampler();
                if (sampler.isArrayed() && sampler.isShadow()) {
                    // sampler2DArrayShadow is not supported
                    return false;
                }

                // Then add the lod bias to the texture() call
                if (sequence.size() == 2) {
                    // we only have 2 parameters, add the 3rd one
                    TIntermSymbol* symbol = intermediate->addSymbol(*pIntermSymbolLodBias);
                    sequence.push_back(symbol);
                } else if (sequence.size() == 3) {
                    // load bias is already specified
                    TIntermSymbol* symbol = intermediate->addSymbol(*pIntermSymbolLodBias);
                    TIntermTyped* pAdd = intermediate->addBinaryMath(TOperator::EOpAdd,
                            sequence[2]->getAsTyped(), symbol,
                            node->getLoc());
                    sequence[2] = pAdd;
                }

                return false;
            });
}

bool GLSLTools::hasCustomDepth(TIntermNode* root, TIntermNode* entryPoint) {

    class HasCustomDepth : public glslang::TIntermTraverser {
        using TVisit = glslang::TVisit;
        TIntermNode* const root;        // shader root
        bool hasCustomDepth = false;

    public:
        bool operator()(TIntermNode* entryPoint) noexcept {
            entryPoint->traverse(this);
            return hasCustomDepth;
        }

        explicit HasCustomDepth(TIntermNode* root) : root(root) {}

        bool visitAggregate(TVisit, TIntermAggregate* node) override {
            if (node->getOp() == EOpFunctionCall) {
                // we have a function call, "recurse" into it to see if we call discard or
                // write to gl_FragDepth.

                // find the entry point corresponding to that call
                TIntermNode* const entryPoint =
                        ASTHelpers::getFunctionBySignature(node->getName(), *root);

                // this should never happen because the shader has already been validated
                assert_invariant(entryPoint);

                hasCustomDepth = hasCustomDepth || HasCustomDepth{ root }(entryPoint);

                return !hasCustomDepth;
            }
            return true;
        }

        // this checks if we write gl_FragDepth
        bool visitBinary(TVisit, glslang::TIntermBinary* node) override {
            TOperator const op = node->getOp();
            Symbol symbol;
            if (op == EOpAssign ||
                op == EOpAddAssign ||
                op == EOpDivAssign ||
                op == EOpSubAssign ||
                op == EOpMulAssign) {
                const TIntermTyped* n = findLValueBase(node->getLeft(), symbol);
                if (n != nullptr && n->getAsSymbolNode() != nullptr) {
                    const TString& symbolTString = n->getAsSymbolNode()->getName();
                    if (symbolTString == "gl_FragDepth") {
                        hasCustomDepth = true;
                    }
                    // Don't visit subtree since we just traced it with findLValueBase()
                    return false;
                }
            }
            return true;
        }

        // this check if we call `discard`
        bool visitBranch(TVisit, glslang::TIntermBranch* branch) override {
            if (branch->getFlowOp() == EOpKill) {
                hasCustomDepth = true;
                return false;
            }
            return true;
        }

    } hasCustomDepth(root);

    return hasCustomDepth(entryPoint);
}

} // namespace filamat
