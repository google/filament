/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "WebGPUPushConstantDescription.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Logger.h>

#include <webgpu/webgpu_cpp.h>


namespace filament::backend {

WebGPUPushConstantDescription::WebGPUPushConstantDescription(Program const& program) {
    uint32_t offset = 0;

    // The range is laid out so that the vertex constants are defined as the first set of bytes,
    // followed by fragment and compute. This means we need to keep track of the offset for each
    // stage. We do the bookeeping in mDescriptions.
    for (auto stage: { ShaderStage::VERTEX, ShaderStage::FRAGMENT, ShaderStage::COMPUTE }) {
        auto const& constants = program.getPushConstants(stage);
        if (constants.empty()) {
            continue;
        }

        auto& description = mDescriptions[(uint8_t) stage];
        description.types.reserve(constants.size());
        std::for_each(constants.cbegin(), constants.cend(),
                [&description](Program::PushConstant t) { description.types.push_back(t.type); });

        uint32_t const constantsSize = (uint32_t) constants.size() * ENTRY_SIZE;
        description.offset = offset;
        offset += constantsSize;
    }
}

void WebGPUPushConstantDescription::setPushConstant(const wgpu::RenderPassEncoder& renderPassEncoder,
        ShaderStage stage, const uint8_t index, const PushConstantVariant value) {
    auto const& description = mDescriptions[(uint8_t) stage];
    UTILS_UNUSED_IN_RELEASE auto const& types = description.types;
    assert_invariant(
            index < types.size() && "Push constant index is out of bounds for current stage");
    uint32_t const offset = description.offset;

    uint32_t data = 0;
    if (std::holds_alternative<int32_t>(value)) {
        assert_invariant(types[index] == ConstantType::INT);
        int32_t v = std::get<int32_t>(value);
        std::memcpy(&data, &v, sizeof(data));
    } else if (std::holds_alternative<float>(value)) {
        assert_invariant(types[index] == ConstantType::FLOAT);
        float v = std::get<float>(value);
        std::memcpy(&data, &v, sizeof(data));
    } else if (std::holds_alternative<bool>(value)) {
        assert_invariant(types[index] == ConstantType::BOOL);
        data = std::get<bool>(value) ? 1 : 0;
    }

#if defined(__EMSCRIPTEN__)
    wgpuRenderPassEncoderSetImmediates(renderPassEncoder.Get(), offset + index * ENTRY_SIZE, &data, ENTRY_SIZE);
#else
    renderPassEncoder.SetImmediates(offset + index * ENTRY_SIZE, &data, ENTRY_SIZE);
#endif
}

} // namespace filament::backend
