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

#ifndef TNT_STATICCODEANALYZER_H
#define TNT_STATICCODEANALYZER_H

#include <deque>
#include <list>
#include <optional>
#include <set>
#include <string>

#include <filamat/MaterialBuilder.h>

#include <ShaderLang.h>

class TIntermNode;

namespace glslang {
class TPoolAllocator;
}

namespace filamat {

// Used for symbol tracking during static code analysis.
struct Access {
    enum Type {
        Swizzling, DirectIndexForStruct, FunctionCall
    };
    Type type;
    std::string string;
    size_t parameterIdx = 0; // Only used when type == FunctionCall;
};

// Record of symbol interactions in a statement involving a symbol. Can track a sequence of up to
// (and in this order):
// Function call: foo(material)
// DirectIndexForStruct e.g: material.baseColor
// Swizzling e.g: material.baseColor.xyz
// Combinations are possible. e.g: foo(material.baseColor.xyz)
class Symbol {
public:
    Symbol() = default;
    explicit Symbol(const std::string& name) {
        mName = name;
    }

    std::string& getName() {
        return mName;
    }

    std::list<Access>& getAccesses() {
        return mAccesses;
    };

    void setName(const std::string& name) {
        mName = name;
    }

    void add(const Access& access) {
        mAccesses.push_front(access);
    }

    std::string toString() const {
        std::string str(mName);
        for (const Access& access: mAccesses) {
            str += ".";
            str += access.string;
        }
        return str;
    }

    bool hasDirectIndexForStruct() const noexcept {
        return std::any_of(mAccesses.begin(), mAccesses.end(), [](auto&& access) {
            return access.type == Access::Type::DirectIndexForStruct;
        });
    }

    std::string getDirectIndexStructName() const noexcept {
        for (const Access& access : mAccesses) {
            if (access.type == Access::Type::DirectIndexForStruct) {
                return access.string;
            }
        }
        return "";
    }

private:
    std::list<Access> mAccesses;
    std::string mName;
};

class GLSLangCleaner {
public:
    GLSLangCleaner();
    ~GLSLangCleaner();

private:
    glslang::TPoolAllocator* mAllocator;
};

class GLSLTools {
public:
    static void init();
    static void shutdown();

    struct FragmentShaderInfo {
        bool userMaterialHasCustomDepth = false;
    };

    // Return true if:
    // The shader is syntactically and semantically valid AND
    // The shader features a material() function AND
    // The shader features a prepareMaterial() function AND
    // prepareMaterial() is called at some point in material() call chain.
    static std::optional<FragmentShaderInfo> analyzeFragmentShader(const std::string& shaderCode,
            filament::backend::ShaderModel model, MaterialBuilder::MaterialDomain materialDomain,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            bool hasCustomSurfaceShading) noexcept;

    static bool analyzeVertexShader(const std::string& shaderCode,
            filament::backend::ShaderModel model,
            MaterialBuilder::MaterialDomain materialDomain, MaterialBuilder::TargetApi targetApi,
            MaterialBuilder::TargetLanguage targetLanguage) noexcept;

    static bool analyzeComputeShader(const std::string& shaderCode,
            filament::backend::ShaderModel model, MaterialBuilder::TargetApi targetApi,
            MaterialBuilder::TargetLanguage targetLanguage) noexcept;

        // Public for unit tests.
    using Property = MaterialBuilder::Property;
    using ShaderModel = filament::backend::ShaderModel;
    // Use static code analysis on the fragment shader AST to guess properties used in user provided
    // glgl code. Populate properties accordingly.
    bool findProperties(
            filament::backend::ShaderStage type,
            const std::string& shaderCode,
            MaterialBuilder::PropertyList& properties,
            MaterialBuilder::TargetApi targetApi = MaterialBuilder::TargetApi::OPENGL,
            MaterialBuilder::TargetLanguage targetLanguage = MaterialBuilder::TargetLanguage::GLSL,
            ShaderModel model = ShaderModel::DESKTOP) const noexcept;

    // use 100 for ES environment, 110 for desktop; this is the GLSL version, not SPIR-V or Vulkan
    // this is intended to be used with glslang's parse() method, which will figure out the actual
    // version.
    static int getGlslDefaultVersion(filament::backend::ShaderModel model);

    // The shading language version. Corresponds to #version $VALUE.
    // returns the version and a boolean (true for essl, false for glsl)
    static std::pair<int, bool> getShadingLanguageVersion(
            filament::backend::ShaderModel model,
            filament::backend::FeatureLevel featureLevel);

    static EShMessages glslangFlagsFromTargetApi(MaterialBuilder::TargetApi targetApi,
            MaterialBuilder::TargetLanguage targetLanguage);

    static void prepareShaderParser(MaterialBuilder::TargetApi targetApi,
            MaterialBuilder::TargetLanguage targetLanguage, glslang::TShader& shader,
            EShLanguage stage, int version);

    static void textureLodBias(glslang::TShader& shader);

    static bool hasCustomDepth(TIntermNode* root, TIntermNode* entryPoint);


private:
    // Traverse a function definition and retrieve all symbol written to and all symbol passed down
    // in a function call.
    // Start in the function matching the signature provided and follow all out and inout calls.
    // Does NOT recurse to follow function calls.
    static bool findSymbolsUsage(std::string_view functionSignature, TIntermNode& root,
            std::deque<Symbol>& symbols) noexcept;


    // Determine how a function affect one of its parameter by following all write and function call
    // operations on that parameter. e.g to follow material(in out  MaterialInputs), call
    // findPropertyWritesOperations("material", 0, ...);
    // Does nothing if the parameter is not marked as OUT or INOUT
    bool findPropertyWritesOperations(std::string_view functionName, size_t parameterIdx,
            TIntermNode* rootNode, MaterialBuilder::PropertyList& properties) const noexcept;

    // Look at a symbol access and find out if it affects filament MaterialInput fields. Will follow
    // function calls if necessary.
    void scanSymbolForProperty(Symbol& symbol, TIntermNode* rootNode,
            MaterialBuilder::PropertyList& properties) const noexcept;

    // add lod bias to texture() calls
    static void textureLodBias(glslang::TIntermediate* intermediate, TIntermNode* root,
            const char* entryPointSignatureish, const char* lodBiasSymbolName) noexcept;
};

} // namespace filamat

#endif //TNT_STATICCODEANALYZER_H
