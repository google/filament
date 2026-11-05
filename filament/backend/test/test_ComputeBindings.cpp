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

#include "PlatformRunner.h"

#include "opengl/gl_headers.h"

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/DriverApi.h>
#include <private/backend/PlatformFactory.h>

#include <backend/Platform.h>
#include <backend/Program.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace filament;
using namespace filament::backend;

namespace {

Backend selectedBackend = Backend::OPENGL;

class ComputeBindings : public testing::Test {
protected:
    Platform* platform = nullptr;
    Driver* driver = nullptr;
    CommandBufferQueue queue{1024 * 1024, 3 * 1024 * 1024, false};
    std::unique_ptr<DriverApi> api;

    void SetUp() override {
        if (selectedBackend != Backend::OPENGL) {
            GTEST_SKIP() << "This regression tests the OpenGL descriptor binding contract";
        }
        Backend backend = selectedBackend;
        platform = PlatformFactory::create(&backend);
        ASSERT_NE(platform, nullptr);
        driver = platform->createDriver(nullptr, {});
        ASSERT_NE(driver, nullptr);
        api = std::make_unique<DriverApi>(*driver, queue.getCircularBuffer());
        if (driver->getFeatureLevel() < FeatureLevel::FEATURE_LEVEL_2) {
            GTEST_SKIP() << "Compute requires feature level 2";
        }
        std::cout << "GL_VERSION=" << glGetString(GL_VERSION)
                  << " GL_RENDERER=" << glGetString(GL_RENDERER) << std::endl;
    }

    void execute() {
        queue.flush();
        for (auto const& item : queue.waitForCommands()) {
            if (item.begin) {
                api->execute(item.begin);
                queue.releaseBuffer(item);
            }
        }
    }

    void finish() {
        api->finish();
        execute();
        driver->purge();
    }

    void TearDown() override {
        if (api) {
            finish();
            api.reset();
        }
        if (driver) {
            driver->terminate();
            delete driver;
        }
        PlatformFactory::destroy(&platform);
    }

    // Keep readback synchronization in the fixture. It is identical for the baseline and
    // both fixes, and is not a change to the backend's compute synchronization contract.
    void barrier() {
        execute();
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
    }

    enum class Mode { SINGLE, MULTIPLE, RANGE, PROGRAM, SET, DYNAMIC, MIXED,
        UNBIND_UPDATE };

    void run(Mode mode) {
        bool const single = mode == Mode::SINGLE;
        bool const dynamic = mode == Mode::DYNAMIC;
        bool const mixed = mode == Mode::MIXED;
        bool const range = mode == Mode::RANGE || dynamic;
        GLint alignment = 1;
        glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment);
        ASSERT_GT(alignment, 0);
        size_t const stride = ((64 + size_t(alignment) - 1) / size_t(alignment)) * size_t(alignment);
        ASSERT_EQ(stride % sizeof(uint32_t), size_t(0));
        uint32_t const offset = range ? uint32_t(stride) : 0;
        size_t const count = (stride * 3 + 64) / sizeof(uint32_t);
        constexpr uint32_t sentinel = 0xdeadbeef;
        std::vector<uint32_t> initial(count, sentinel);
        std::vector<uint32_t> input(count);
        for (size_t i = 0; i < count; ++i) {
            input[i] = uint32_t(i * 3 + 7);
        }
        auto makeBuffer = [&](BufferObjectBinding type, std::vector<uint32_t> const& data) {
            auto h = api->createBufferObject(uint32_t(data.size() * 4), type, BufferUsage::STATIC);
            api->updateBufferObject(h, {data.data(), data.size() * 4}, 0);
            return h;
        };
        auto output = makeBuffer(BufferObjectBinding::SHADER_STORAGE, initial);
        auto outputB = makeBuffer(BufferObjectBinding::SHADER_STORAGE, initial);
        auto source = makeBuffer(BufferObjectBinding::SHADER_STORAGE, input);
        std::vector<uint32_t> params{11, 0, 0, 0};
        auto uniform = makeBuffer(BufferObjectBinding::UNIFORM, params);
        std::array<uint8_t, 4> pixel{17, 0, 0, 255};
        TextureHandle texture;
        if (mixed) {
            texture = api->createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
                    1, 1, 1, TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE);
            api->update3DImage(texture, 0, 0, 0, 0, 1, 1, 1,
                    PixelBufferDescriptor(pixel.data(), pixel.size(),
                            PixelDataFormat::RGBA, PixelDataType::UBYTE));
        }

