/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "ComputeTest.h"

using namespace filament;
using namespace filament::backend;

TEST_F(ComputeTest, basic) {
    auto& driver = getDriverApi();

    std::string_view shader_gles310 = {R"(
#version 310 es
layout(local_size_x = 16) in;
void main() {
}
)"};

    std::string_view shader_gl450 = {R"(
#version 450 core
layout(local_size_x = 16) in;
void main() {
}
)"};

    std::string_view shader_msl = {R"(
// TODO: msl test
)"};

    std::string_view shader_spirv = {R"(
// TODO: spirv test
)"};

    std::string_view shader;
    switch (getBackend()) {
        case Backend::OPENGL:   shader = isMobile() ? shader_gles310 : shader_gl450;    break;
        case Backend::VULKAN:   shader = shader_spirv;  break;
        case Backend::METAL:    shader = shader_msl;    break;
        default:
            GTEST_FATAL_FAILURE_("unexpected backend");
    }

    Program program;
    program.shader(ShaderStage::COMPUTE, shader.data(), shader.size());

    Handle<HwProgram> ph = driver.createProgram(std::move(program));
    driver.dispatchCompute(ph, { 1, 1, 1 });
    driver.destroyProgram(ph);
    driver.finish();

    executeCommands();
    getDriver().purge();
}

TEST_F(ComputeTest, copy) {
    auto& driver = getDriverApi();

    std::string_view shader_gles310 = {R"(
#version 310 es
layout(local_size_x = 16) in;
layout(std430) buffer;
layout(binding = 0) writeonly buffer Output { float elements[]; } output_data;
layout(binding = 1) readonly  buffer Input0 { float elements[]; } input_data;
void main() {
    uint ident = gl_GlobalInvocationID.x;
    output_data.elements[ident] = input_data.elements[ident];
}
)"};

    std::string_view shader_gl450 = {R"(
#version 450 core
layout(local_size_x = 16) in;
layout(std430) buffer;
layout(binding = 0) writeonly buffer Output { float elements[]; } output_data;
layout(binding = 1) readonly  buffer Input0 { float elements[]; } input_data;
void main() {
    uint ident = gl_GlobalInvocationID.x;
    output_data.elements[ident] = input_data.elements[ident];
}
)"};

    std::string_view shader_msl = {R"(
// TODO: msl test
)"};

    std::string_view shader_spirv = {R"(
// TODO: spirv test
)"};

    std::string_view shader;
    switch (getBackend()) {
        case Backend::OPENGL:   shader = isMobile() ? shader_gles310 : shader_gl450;    break;
        case Backend::VULKAN:   shader = shader_spirv;  break;
        case Backend::METAL:    shader = shader_msl;    break;
        default:
            GTEST_FATAL_FAILURE_("unexpected backend");
    }

    size_t groupSize = 16;
    size_t groupCount = 1024;
    size_t size = groupSize * groupCount * sizeof(float);

    std::vector<float> data(groupSize * groupCount);
    std::generate(data.begin(), data.end(), [v = 0.0f]() mutable {
        v = v + 1.0f;
        return v;
    });

    auto output_data = driver.createBufferObject(size, BufferObjectBinding::SHADER_STORAGE, BufferUsage::STATIC);
    auto input_data = driver.createBufferObject(size, BufferObjectBinding::SHADER_STORAGE, BufferUsage::STATIC);
    driver.updateBufferObject(input_data, { data.data(), size }, 0);

    Program program;
    program.shader(ShaderStage::COMPUTE, shader.data(), shader.size());
    Handle<HwProgram> ph = driver.createProgram(std::move(program));


    driver.bindBufferRange(BufferObjectBinding::SHADER_STORAGE, 0, output_data, 0, size);
    driver.bindBufferRange(BufferObjectBinding::SHADER_STORAGE, 1, input_data, 0, size);

    driver.dispatchCompute(ph, { groupCount, 1, 1 });

    driver.unbindBuffer(BufferObjectBinding::SHADER_STORAGE, 0);
    driver.unbindBuffer(BufferObjectBinding::SHADER_STORAGE, 1);

    float* const user = (float*)malloc(size);
    driver.readBufferSubData(output_data, 0, size, { user, size });

    driver.destroyProgram(ph);
    driver.destroyBufferObject(input_data);
    driver.destroyBufferObject(output_data);
    driver.finish();

    executeCommands();
    getDriver().purge();

    // TODO: check buffer content
    EXPECT_EQ(memcmp(user, data.data(), size), 0);

    free(user);
}
