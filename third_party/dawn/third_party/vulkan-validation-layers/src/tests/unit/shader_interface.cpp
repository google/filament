/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeShaderInterface : public VkLayerTest {};

TEST_F(NegativeShaderInterface, MaxVertexComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpMemberDecorate built-ins");

    RETURN_IF_SKIP(InitFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // vec4 == 4 components
    // This gives 124 which is just below the set max limit
    const uint32_t numVec4 = 31;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }

    // GLSL is defined to have a struct for the vertex shader built-in:
    //
    //    out gl_PerVertex {
    //        vec4 gl_Position;
    //        float gl_PointSize;
    //        float gl_ClipDistance[];
    //        float gl_CullDistance[];
    //    } gl_out[];
    //
    // by including gl_Position here 7 extra vertex input components are added pushing it over the 128
    // 124 + 7 > 128 limit
    vsSourceStr += "    gl_Position = x;\n";
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
    // maxFragmentInputComponents is not reached because GLSL should not be including any input fragment stage built-ins by default
    // only maxVertexOutputComponents is reached
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(NegativeShaderInterface, MaxFragmentComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpDecorate built-ins");

    RETURN_IF_SKIP(InitFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // vec4 == 4 components
    // This gives 128 which is the max limit
    const uint32_t numVec4 = 32;  // 32 * 4 == 128

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    // By added gl_PointCoord it adds 2 more components to the fragment input stage
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1) * gl_PointCoord.x;\n"
        "}\n";

    // maxVertexOutputComponents is not reached because GLSL should not be including any output vertex stage built-ins
    // only maxFragmentInputComponents is reached
    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MaxVertexOutputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of output components from the vertex stage exceeds the device limit");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxVsOutComp = m_device->Physical().limits_.maxVertexOutputComponents + overflow;
        std::string vsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxVsOutComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            vsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") out vec4 vn;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numVec4; i++) {
                vsSourceStr += "layout(location=" + std::to_string(location) + ") out vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxVsOutComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    vsSourceStr += "layout(location=" + std::to_string(location) + ") out float" + " vn;\n";
                } else {
                    vsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") out vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        vsSourceStr +=
            "void main(){\n"
            "}\n";

        switch (overflow) {
            case 0: {
                VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
                break;
            }
            case 1: {
                // component and location limit (maxVertexOutputComponents)
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            case 2: {
                // just component limit (maxVertexOutputComponents)
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeShaderInterface, MaxComponentsBlocks) {
    TEST_DESCRIPTION("Test if the max componenets checks are done properly when in a single block");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // To make the test simple, just make sure max is 128 or less (most HW is 64 or 128)
    if (m_device->Physical().limits_.maxVertexOutputComponents > 128 ||
        m_device->Physical().limits_.maxFragmentInputComponents > 128) {
        GTEST_SKIP() << "maxVertexOutputComponents or maxFragmentInputComponents too high for test";
    }
    // vec4 == 4 components
    // so this put the test over 128
    const uint32_t numVec4 = 33;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    // maxVertexOutputComponents
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();

    // maxFragmentInputComponents
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MaxFragmentInputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of input components from the fragment stage exceeds the device limit");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxFsInComp = m_device->Physical().limits_.maxFragmentInputComponents + overflow;
        std::string fsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxFsInComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            fsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") in float" + " vn;\n";
        } else {
            for (uint32_t i = 0; i < numVec4; i++) {
                fsSourceStr += "layout(location=" + std::to_string(location) + ") in vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxFsInComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    fsSourceStr += "layout(location=" + std::to_string(location) + ") in float" + " vn;\n";
                } else {
                    fsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") in vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        fsSourceStr +=
            "layout(location=0) out vec4 color;"
            "\n"
            "void main(){\n"
            "    color = vec4(1);\n"
            "}\n";

        switch (overflow) {
            case 0: {
                VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
                break;
            }
            case 1: {
                // (maxFragmentInputComponents)
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            case 2: {
                // (maxFragmentInputComponents)
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeShaderInterface, FragmentInputNotProvided) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input which is not present in the outputs of the previous stage");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, FragmentInputNotProvidedInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input within an interface block, which is not present in the outputs "
        "of the previous stage.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched types across the vertex->fragment shader interface");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out int x;
        void main(){
           x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatch2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8443");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out int x;
        void main(){
           x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        // Flipped here
        helper.shader_stages_ = {fs.GetStageCreateInfo(), vs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for mismatched types across the vertex->fragment shader interface, when the variable is "
        "contained within an interface block");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0) int x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchVectorSize) {
    TEST_DESCRIPTION("OpTypeVector has larger output than input");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        void main(){
           gl_Position = vec4(1.0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(1.0);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maintenance4-06817");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStruct) {
    TEST_DESCRIPTION("Have a struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S y;
            layout(location=10) int[4] z;
        } outBlock;

        void main() {
            outBlock.y.a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S y;
            layout(location=10) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y.a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStruct64bit) {
    TEST_DESCRIPTION("Have a struct inside a block between shaders");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable

        struct S {
            vec4 a[2];
            float b;
            f64vec3 c; // difference (takes 2 locations)
        };

        out block {
            layout(location=0) S x[2];
            layout(location=10) int y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            vec4 c; // difference
            vec2 d;
        };

        in block {
            layout(location=0) S x[2];
            layout(location=10) int y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockArrayOfStruct) {
    TEST_DESCRIPTION("Have an array of struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y; // each array is 4 locations slots
            layout(location=14) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=14) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructInnerArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[2] c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[3] c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // Both are errors, depending on compiler, the order of variables listed will hit one before the other
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-07754");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructOuterArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c;
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y; // difference
            layout(location=20) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c;
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[3] y; // difference
            layout(location=20) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructArraySizeVertex) {
    TEST_DESCRIPTION(
        "Have an struct inside a block between shaders, array size is difference, but from the vertex shader being too large");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[3] c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[2] c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // Both are errors, depending on compiler, the order of variables listed will hit one before the other
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-08743");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructOuter2DArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
        };

        out block {
            layout(location=0) float x;
            layout(location=2) S[2][2] y; // difference
            layout(location=21) int[2] z;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
        };

        in block {
            layout(location=0) float x;
            layout(location=2) S[2][3] y; // difference
            layout(location=21) int[2] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockNestedStructType64bit) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable

        struct A {
            float a0_;
        };
        struct B {
            f64vec3 b0_;
            A b1_;
        };
        struct C {
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            vec3 b0_;
            A b1_;
        };
        struct C {
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockNestedStructArray) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_; // difference
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_[2];  // difference
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsMismatchByLocation) {
    TEST_DESCRIPTION(
        "Test that an error is produced for location mismatches across the vertex->fragment shader interface; This should manifest "
        "as a not-written/not-consumed pair, but flushes out broken walking of the interfaces");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=1) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsMismatchByComponent) {
    TEST_DESCRIPTION(
        "Test that an error is produced for component mismatches across the vertex->fragment shader interface. It's not enough to "
        "have the same set of locations in use; matching is defined in terms of spirv variables.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0, component=0) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0, component=1) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchShaderObject) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out int x;
        void main(){
           x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fsSource));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpEntryPoint-07754");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchVectorSizeShaderObject) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        void main(){
           gl_Position = vec4(1.0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(1.0);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fsSource));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-maintenance4-06817");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderInterface, InputOutputMismatch) {
    TEST_DESCRIPTION("Test mismatch between vertex shader output and fragment shader input.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vsSource[] = R"glsl(
        #version 450
        layout(location = 1) out int v;
        void main() {
            v = 1;
        }
    )glsl";

    const char fsSource[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 color;
        layout(location = 1) in float v;
        void main() {
           color = vec4(v);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpEntryPoint-07754");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, VertexOutputNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex output that is not consumed by the fragment stage");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float x;
        void main(){
           gl_Position = vec4(1);
           x = 0;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "WARNING-Shader-OutputNotConsumed");
}

// Spec doesn't clarify if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/3293
TEST_F(NegativeShaderInterface, DISABLED_InputAndOutputComponents) {
    TEST_DESCRIPTION("Test invalid shader layout in and out with different components.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out float r;
                layout(location = 0, component = 2) out float b;

                void main() {
                    r = 0.25f;
                    b = 0.75f;
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec3 rgb;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(rgb, 1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float a;
                layout(location = 0, component = 2) in float b;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "WARNING-Shader-OutputNotConsumed");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec4 v;

                layout (location = 0) out vec4 color;

                void main() {
                    color = v;
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v1;
                layout(location = 1) out vec3 v2;
                layout(location = 2) out vec3 v3;

                void main() {
                    v1 = vec3(1.0);
                    v2 = vec3(2.0);
                    v3 = vec3(3.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) in vec3 v1;
                layout (location = 2) in vec3 v3;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(v1 * v3, 1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(NegativeShaderInterface, AlphaToCoverageOutputLocation0) {
    TEST_DESCRIPTION("Test that an error is produced when alpha to coverage is enabled but no output at location 0 is declared.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    VkShaderObj fs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageOutputIndex1) {
    TEST_DESCRIPTION("DualSource blend has two outputs at location zero, so Index 0 is the one that's required");
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0, index = 1) out vec4 c0;
        void main() {
            c0 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageOutputNoAlpha) {
    TEST_DESCRIPTION(
        "Test that an error is produced when alpha to coverage is enabled but output at location 0 doesn't have alpha component.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        void main(){
           x = vec3(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageArrayIndex) {
    TEST_DESCRIPTION("Have array out outputs, but start at index 1");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=1) out vec4 fragData[3];
        void main() {
            fragData[0] = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageArrayVec3) {
    TEST_DESCRIPTION("Have array out outputs, but not contain the alpha component");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 fragData[4];
        void main() {
            fragData[0] = vec3(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, MultidimensionalArray) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][2][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[4][3][2] x; // 2 extra Locations
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, MultidimensionalArrayDim) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][2][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[17] x; // 1 extra Locations
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, MultidimensionalArray64bit) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled for 64bits");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxFragmentOutputAttachments < 25) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) out f64vec3[2][2][2] x; // take 2 locations each (total 16)
        layout(location=24) out float y;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) flat in f64vec3[2][3][2] x;
        layout(location=24) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, PackingInsideArray) {
    TEST_DESCRIPTION("From https://gitlab.khronos.org/vulkan/vulkan/-/issues/3558");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0, component = 1) out float[2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0, component = 1) in float x;
        layout(location = 1, component = 0) in float y;
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputNotWritten) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader which does not provide an output for one of the pipeline's color "
        "attachments");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj fs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.cb_attachments_.colorWriteMask = 0xf;  // all components
    m_errorMonitor->SetDesiredWarning("Undefined-Value-ShaderInputNotProduced");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7923
TEST_F(NegativeShaderInterface, DISABLED_CreatePipelineFragmentOutputNotWrittenDynamicRendering) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader which does not provide an output for one of the pipeline's color "
        "attachments");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    VkShaderObj fs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkFormat color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.cb_attachments_.colorWriteMask = 1;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredWarning("Undefined-Value-ShaderInputNotProduced-DynamicRendering");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a mismatch between the fundamental type of an fragment shader output variable, and the "
        "format of the corresponding attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out ivec4 x; /* not UNORM */
        void main(){
           x = ivec4(1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Undefined-Value-ShaderFragmentOutputMismatch");
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputNotConsumed) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a fragment shader which provides a spurious output with no matching attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(location=1) out vec4 y; /* no matching attachment for this */
        void main(){
           x = vec4(1);
           y = vec4(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Undefined-Value-ShaderOutputNotConsumed");
}

TEST_F(NegativeShaderInterface, InvalidStaticSpirv) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a fragment shader which provides a spurious output with no matching attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %fragCoord %block_var
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %fragCoord Location 0
               OpDecorate %block Block
               OpMemberDecorate %block 0 Location 1
       %void = OpTypeVoid
      %voidfn = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%ptr_v4float = OpTypePointer Output %v4float
  %fragCoord = OpVariable %ptr_v4float Output
      %block = OpTypeStruct %v4float %v4float
  %block_ptr = OpTypePointer Output %block
  %block_var = OpVariable %block_ptr Output
       %main = OpFunction %void None %voidfn
       %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    // VUID-StandaloneSpirv-Location-04919
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    auto fs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, InvalidStaticSpirvMaintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 460
               OpMemberDecorate %gl_PerVertex 2 BuiltIn Position
               OpDecorate %gl_PerVertex Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
    std::vector<uint32_t> shader;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, spv_source, shader);

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = shader.data();
    module_create_info.codeSize = shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, InvalidStaticSpirvMaintenance5Compute) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpSource GLSL 460
               OpMemberDecorate %gl_PerVertex 2 BuiltIn Position
               OpDecorate %gl_PerVertex Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
    std::vector<uint32_t> shader;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, spv_source, shader);

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = shader.data();
    module_create_info.codeSize = shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    vkt::PipelineLayout layout(*m_device, {});
    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = stage_ci;
    pipe.cp_ci_.layout = layout.handle();

    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, PhysicalStorageBuffer) {
    TEST_DESCRIPTION("Regression shaders from https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/5349");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference_uvec2 : enable

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            highp int value1;
            highp int value2;
        };

        layout(location=0) out dataBuffer outgoingPtr;
        void main() {
            outgoingPtr = dataBuffer(uvec2(2.0));
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference_uvec2 : enable

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            highp int value1;
            highp int value2;
        };

        layout(location=0) in dataBuffer incomingPtr;
        layout(location=0) out highp vec4 fragColor;
        void main() {
            highp ivec2 v = ivec2(incomingPtr.value1, incomingPtr.value2);
            fragColor = vec4(float(v.x)/255.0,float(v.y)/255.0, float(v.x+v.y)/255.0,1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MultipleFragmentAttachment) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7923");
    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color0;
        layout(location=1) out vec4 color1;
        void main() {
           color0 = vec4(1.0);
           color1 = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.AddColorAttachment(2);
    rp.CreateRenderPass();

    VkPipelineColorBlendAttachmentState cb_as = {VK_FALSE,
                                                 VK_BLEND_FACTOR_ZERO,
                                                 VK_BLEND_FACTOR_ZERO,
                                                 VK_BLEND_OP_ADD,
                                                 VK_BLEND_FACTOR_ZERO,
                                                 VK_BLEND_FACTOR_ZERO,
                                                 VK_BLEND_OP_ADD,
                                                 0xf};
    VkPipelineColorBlendAttachmentState cb_states[3] = {cb_as, cb_as, cb_as};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.cb_ci_.attachmentCount = 3;
    pipe.cb_ci_.pAttachments = cb_states;
    pipe.gp_ci_.renderPass = rp.Handle();
    m_errorMonitor->SetDesiredWarning("Undefined-Value-ShaderInputNotProduced");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MissingInputAttachmentIndex) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs;
    // layout(location=0) out vec4 color;
    // void main() {
    //     color = subpassLoad(xs);
    // }
    //
    // missing OpDecorate %xs InputAttachmentIndex 0
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %xs DescriptorSet 0
               OpDecorate %xs Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
         %xs = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
         %17 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %xs
         %18 = OpImageRead %v4float %13 %17
               OpStore %color %18
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.gp_ci_.renderPass = rp.Handle();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-09558");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MissingInputAttachmentIndexArray) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::dynamicRenderingLocalRead);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[1];
    // layout(location=0) out vec4 color;
    // void main() {
    //     color = subpassLoad(xs[0]);
    // }
    //
    // missing OpDecorate %xs InputAttachmentIndex 0
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %xs DescriptorSet 0
               OpDecorate %xs Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_10_uint_1 = OpTypeArray %10 %uint_1
%_ptr_UniformConstant__arr_10_uint_1 = OpTypePointer UniformConstant %_arr_10_uint_1
         %xs = OpVariable %_ptr_UniformConstant__arr_10_uint_1 UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
      %v2int = OpTypeVector %int 2
         %22 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_UniformConstant_10 %xs %int_0
         %20 = OpLoad %10 %19
         %23 = OpImageRead %v4float %20 %22
               OpStore %color %23
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeImage-09644");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, MissingInputAttachmentIndexShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());

    // layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs;
    // layout(location=0) out vec4 color;
    // void main() {
    //     color = subpassLoad(xs);
    // }
    //
    // missing OpDecorate %xs InputAttachmentIndex 0
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %xs DescriptorSet 0
               OpDecorate %xs Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
         %xs = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
         %17 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %xs
         %18 = OpImageRead %v4float %13 %17
               OpStore %color %18
               OpReturn
               OpFunctionEnd
    )";
    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, fsSource, spv);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-09558");
    const vkt::Shader frag_shader(*m_device,
                                  ShaderCreateInfo(spv, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &descriptor_set.layout_.handle()));
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, PhysicalStorageBufferArray) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference_uvec2 : enable

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            int value2;
        };

        layout(location=0) out dataBuffer outgoingPtr[3];
        void main() {
            outgoingPtr[2] = dataBuffer(uvec2(2.0));
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference_uvec2 : enable

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            int value2;
        };

        layout(location=0) in dataBuffer incomingPtr[3];
        layout(location=0) out vec4 fragColor;
        void main() {
            ivec2 v = ivec2(incomingPtr[1].value1, incomingPtr[2].value2);
            fragColor = vec4(float(v.x)/255.0,float(v.y)/255.0, float(v.x+v.y)/255.0,1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, PhysicalStorageBufferLinkedList) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer dataBuffer;
        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            dataBuffer next;
        };

        layout(location=0) out dataBuffer outgoingPtr;
        void main() {
            outgoingPtr.next.value1 = 3;
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer dataBuffer;
        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            dataBuffer next;
        };

        layout(location=0) in dataBuffer incomingPtr;
        layout(location=0) out vec4 fragColor;
        void main() {
            ivec2 v = ivec2(incomingPtr.value1, incomingPtr.next.value1);
            fragColor = vec4(float(v.x)/255.0,float(v.y)/255.0, float(v.x+v.y)/255.0,1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, PhysicalStorageBufferNested) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, buffer_reference_align = 4) readonly buffer t2 {
            int values[];
        };

        layout(buffer_reference, buffer_reference_align = 4) readonly buffer t1 {
            t2 c;
        };

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            t1 next;
        };

        layout(location=0) out dataBuffer outgoingPtr;
        void main() {
            outgoingPtr.next.c.values[3] = 3;
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, buffer_reference_align = 4) readonly buffer t2 {
            int values[];
        };

        layout(buffer_reference, buffer_reference_align = 4) readonly buffer t1 {
            t2 c;
        };

        layout(set=0, binding=0) layout(buffer_reference, std430) buffer dataBuffer {
            int value1;
            t1 next;
        };

        layout(location=0) in dataBuffer incomingPtr;
        layout(location=0) out vec4 fragColor;
        void main() {
            ivec2 v = ivec2(incomingPtr.value1, incomingPtr.next.c.values[2]);
            fragColor = vec4(float(v.x)/255.0,float(v.y)/255.0, float(v.x+v.y)/255.0,1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredWarning("WARNING-PhysicalStorageBuffer-interface");
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}