        auto makeProgram = [&](unsigned outBinding, unsigned inBinding) {
            std::string text = driver->getShaderModel() == ShaderModel::MOBILE
                    ? "#version 310 es\nprecision highp float;\nprecision highp int;\n"
                    : "#version 450 core\n";
            text += "layout(local_size_x=16) in;\n";
            // Deliberately declare Output before Input, while descriptors sort Input first.
            text += "layout(std430, binding=" + std::to_string(outBinding) +
                    ") writeonly buffer Output { uint values[]; } dst;\n";
            if (!single) {
                text += "layout(std430, binding=" + std::to_string(inBinding) +
                        ") readonly buffer Input { uint values[]; } src;\n";
            }
            text += "layout(std430, binding=2) buffer Unused { uint values[]; } unusedData;\n";
            if (mixed) {
                text += "layout(std140) uniform Params { uvec4 value; } params;\n"
                        "uniform highp sampler2D tex;\n";
            }
            text += "void main() { uint i = gl_GlobalInvocationID.x; dst.values[i] = ";
            text += single ? "i + 23u" : "src.values[i] + 23u";
            if (mixed) {
                text += " + params.value.x + uint(round(texelFetch(tex, ivec2(0), 0).r * 255.0))";
            }
            text += "; }\n";
            Program program;
            program.shaderLanguage(ShaderLanguage::ESSL3);
            program.shader(ShaderStage::COMPUTE, text.c_str(), text.size() + 1);
            Program::DescriptorBindingsInfo bindings;
            bindings.reserve(5);
            bindings.push_back({"Output", DescriptorType::SHADER_STORAGE_BUFFER, 2});
            bindings.push_back({"Unused", DescriptorType::SHADER_STORAGE_BUFFER, 1});
            if (!single) {
                bindings.push_back({"Input", DescriptorType::SHADER_STORAGE_BUFFER, 0});
            }
            if (mixed) {
                bindings.push_back({"Params", DescriptorType::UNIFORM_BUFFER, 3});
                bindings.push_back({"tex", DescriptorType::SAMPLER_2D_FLOAT, 4});
            }
            program.descriptorBindings(1, std::move(bindings));
            return api->createProgram(std::move(program));
        };
        auto program = makeProgram(single ? 0 : 3, 1);
        ProgramHandle programB;
        if (mode == Mode::PROGRAM) {
            programB = makeProgram(1, 3);
        }
        DescriptorSetLayout layout;
        layout.descriptors.reserve(5);
        if (!single) {
            layout.descriptors.push_back({DescriptorType::SHADER_STORAGE_BUFFER,
                    ShaderStageFlags::COMPUTE, 0});
        }
        layout.descriptors.push_back({DescriptorType::SHADER_STORAGE_BUFFER,
                ShaderStageFlags::COMPUTE, 1});
        layout.descriptors.push_back({DescriptorType::SHADER_STORAGE_BUFFER,
                ShaderStageFlags::COMPUTE, 2,
                dynamic ? DescriptorFlags::DYNAMIC_OFFSET : DescriptorFlags::NONE});
        if (mixed) {
            layout.descriptors.push_back({DescriptorType::UNIFORM_BUFFER,
                    ShaderStageFlags::COMPUTE, 3});
            layout.descriptors.push_back({DescriptorType::SAMPLER_2D_FLOAT,
                    ShaderStageFlags::COMPUTE, 4});
        }
        auto dsl = api->createDescriptorSetLayout(std::move(layout));
        auto makeSet = [&](BufferObjectHandle target) {
            auto set = api->createDescriptorSet(dsl);
            if (!single) {
                api->updateDescriptorSetBuffer(set, 0, source, offset, 64);
            }
            api->updateDescriptorSetBuffer(set, 1, source, 0, 64);
            api->updateDescriptorSetBuffer(set, 2, target, dynamic ? 0 : offset, 64);
            if (mixed) {
                api->updateDescriptorSetBuffer(set, 3, uniform, 0, 16);
                api->updateDescriptorSetTexture(set, 4, texture, {});
            }
            return set;
        };
        auto set = makeSet(output);
        DescriptorSetHandle setB;
        if (mode == Mode::SET) {
            setB = makeSet(outputB);
        }
        auto bind = [&](DescriptorSetHandle h, uint32_t dynamicOffset) {
            if (dynamic) {
                api->bindDescriptorSet(h, 1, DescriptorSetOffsetArray({dynamicOffset}, *api));
            } else {
                api->bindDescriptorSet(h, 1, {});
            }
        };
        auto check = [&](BufferObjectHandle h, uint32_t writtenOffset, bool written = true) {
            std::vector<uint32_t> actual(count, 0xaaaaaaaa);
            api->readBufferSubData(h, 0, uint32_t(count * 4), {actual.data(), count * 4});
            finish();
            for (size_t i = 0; i < count; ++i) {
                uint32_t expected = sentinel;
                if (written && i >= writtenOffset / 4 && i < writtenOffset / 4 + 16) {
                    size_t const element = i - writtenOffset / 4;
                    expected = (single ? uint32_t(element) : input[offset / 4 + element]) + 23;
                    if (mixed) { expected += 28; }
                }
                EXPECT_EQ(actual[i], expected) << "word " << i;
            }
            EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
        };
        bind(set, offset);
        api->dispatchCompute(program, {1, 1, 1});
        barrier();
        check(output, offset);
        if (mode == Mode::PROGRAM || mode == Mode::SET || dynamic ||
                mode == Mode::UNBIND_UPDATE) {
            // Reset Output after a BUFFER_UPDATE barrier; every subsequent dispatch must write
            // fresh values rather than passing on the previous dispatch's output.
            api->updateBufferObject(output, {initial.data(), count * 4}, 0);
            uint32_t nextOffset = offset;
            auto nextProgram = program;
            auto nextOutput = output;
            if (mode == Mode::PROGRAM) { nextProgram = programB; }
            if (mode == Mode::SET) { bind(setB, 0); nextOutput = outputB; }
            if (dynamic) { nextOffset = uint32_t(stride * 2); bind(set, nextOffset); }
            if (mode == Mode::UNBIND_UPDATE) {
                api->bindDescriptorSet({}, 1, {});
                api->updateDescriptorSetBuffer(set, 2, outputB, 0, 64);
                bind(set, 0);
                nextOutput = outputB;
            }
            api->dispatchCompute(nextProgram, {1, 1, 1});
            barrier();
            check(nextOutput, nextOffset);
            if (nextOutput != output) { check(output, 0, false); }
        }
        std::vector<uint32_t> actualInput(count, 0xaaaaaaaa);
        api->readBufferSubData(source, 0, uint32_t(count * 4), {actualInput.data(), count * 4});
        finish();
        EXPECT_EQ(actualInput, input);
        api->bindDescriptorSet({}, 1, {});
        if (setB) { api->destroyDescriptorSet(setB); }
        api->destroyDescriptorSet(set);
        api->destroyDescriptorSetLayout(dsl);
        if (programB) { api->destroyProgram(programB); }
        api->destroyProgram(program);
        if (texture) { api->destroyTexture(texture); }
        api->destroyBufferObject(uniform);
        api->destroyBufferObject(source);
        api->destroyBufferObject(outputB);
        api->destroyBufferObject(output);
        finish();
    }
};

TEST_F(ComputeBindings, SingleStorageBuffer) { run(Mode::SINGLE); }
TEST_F(ComputeBindings, NoncontiguousBindingsAndUnusedBlock) { run(Mode::MULTIPLE); }
TEST_F(ComputeBindings, OffsetAndRange) { run(Mode::RANGE); }
TEST_F(ComputeBindings, ProgramSwitch) { run(Mode::PROGRAM); }
TEST_F(ComputeBindings, DescriptorSetSwitch) { run(Mode::SET); }
TEST_F(ComputeBindings, DynamicOffset) { run(Mode::DYNAMIC); }
TEST_F(ComputeBindings, UniformBufferAndSampler) { run(Mode::MIXED); }
TEST_F(ComputeBindings, UnbindBeforeUpdate) { run(Mode::UNBIND_UPDATE); }
} // namespace

int main(int argc, char** argv) {
    selectedBackend = Backend(test::parseArguments(argc, argv).backend);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
