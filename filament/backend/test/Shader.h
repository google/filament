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

#ifndef TNT_SHADER_H
#define TNT_SHADER_H

#include "Lifetimes.h"
#include "private/filament/SamplerInterfaceBlock.h"

namespace test {

struct UniformConfig {
    utils::CString name;
    // If not specified this will be DescriptorType::UNIFORM_BUFFER
    std::optional<filament::backend::DescriptorType> type;
    std::optional<filament::SamplerInterfaceBlock::SamplerInfo> samplerInfo;
};

// All describing a shader that should be created.
struct ShaderConfig {
    filament::backend::ShaderLanguage vertexLanguage;
    std::string vertexShader;
    filament::backend::ShaderLanguage fragmentLanguage;
    std::string fragmentShader;
    std::vector<UniformConfig> uniforms;
};

// All values describing a uniform.
struct ResolvedUniformBindingConfig {
    size_t dataSize;
    size_t bufferSize;
    uint32_t byteOffset;
    filament::backend::descriptor_set_t set;
    filament::backend::descriptor_binding_t binding;
    std::optional<filament::backend::DescriptorSetHandle> descriptorSet;
};

// An equivalent to ResolvedUniformBindingConfig with all fields optional.
// resolve can be called to create a ResolvedBindingConfig with default values
// use in place of nullopt values.
struct UniformBindingConfig {
    std::optional<size_t> dataSize;
    std::optional<size_t> bufferSize;
    std::optional<uint32_t> byteOffset;
    std::optional<filament::backend::descriptor_set_t> set;
    std::optional<filament::backend::descriptor_binding_t> binding;
    std::optional<filament::backend::DescriptorSetHandle> descriptorSet;

    template<typename UniformType>
    ResolvedUniformBindingConfig resolve();
};

class Shader {
public:
    // All graphics resources have their lifetime controlled by the Cleanup and not this object.
    // Shader must not outlive either the DriverApi or the Cleanup.
    Shader(filament::backend::DriverApi& api, Cleanup& cleanup, ShaderConfig config);

    // Uploads a uniform with the default config.
    template<typename UniformType>
    void uploadUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformType uniforms) const;
    // Binds a uniform with the default config.
    template<typename UniformType>
    void bindUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer) const;
    // Uploads a uniform, with UniformBindingConfig providing the arguments.
    template<typename UniformType>
    void uploadUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformBindingConfig config, UniformType uniforms) const;
    // Binds a uniform, with UniformBindingConfig providing the arguments.
    template<typename UniformType>
    void bindUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformBindingConfig config) const;

    filament::backend::ProgramHandle getProgram() const;
    filament::backend::DescriptorSetLayoutHandle getDescriptorSetLayout() const;

    void addProgramToPipelineState(filament::backend::PipelineState& state) const;

    filament::backend::DescriptorSetHandle createDescriptorSet(
            filament::backend::DriverApi& api) const;

protected:
    Cleanup& mCleanup;
    filament::backend::ProgramHandle mProgram;
    // This will be a null handle if there are no uniforms.
    filament::backend::DescriptorSetLayoutHandle mDescriptorSetLayout;
};

template<typename UniformType>
ResolvedUniformBindingConfig UniformBindingConfig::resolve() {
    auto resolvedDataSize = dataSize.value_or(sizeof(UniformType));
    return ResolvedUniformBindingConfig{
            .dataSize = resolvedDataSize,
            .bufferSize = bufferSize.value_or(resolvedDataSize),
            .byteOffset = byteOffset.value_or(0),
            .set = set.value_or(0),
            .binding = binding.value_or(0),
            .descriptorSet = descriptorSet
    };
}

template<typename UniformType>
void Shader::uploadUniform(filament::backend::DriverApi& api,
        filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
        UniformBindingConfig config, UniformType uniforms) const {
    auto resolvedConfig = config.resolve<UniformType>();

    UniformType* tmp = new UniformType(uniforms);
    auto cb = [](void* buffer, size_t size, void* user) {
        UniformType* sp = (UniformType*)buffer;
        delete sp;
    };
    filament::backend::BufferDescriptor bd(tmp, resolvedConfig.dataSize, cb);
    api.updateBufferObject(hwBuffer, std::move(bd), resolvedConfig.byteOffset);
}

template<typename UniformType>
void Shader::bindUniform(filament::backend::DriverApi& api,
        filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
        UniformBindingConfig config) const {
    auto resolvedConfig = config.resolve<UniformType>();

    filament::backend::DescriptorSetHandle descriptorSet;
    if (resolvedConfig.descriptorSet.has_value()) {
        descriptorSet = *resolvedConfig.descriptorSet;
    } else {
        descriptorSet = createDescriptorSet(api);
    }

    api.updateDescriptorSetBuffer(descriptorSet, resolvedConfig.binding, hwBuffer, 0,
            resolvedConfig.bufferSize);
    api.bindDescriptorSet(descriptorSet, resolvedConfig.set, {});
}

template<typename UniformType>
void Shader::uploadUniform(filament::backend::DriverApi& api,
        filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
        UniformType uniforms) const {
    uploadUniform(api, hwBuffer, UniformBindingConfig{}, uniforms);
}

template<typename UniformType>
void Shader::bindUniform(filament::backend::DriverApi& api,
        filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer) const {
    bindUniform<UniformType>(api, hwBuffer, UniformBindingConfig{});
}

} // namespace test

#endif //TNT_SHADER_H
