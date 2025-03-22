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

namespace test {

struct ShaderConfig {
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<utils::CString> uniformNames;
};

struct UniformBindingConfig {
    std::optional<size_t> dataSize;
    std::optional<size_t> bufferSize;
    std::optional<uint32_t> byteOffset;
    std::optional<filament::backend::descriptor_set_t> set;
    std::optional<filament::backend::descriptor_binding_t> binding;

    template <typename UniformType>
    void resolve() {
        if (!dataSize.has_value()) {
            dataSize = sizeof(UniformType);
        }
        if (!bufferSize.has_value()) {
            bufferSize = *dataSize;
        }
        if (!byteOffset.has_value()) {
            byteOffset = 0;
        }
        if (!set.has_value()) {
            set = 1;
        }
        if (!binding.has_value()) {
            binding = 0;
        }
    }
};

class Shader {
public:
    Shader(filament::backend::DriverApi& api, Cleanup& cleanup, ShaderConfig config);

    // Uploads and binds a uniform, defaulting to the binding 0, set 1, with no offset and using
    // UniformType's size as the size of data to upload.
    template<typename UniformType>
    void uploadUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformType uniforms) const;
    template<typename UniformType>
    void bindUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer) const;
    // Uploads and binds a uniform, with UniformBindingConfig optionally overriding any of the
    // default assumptions.
    template<typename UniformType>
    void uploadUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformBindingConfig config, UniformType uniforms) const;
    template<typename UniformType>
    void bindUniform(filament::backend::DriverApi& api,
            filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer,
            UniformBindingConfig config) const;

    filament::backend::ProgramHandle getProgram() const;
    filament::backend::DescriptorSetLayoutHandle getDescriptorSetLayout() const;
    filament::backend::DescriptorSetHandle getDescriptorSet() const;

protected:
    filament::backend::ProgramHandle mProgram;
    filament::backend::DescriptorSetLayoutHandle mDescriptorSetLayout;
    filament::backend::DescriptorSetHandle mDescriptorSet;
};

template <typename UniformType>
void Shader::uploadUniform(filament::backend::DriverApi& api, filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer, UniformBindingConfig config, UniformType uniforms) const {
    config.resolve<UniformType>();

    UniformType* tmp = new UniformType(uniforms);
    auto cb = [](void* buffer, size_t size, void* user) {
        UniformType* sp = (UniformType*) buffer;
        delete sp;
    };
    filament::backend::BufferDescriptor bd(tmp, *config.dataSize, cb);
    api.updateBufferObject(hwBuffer, std::move(bd), *config.byteOffset);
}

template <typename UniformType>
void Shader::bindUniform(filament::backend::DriverApi& api, filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer, UniformBindingConfig config) const {
    config.resolve<UniformType>();

    api.updateDescriptorSetBuffer(getDescriptorSet(), *config.binding, hwBuffer, 0, *config.bufferSize);
    api.bindDescriptorSet(getDescriptorSet(), *config.set, {});
}

template <typename UniformType>
void Shader::uploadUniform(filament::backend::DriverApi& api, filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer, UniformType uniforms) const {
    uploadUniform(api, hwBuffer, UniformBindingConfig{}, uniforms);
}

template <typename UniformType>
void Shader::bindUniform(filament::backend::DriverApi& api, filament::backend::Handle<filament::backend::HwBufferObject> hwBuffer) const {
    bindUniform<UniformType>(api, hwBuffer, UniformBindingConfig{});
}

} // namespace test

#endif //TNT_SHADER_H
