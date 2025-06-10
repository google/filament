/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "Shader.h"

#include "BackendTest.h"
#include "ShaderGenerator.h"

namespace test {

using namespace filament::backend;

Shader::Shader(DriverApi& api, Cleanup& cleanup, ShaderConfig config) : mCleanup(cleanup) {
    utils::FixedCapacityVector<DescriptorSetLayoutBinding> kLayouts(config.uniforms.size());
    for (unsigned char i = 0; i < config.uniforms.size(); ++i) {
        kLayouts[i] = {
                config.uniforms[i].type.value_or(DescriptorType::UNIFORM_BUFFER),
                ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, i };
    }

    // This assumes that the uniforms will all be in a single descriptor set at index 1.
    // If there are shaders with uniforms in other sets then ShaderConfig will need to be expanded
    // to accommodate that.
    size_t kDescriptorSetIndex = 0;
    filamat::DescriptorSets descriptors;
    descriptors[kDescriptorSetIndex] = filamat::DescriptorSetInfo(config.uniforms.size());
    for (unsigned char i = 0; i < config.uniforms.size(); ++i) {
        descriptors[kDescriptorSetIndex][i] = {
                config.uniforms[i].name, kLayouts[i], config.uniforms[i].samplerInfo };
    }
    // TODO(b/422803382): When the shader language isn't GLSL dont' use the ShaderGenerator to
    //  compile the shader.
    ShaderGenerator shaderGen(
            std::move(config.vertexShader), std::move(config.fragmentShader), BackendTest::sBackend,
            BackendTest::sIsMobilePlatform, std::move(descriptors));
    Program prog = shaderGen.getProgram(api);

    Program::DescriptorBindingsInfo bindingsInfo(config.uniforms.size());
    for (unsigned char i = 0; i < config.uniforms.size(); ++i) {
        bindingsInfo[i] = {
                config.uniforms[i].name,
                config.uniforms[i].type.value_or(DescriptorType::UNIFORM_BUFFER), i };
    }
    prog.descriptorBindings(0, bindingsInfo);
    mProgram = cleanup.add(api.createProgram(std::move(prog)));

    if (!kLayouts.empty()) {
        mDescriptorSetLayout = cleanup.add(
                api.createDescriptorSetLayout(DescriptorSetLayout{ .bindings = kLayouts }));
    }
}

DescriptorSetHandle Shader::createDescriptorSet(DriverApi& api) const {
    return mCleanup.add(api.createDescriptorSet(mDescriptorSetLayout));
}

ProgramHandle Shader::getProgram() const {
    assert(mProgram);
    EXPECT_THAT(mProgram, ::testing::IsTrue())
                        << "Shader program accessed despite being null.";
    return mProgram;
}

DescriptorSetLayoutHandle Shader::getDescriptorSetLayout() const {
    EXPECT_THAT(mDescriptorSetLayout, ::testing::IsTrue())
            << "Shader descriptor set layout accessed despite being null.";
    return mDescriptorSetLayout;
}

void Shader::addProgramToPipelineState(PipelineState& state) const {
    state.program = getProgram();
    // In case another shader was set first, clear the set layout and then set this shader's values.
    state.pipelineLayout.setLayout = PipelineLayout::SetLayout();
    if (mDescriptorSetLayout) {
        state.pipelineLayout.setLayout[0] = { getDescriptorSetLayout() };
    }
}

} // namespace test
