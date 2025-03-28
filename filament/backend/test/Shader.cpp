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

Shader::Shader(DriverApi& api, Cleanup& cleanup, ShaderConfig config) {
    utils::FixedCapacityVector<DescriptorSetLayoutBinding> kLayouts(config.uniformNames.size());
    for (unsigned char i = 0; i < config.uniformNames.size(); ++i) {
        kLayouts[i] =
                { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, i };
    };

    filamat::DescriptorSets descriptors;
    for (unsigned char i = 0; i < config.uniformNames.size(); ++i) {
        descriptors[i + 1] = {{ config.uniformNames[i], kLayouts[i], {}}};
    }
    ShaderGenerator shaderGen(
            std::move(config.vertexShader), std::move(config.fragmentShader), BackendTest::sBackend,
            BackendTest::sIsMobilePlatform, std::move(descriptors));
    Program prog = shaderGen.getProgram(api);
    for (unsigned char i = 0; i < config.uniformNames.size(); ++i) {
        prog.descriptorBindings(1, {{ config.uniformNames[i], DescriptorType::UNIFORM_BUFFER, i }});
    }
    mProgram = cleanup.add(api.createProgram(std::move(prog)));

    mDescriptorSetLayout = cleanup.add(
            api.createDescriptorSetLayout(DescriptorSetLayout{ kLayouts }));

    mDescriptorSet = cleanup.add(api.createDescriptorSet(mDescriptorSetLayout));
}

filament::backend::ProgramHandle Shader::getProgram() const {
    return mProgram;
}

filament::backend::DescriptorSetLayoutHandle Shader::getDescriptorSetLayout() const {
    return mDescriptorSetLayout;
}

filament::backend::DescriptorSetHandle Shader::getDescriptorSet() const {
    return mDescriptorSet;
}

} // namespace test
