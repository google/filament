// Copyright (c) 2025 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include "TestFixture.h"

#include "glslang/MachineIndependent/LiveTraverser.h"

namespace glslangtest {
namespace {

struct LiveTraverserTestParams {
    std::string fileName;
    std::vector<std::string> liveVars;
};

using LiveTraverserTest = GlslangTest<::testing::TestWithParam<LiveTraverserTestParams>>;

TEST_P(LiveTraverserTest, FromFile)
{
    const auto& fileName = GetParam().fileName;
    const auto& expectedLiveVars = GetParam().liveVars;
    const EShMessages controls = DeriveOptions(Source::GLSL, Semantics::Vulkan, Target::AST);
    GlslangResult result;
    result.validationResult = true;

    std::string contents;
    tryLoadFile(GlobalTestSettings.testRoot + "/" + fileName, "input", &contents);
    std::unique_ptr<glslang::TShader> shader = std::make_unique<glslang::TShader>(GetShaderStage(GetSuffix(fileName)));

    bool success = compile(shader.get(), contents, "", controls);
    result.shaderResults.push_back({fileName, shader->getInfoLog(), shader->getInfoDebugLog()});

    std::ostringstream stream;
    outputResultToStream(&stream, result, controls);

    class TLiveSymbolTraverser : public glslang::TLiveTraverser {
    public:
        TLiveSymbolTraverser(const glslang::TIntermediate& i, std::vector<std::string>& liveVars)
            : glslang::TLiveTraverser(i), liveVars(liveVars)
        {
        }

        virtual void visitSymbol(glslang::TIntermSymbol* symbol)
        {
            if (symbol->getAsSymbolNode()->getAccessName().compare(0, 3, "gl_") == 0)
                return;

            if (symbol->getQualifier().storage == glslang::TStorageQualifier::EvqVaryingIn ||
                symbol->getQualifier().storage == glslang::TStorageQualifier::EvqVaryingOut ||
                symbol->getQualifier().storage == glslang::TStorageQualifier::EvqUniform ||
                symbol->getQualifier().storage == glslang::TStorageQualifier::EvqBuffer) {
                liveVars.push_back(symbol->getAccessName().c_str());
            }
        }

    private:
        std::vector<std::string>& liveVars;
    };

    if (success) {
        std::vector<std::string> actualLiveVars;
        TLiveSymbolTraverser liveTraverser(*shader->getIntermediate(), actualLiveVars);
        liveTraverser.pushFunction(shader->getIntermediate()->getEntryPointMangledName().c_str());
        while (!liveTraverser.destinations.empty()) {
            TIntermNode* destination = liveTraverser.destinations.back();
            liveTraverser.destinations.pop_back();
            destination->traverse(&liveTraverser);
        }

        for (const auto& expectedVar : expectedLiveVars) {
            auto it = std::find(actualLiveVars.begin(), actualLiveVars.end(), expectedVar);
            EXPECT_NE(it, actualLiveVars.end());
            if (it != actualLiveVars.end())
                actualLiveVars.erase(it);
        }
        EXPECT_TRUE(actualLiveVars.empty());
    }

    // Check with expected results.
    const std::string expectedOutputFname = GlobalTestSettings.testRoot + "/baseResults/" + fileName + ".out";
    std::string expectedOutput;
    tryLoadFile(expectedOutputFname, "expected output", &expectedOutput);

    checkEqAndUpdateIfRequested(expectedOutput, stream.str(), expectedOutputFname, result.spirvWarningsErrors);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    Glsl, LiveTraverserTest,
    ::testing::ValuesIn(std::vector<LiveTraverserTestParams>({
        {"liveTraverser.switch.vert", {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10"}},
        // TODO: implement test for if statements
    }))
);
// clang-format on

} // anonymous namespace
} // namespace glslangtest
