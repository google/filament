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
    enum Type {Swizzling, DirectIndexForStruct, FunctionCall};
    Type type;
    std::string string;
    size_t parameterIdx = 0; // Only used when type == FunctionCall;
};

// Record of symbol interactions in a statment involving a symbol. Can track a sequence of up to
// (and in this order):
// Function call: foo(material)
// DirectIndexForStruct e.g: material.baseColor
// Swizzling e.g: material.baseColor.xyz
// Combinations are possible. e.g: foo(material.baseColor.xyz)
class Symbol {
public:
    Symbol() {}
    Symbol(const std::string& name) {
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
        for (Access access: mAccesses) {
            str += ".";
            str += access.string;
        }
        return str;
    }

    bool hasDirectIndexForStruct() const noexcept {
        for (Access access : mAccesses) {
            if (access.type == Access::Type::DirectIndexForStruct) {
                return true;
            }
        }
        return false;
    }

    const std::string getDirectIndexStructName() const noexcept {
        for (Access access : mAccesses) {
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

    // Return true if:
    // The shader is syntactically and semantically valid AND
    // The shader features a material() function AND
    // The shader features a prepareMaterial() function AND
    // prepareMaterial() is called at some point in material() call chain.
    bool analyzeFragmentShader(const std::string& shaderCode,
            filament::backend::ShaderModel model,
            MaterialBuilder::MaterialDomain materialDomain,
            MaterialBuilder::TargetApi targetApi) const noexcept;

    bool analyzeVertexShader(const std::string& shaderCode,
            filament::backend::ShaderModel model,
            MaterialBuilder::MaterialDomain materialDomain,
            MaterialBuilder::TargetApi targetApi) const noexcept;

    // Public for unit tests.
    using Property = MaterialBuilder::Property;
    using ShaderModel = filament::backend::ShaderModel;
    // Use static code analysis on the fragment shader AST to guess properties used in user provided
    // glgl code. Populate properties accordingly.
    bool findProperties(
            filament::backend::ShaderType type,
            const std::string& shaderCode,
            MaterialBuilder::PropertyList& properties,
            MaterialBuilder::TargetApi targetApi = MaterialBuilder::TargetApi::OPENGL,
            ShaderModel model = ShaderModel::GL_CORE_41) const noexcept;

    static int glslangVersionFromShaderModel(filament::backend::ShaderModel model);

    static EShMessages glslangFlagsFromTargetApi(MaterialBuilder::TargetApi targetApi);

    static void prepareShaderParser(glslang::TShader& shader, EShLanguage language,
            int version, MaterialBuilder::Optimization optimization);

private:


    // Traverse a function definition and retrieve all symbol written to and all symbol passed down
    // in a function call.
    // Start in the function matching the signature provided and follow all out and inout calls.
    // Does NOT recurse to follow function calls.
    bool findSymbolsUsage(const std::string& functionSignature, TIntermNode& root,
            std::deque<Symbol>& symbols) const noexcept;


    // Determine how a function affect one of its parameter by following all write and function call
    // operations on that parameter. e.g to follow material(in out  MaterialInputs), call
    // findPropertyWritesOperations("material", 0, ...);
    // Does nothing if the parameter is not marked as OUT or INOUT
    bool findPropertyWritesOperations(const std::string& functionName, size_t parameterIdx,
            TIntermNode* rootNode, MaterialBuilder::PropertyList& properties) const noexcept;

    // Look at a symbol access and find out if it affects filament MaterialInput fields. Will follow
    // function calls if necessary.
    void scanSymbolForProperty(Symbol& symbol, TIntermNode* rootNode,
            MaterialBuilder::PropertyList& properties) const noexcept;

};

} // namespace filamat

#endif //TNT_STATICCODEANALYZER_H
