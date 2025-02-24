//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanDescriptorSetTest:
//   Various tests related for Vulkan descriptor sets.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

using namespace angle;

namespace
{

class VulkanDescriptorSetTest : public ANGLETest<>
{
  protected:
    VulkanDescriptorSetTest() {}

    void testSetUp() override
    {
        mMaxSetsPerPool = rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolForTesting();
        mMaxSetsPerPoolMultiplier =
            rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolMultiplierForTesting();
    }

    void testTearDown() override
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(mMaxSetsPerPool);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            mMaxSetsPerPoolMultiplier);
    }

    static constexpr uint32_t kMaxSetsForTesting           = 1;
    static constexpr uint32_t kMaxSetsMultiplierForTesting = 1;

    void limitMaxSets()
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(kMaxSetsForTesting);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            kMaxSetsMultiplierForTesting);
    }

  private:
    uint32_t mMaxSetsPerPool;
    uint32_t mMaxSetsPerPoolMultiplier;
};

// Test atomic counter read.
TEST_P(VulkanDescriptorSetTest, AtomicCounterReadLimitedDescriptorPool)
{
    // Skipping test while we work on enabling atomic counter buffer support in th D3D renderer.
    // http://anglebug.com/42260658
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Must be before program creation to limit the descriptor pool sizes when creating the pipeline
    // layout.
    limitMaxSets();

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 0, offset = 4) uniform atomic_uint ac;\n"
        "out highp vec4 my_color;\n"
        "void main()\n"
        "{\n"
        "    my_color = vec4(0.0);\n"
        "    uint a1 = atomicCounter(ac);\n"
        "    if (a1 == 3u) my_color = vec4(1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);

    glUseProgram(program);

    // The initial value of counter 'ac' is 3u.
    unsigned int bufferData[3] = {11u, 3u, 1u};
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    for (int i = 0; i < 5; ++i)
    {
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    }
}

class VulkanDescriptorSetLayoutDescTest : public ANGLETest<>
{
  protected:
    VulkanDescriptorSetLayoutDescTest() {}

    void testSetUp() override { ANGLETest::testSetUp(); }

    void testTearDown() override { ANGLETest::testTearDown(); }

    gl::Context *hackContext() const
    {
        egl::Display *display   = static_cast<egl::Display *>(getEGLWindow()->getDisplay());
        gl::ContextID contextID = {
            static_cast<GLuint>(reinterpret_cast<uintptr_t>(getEGLWindow()->getContext()))};
        return display->getContext(contextID);
    }

    rx::ContextVk *hackANGLE() const
    {
        // Hack the angle!
        return rx::GetImplAs<rx::ContextVk>(hackContext());
    }

    struct DescriptorSetBinding
    {
        uint32_t bindingIndex;
        VkDescriptorType type;
        uint32_t bindingCount;
        VkShaderStageFlagBits shaderStage;
    };

    const std::array<DescriptorSetBinding, 12> mBindings = {{
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {6, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {10, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT},
        {11, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
    }};

    void addBindings(const std::vector<uint32_t> &bindingIndices,
                     rx::vk::DescriptorSetLayoutDesc *desc)
    {
        for (uint32_t index : bindingIndices)
        {
            ASSERT(index < mBindings.size());
            const DescriptorSetBinding &binding = mBindings[index];
            desc->addBinding(binding.bindingIndex, binding.type, binding.bindingCount,
                             binding.shaderStage, nullptr);
        }
    }

    rx::vk::DescriptorSetLayoutDesc mDescriptorSetLayoutDesc;
    rx::DescriptorSetLayoutCache mDescriptorSetLayoutCache;
};

// Test basic interaction between DescriptorSetLayoutDesc and DescriptorSetLayoutCache
TEST_P(VulkanDescriptorSetLayoutDescTest, Basic)
{
    const std::vector<uint32_t> bindingsPattern1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    const std::vector<uint32_t> bindingsPattern2 = {0, 1};
    const std::vector<uint32_t> bindingsPattern3 = {0, 1, 5, 9};

    angle::Result result;
    rx::ContextVk *contextVk = hackANGLE();
    rx::vk::DescriptorSetLayoutPtr descriptorSetLayout;

    mDescriptorSetLayoutDesc = {};
    addBindings(bindingsPattern1, &mDescriptorSetLayoutDesc);
    result = mDescriptorSetLayoutCache.getDescriptorSetLayout(contextVk, mDescriptorSetLayoutDesc,
                                                              &descriptorSetLayout);
    EXPECT_EQ(result, angle::Result::Continue);
    EXPECT_EQ(mDescriptorSetLayoutCache.getCacheMissCount(), 1u);

    mDescriptorSetLayoutDesc = {};
    addBindings(bindingsPattern2, &mDescriptorSetLayoutDesc);
    result = mDescriptorSetLayoutCache.getDescriptorSetLayout(contextVk, mDescriptorSetLayoutDesc,
                                                              &descriptorSetLayout);
    EXPECT_EQ(result, angle::Result::Continue);
    EXPECT_EQ(mDescriptorSetLayoutCache.getCacheMissCount(), 2u);

    mDescriptorSetLayoutDesc = {};
    addBindings(bindingsPattern3, &mDescriptorSetLayoutDesc);
    size_t reusedDescHash = mDescriptorSetLayoutDesc.hash();
    result = mDescriptorSetLayoutCache.getDescriptorSetLayout(contextVk, mDescriptorSetLayoutDesc,
                                                              &descriptorSetLayout);
    EXPECT_EQ(result, angle::Result::Continue);
    EXPECT_EQ(mDescriptorSetLayoutCache.getCacheMissCount(), 3u);

    rx::vk::DescriptorSetLayoutDesc desc;
    addBindings(bindingsPattern3, &desc);
    size_t newDescHash = desc.hash();
    EXPECT_EQ(reusedDescHash, newDescHash);

    result =
        mDescriptorSetLayoutCache.getDescriptorSetLayout(contextVk, desc, &descriptorSetLayout);
    EXPECT_EQ(result, angle::Result::Continue);
    EXPECT_EQ(mDescriptorSetLayoutCache.getCacheHitCount(), 1u);
    EXPECT_EQ(mDescriptorSetLayoutCache.getCacheMissCount(), 3u);

    descriptorSetLayout.reset();
    mDescriptorSetLayoutCache.destroy(contextVk->getRenderer());
}

ANGLE_INSTANTIATE_TEST(VulkanDescriptorSetTest, ES31_VULKAN(), ES31_VULKAN_SWIFTSHADER());
ANGLE_INSTANTIATE_TEST(VulkanDescriptorSetLayoutDescTest, ES31_VULKAN());

}  // namespace
