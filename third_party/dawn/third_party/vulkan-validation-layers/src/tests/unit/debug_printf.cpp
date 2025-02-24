/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/buffer_helper.h"
#include "../framework/gpu_av_helper.h"

void DebugPrintfTests::InitDebugPrintfFramework(void *p_next, bool reserve_slot) {
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
                                              VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper(p_next);
    // Most tests don't need to reserve the slot, so keep it as an option for now
    features.enabledValidationFeatureCount = reserve_slot ? 2 : 1;
    features.disabledValidationFeatureCount = 0;
    features.pEnabledValidationFeatures = enables;

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&features));

    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "Currently disabled for Portability";
    }
}

class NegativeDebugPrintf : public DebugPrintfTests {
  public:
    void BasicComputeTest(const char *shader, const char *message);
    void BasicFormattingTest(const char *shader, bool warning = false);
};

void NegativeDebugPrintf::BasicComputeTest(const char *shader, const char *message) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo(message);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, Float) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float == 3.141500");
}

TEST_F(NegativeDebugPrintf, IntUnsigned) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uint foo = 127;
            debugPrintfEXT("unsigned == %u", foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned == 127");
}

TEST_F(NegativeDebugPrintf, IntUnsignedUnderflow) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uint foo = 127;
            debugPrintfEXT("underflow == %u", foo - 128);
        }
    )glsl";
    BasicComputeTest(shader_source, "underflow == 4294967295");
}

TEST_F(NegativeDebugPrintf, IntSignedOverflow) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = 2147483647;
            debugPrintfEXT("overflow == %d", foo + 4);
        }
    )glsl";
    BasicComputeTest(shader_source, "overflow == -2147483645");
}

TEST_F(NegativeDebugPrintf, TwoFloats) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("Here are two float values %f, %F", 1.0, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "Here are two float values 1.000000, 3.141500");
}

TEST_F(NegativeDebugPrintf, FloatPrecision) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("smaller float value %1.2f", myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "smaller float value 3.14");
}

TEST_F(NegativeDebugPrintf, TextBeforeAndAfter) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = -135;
            debugPrintfEXT("Here's an integer %i with text before and after it", foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "Here's an integer -135 with text before and after it");
}

TEST_F(NegativeDebugPrintf, IntOctal) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = 256;
            debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "Here's an integer in octal 400 and hex 0x100");
}

TEST_F(NegativeDebugPrintf, IntOctalNegative) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = -4;
            debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "Here's an integer in octal 37777777774 and hex 0xfffffffc");
}

TEST_F(NegativeDebugPrintf, IntNegative) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = -135;
            debugPrintfEXT("%d is a negative integer", foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "-135 is a negative integer");
}

TEST_F(NegativeDebugPrintf, FloatVector2) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec2 floatvec = vec2(1.2f, 2.2f);
            debugPrintfEXT("vector of floats %v2f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.200000, 2.200000");
}

TEST_F(NegativeDebugPrintf, FloatVector3) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec3 floatvec = vec3(1.2f, 2.2f, 3.2f);
            debugPrintfEXT("vector of floats %v3f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.200000, 2.200000, 3.200000");
}

TEST_F(NegativeDebugPrintf, FloatVector4) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
            debugPrintfEXT("vector of floats %v4f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.200000, 2.200000, 3.200000");
}

TEST_F(NegativeDebugPrintf, FloatVectorPrecision) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
            debugPrintfEXT("vector of floats %1.2v4f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.20, 2.20, 3.20, 4.20");
}

TEST_F(NegativeDebugPrintf, FloatVectorPrecisionZeroPad) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec2 floatvec = vec2(1.2f, 2.2f);
            debugPrintfEXT("vector of floats %1.2v4f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.20, 2.20, 0.00, 0.00");
}

TEST_F(NegativeDebugPrintf, FloatVectorZeroPad) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec2 floatvec = vec2(1.2f, 2.2f);
            debugPrintfEXT("vector of floats %v4f", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.200000, 2.200000, 0.000000, 0.000000");
}

TEST_F(NegativeDebugPrintf, FloatVectorScientificNotation) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec2 floatvec = vec2(1.2f, 2.2f);
            debugPrintfEXT("vector of floats %v2e", floatvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of floats 1.200000e+00, 2.200000e+00");
}

TEST_F(NegativeDebugPrintf, IntVector) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            ivec3 intvec = ivec3(-4, 32, 64);
            debugPrintfEXT("vector of ints %v3d", intvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of ints -4, 32, 64");
}

TEST_F(NegativeDebugPrintf, IntVectorUnsigned) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uvec3 intvec = uvec3(1, 2, 3);
            debugPrintfEXT("vector of ints %v3u", intvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of ints 1, 2, 3");
}

TEST_F(NegativeDebugPrintf, IntVectorHex) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            ivec3 intvec = ivec3(-4, 32, 64);
            debugPrintfEXT("vector of ints %v3x", intvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of ints fffffffc, 20, 40");
}

TEST_F(NegativeDebugPrintf, IntVectorZeroPad) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            ivec3 intvec = ivec3(1, 2, 3);
            debugPrintfEXT("vector of ints %v4d", intvec);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of ints 1, 2, 3, 0");
}

TEST_F(NegativeDebugPrintf, ScientificNotation) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float in sn %e and %E", myfloat, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float in sn 3.141500e+00 and 3.141500E+00");
}

TEST_F(NegativeDebugPrintf, ScientificNotationPrecision) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float in sn %1.2e and %1.2E", myfloat, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float in sn 3.14e+00 and 3.14E+00");
}

TEST_F(NegativeDebugPrintf, FloatShortest) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float in shortest %g and %G", myfloat, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float in shortest 3.1415 and 3.1415");
}

// TODO - This prints out  0x1.921cacp+1 vs 0x1.921cac0000000p+1 depending on Windows or not
TEST_F(NegativeDebugPrintf, DISABLED_FloatHex) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float in hex %a and %A", myfloat, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float in hex 0x1.921cacp+1 and 0X1.921CACP+1");
}

TEST_F(NegativeDebugPrintf, FloatHexPrecision) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float in hex %1.3a and %1.9A", myfloat, myfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "float in hex 0x1.922p+1 and 0X1.921CAC000P+1");
}

TEST_F(NegativeDebugPrintf, Int64) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
        }
    )glsl";
    BasicComputeTest(shader_source, "Here's an unsigned long 0x2000000000000001");
}

TEST_F(NegativeDebugPrintf, Int64Vector) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
            debugPrintfEXT("vector of ul %v4ul", vecul);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
}

TEST_F(NegativeDebugPrintf, Int64Hex) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
        }
    )glsl";
    BasicComputeTest(shader_source, "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
}

TEST_F(NegativeDebugPrintf, Int64VectorHex) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            u64vec2 vecul = u64vec2(bigvar, bigvar);
            debugPrintfEXT("vector of lx 0x%v2lx", vecul);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of lx 0x2000000000000001, 2000000000000001");
}

// TODO - Windows trims the leading values and will print 0x001 (Linux ignores the Precision)
TEST_F(NegativeDebugPrintf, DISABLED_Int64VectorHexPrecision) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            u64vec2 vecul = u64vec2(bigvar, bigvar);
            debugPrintfEXT("vector of lx 0x%1.3v2lx", vecul);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of lx 0x2000000000000001, 2000000000000001");
}

TEST_F(NegativeDebugPrintf, Int64VectorDecimal) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            u64vec2 vecul = u64vec2(bigvar, bigvar);
            debugPrintfEXT("vector of lu %v2lu", vecul);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of lu 2305843009213693953, 2305843009213693953");
}

TEST_F(NegativeDebugPrintf, Float64) {
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        void main() {
            float64_t foo = 1.23456789;
            float bar = 1.23456789;
            debugPrintfEXT("floats and doubles %f %f %f %f %f", foo, bar, foo, bar, foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "floats and doubles 1.234568 1.234568 1.234568 1.234568 1.234568");
}

TEST_F(NegativeDebugPrintf, Float64Vector) {
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        void main() {
            float64_t foo = 1.23456789;
            f64vec3 vecfloat = f64vec3(foo, foo, foo);
            debugPrintfEXT("vector of float64 %v3f", vecfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of float64 1.234568, 1.234568, 1.234568");
}

TEST_F(NegativeDebugPrintf, Float64VectorPrecision) {
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        void main() {
            float64_t foo = 1.23456789;
            f64vec2 vecfloat = f64vec2(foo, foo);
            debugPrintfEXT("vector of float64 %1.2v2f", vecfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of float64 1.23, 1.23");
}

TEST_F(NegativeDebugPrintf, FloatMix) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        void main() {
            float16_t a = float16_t(3.3333333333);
            float b = 3.3333333333;
            float64_t c = float64_t(3.3333333333);
            debugPrintfEXT("%f %f %f %f", a, b, c, 3.3333333333f);
        }
    )glsl";
    BasicComputeTest(shader_source, "3.332031 3.333333 3.333333 3.333333");
}

TEST_F(NegativeDebugPrintf, Float16) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        void main() {
            float16_t foo = float16_t(3.3);
            float bar = 3.3;
            debugPrintfEXT("32, 16, 32 | %f %f %f", bar, foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "32, 16, 32 | 3.300000 3.298828 3.300000");
}

TEST_F(NegativeDebugPrintf, Float16Vector) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        void main() {
            float16_t foo = float16_t(3.3);
            f16vec2 vecfloat = f16vec2(foo, foo);
            debugPrintfEXT("vector of float16 %v2f", vecfloat);
        }
    )glsl";
    BasicComputeTest(shader_source, "vector of float16 3.298828, 3.298828");
}

TEST_F(NegativeDebugPrintf, Float16Precision) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        void main() {
            float16_t foo = float16_t(3.3);
            debugPrintfEXT("float16 %1.3f", foo);
        }
    )glsl";
    BasicComputeTest(shader_source, "float16 3.299");
}

// TODO casting is wrong
TEST_F(NegativeDebugPrintf, Int16) {
    AddRequiredFeature(vkt::Feature::shaderInt16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
        void main() {
            uint16_t foo = uint16_t(123);
            int16_t bar = int16_t(-123);
            debugPrintfEXT("unsigned and signed %d %d", foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 123 -123");
}

TEST_F(NegativeDebugPrintf, Int16Vector) {
    AddRequiredFeature(vkt::Feature::shaderInt16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
        void main() {
            uint16_t foo = uint16_t(123);
            u16vec2 fooVec = u16vec2(foo, foo);
            int16_t bar = int16_t(-123);
            i16vec2 barVec = i16vec2(bar, bar);
            debugPrintfEXT("unsigned and signed %v2d | %v2d", fooVec, barVec);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 123, 123 | -123, -123");
}

TEST_F(NegativeDebugPrintf, Int16Hex) {
    AddRequiredFeature(vkt::Feature::shaderInt16);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
        void main() {
            uint16_t foo = uint16_t(123);
            int16_t bar = int16_t(-123);
            debugPrintfEXT("unsigned and signed 0x%x 0x%x", foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 0x7b 0xff85");
}

TEST_F(NegativeDebugPrintf, Int8) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
        void main() {
            uint8_t foo = uint8_t(123);
            int8_t bar = int8_t(-123);
            debugPrintfEXT("unsigned and signed %d %d", foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 123 -123");
}

TEST_F(NegativeDebugPrintf, Int8Vector) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
        void main() {
            uint8_t foo = uint8_t(123);
            u8vec2 fooVec = u8vec2(foo, foo);
            int8_t bar = int8_t(-123);
            i8vec2 barVec = i8vec2(bar, bar);
            debugPrintfEXT("unsigned and signed %v2d | %v2d", fooVec, barVec);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 123, 123 | -123, -123");
}

TEST_F(NegativeDebugPrintf, Int8Hex) {
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
        void main() {
            uint8_t foo = uint8_t(123);
            int8_t bar = int8_t(-123);
            debugPrintfEXT("unsigned and signed 0x%x 0x%x", foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "unsigned and signed 0x7b 0x85");
}

TEST_F(NegativeDebugPrintf, BoolAsHex) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            bool foo = true;
            bool bar = false;
            debugPrintfEXT("bool fun 0x%x%x%x%x", foo, bar, foo, bar);
        }
    )glsl";
    BasicComputeTest(shader_source, "bool fun 0x1010");
}

TEST_F(NegativeDebugPrintf, BoolVector) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        bool foo(int x) {
            return x == 1;
        }

        void main() {
            bool a = foo(1);
            bool b = !a;
            bvec2 c = bvec2(a, b);
            debugPrintfEXT("bvec2 %v2u", c);
        }
    )glsl";
    BasicComputeTest(shader_source, "bvec2 1, 0");
}

TEST_F(NegativeDebugPrintf, BoolNonConstant) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        bool foo(int x) {
            return x == 1;
        }

        void main() {
            debugPrintfEXT("bool %u", foo(1));
        }
    )glsl";
    BasicComputeTest(shader_source, "bool 1");
}

TEST_F(NegativeDebugPrintf, Int32Before) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uint idx_32 = 42;
            uint a = 1;
            uint b = 2;
            uint c = 3;
            debugPrintfEXT("Results: %8d %8x %8x %8x\n", idx_32, a, b, c);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results:       42        1        2        3");
}

TEST_F(NegativeDebugPrintf, Int32After) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uint idx_32 = 42;
            uint a = 1;
            uint b = 2;
            uint c = 3;
            debugPrintfEXT("Results: %8x %8x %8x %8d\n", a, b, c, idx_32);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results:        1        2        3       42");
}

TEST_F(NegativeDebugPrintf, Int64Before) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t idx_64 = 42;
            uint a = 1;
            uint b = 2;
            uint c = 3;

            debugPrintfEXT("Results: %8lu %8x %8x %8x\n", idx_64, a, b, c);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results:       42        1        2        3");
}

TEST_F(NegativeDebugPrintf, Int64After) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t idx_64 = 42;
            uint a = 1;
            uint b = 2;
            uint c = 3;
            debugPrintfEXT("Results: %8x %8x %8x %8lu\n", a, b, c, idx_64);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results:        1        2        3       42");
}

TEST_F(NegativeDebugPrintf, Int64Signed) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            int64_t zero = 0;
            int64_t neg1 = -1;
            int64_t neg42 = -42;
            int64_t negMin = -9223372036854775808l; // INT64_MIN
            debugPrintfEXT("Results: %ld %ld %ld %ld\n", zero, neg1, neg42, negMin);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results: 0 -1 -42 -9223372036854775808");
}

TEST_F(NegativeDebugPrintf, Int64SignedMix) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            int64_t pos = 42;
            int64_t neg = -42;
            int a = 1;
            debugPrintfEXT("Results: %d %ld %ld %d\n", a, pos, neg, 2);
        }
    )glsl";
    BasicComputeTest(shader_source, "Results: 1 42 -42 2");
}

TEST_F(NegativeDebugPrintf, Empty) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = -135;
            debugPrintfEXT("First printf with a %% and no value");
            debugPrintfEXT("Second printf with a value %i", foo);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("First printf with a % and no value");
    m_errorMonitor->SetDesiredInfo("Second printf with a value -135");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MultipleFunctions) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        int data = 0;

        void fn2(bool x) {
            if (x) {
                debugPrintfEXT("fn2 x [%d]", data++);
            } else {
                debugPrintfEXT("fn2 !x [%d]", data++);
            }
        }

        void fn1() {
            debugPrintfEXT("fn1 [%d]", data++);
            fn2(true);
            fn2(false);
        }

        void main() {
            debugPrintfEXT("START");
            fn1();
            debugPrintfEXT("END");
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("START");
    m_errorMonitor->SetDesiredInfo("fn1 [0]");
    m_errorMonitor->SetDesiredInfo("fn2 x [1]");
    m_errorMonitor->SetDesiredInfo("fn2 !x [2]");
    m_errorMonitor->SetDesiredInfo("END");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, Fragment) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;

        void main() {
            if (gl_FragCoord.x > 10 && gl_FragCoord.x < 11) {
                if (gl_FragCoord.y > 10 && gl_FragCoord.y < 12) {
                    debugPrintfEXT("gl_FragCoord.xy %1.2f, %1.2f\n", gl_FragCoord.x, gl_FragCoord.y);
                }
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("gl_FragCoord.xy 10.50, 10.50");
    m_errorMonitor->SetDesiredInfo("gl_FragCoord.xy 10.50, 11.50");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, HLSL) {
    TEST_DESCRIPTION("Make sure HLSL input works");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    // [numthreads(64, 1, 1)]
    // void main(uint2 launchIndex: SV_DispatchThreadID) {
    //     if (launchIndex.x > 1 && launchIndex.x < 4) {
    //         printf("launchIndex %v2d", launchIndex);
    //    }
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
         %29 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %launchIndex
               OpExecutionMode %main LocalSize 64 1 1
         %27 = OpString "launchIndex %v2d"
               OpSource HLSL 500
               OpName %main "main"
               OpName %launchIndex "launchIndex"
               OpDecorate %launchIndex BuiltIn GlobalInvocationId
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
     %uint_0 = OpConstant %uint 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1
       %bool = OpTypeBool
     %uint_4 = OpConstant %uint 4
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%launchIndex = OpVariable %_ptr_Input_v3uint Input
       %main = OpFunction %void None %3
          %5 = OpLabel
      %param = OpVariable %_ptr_Function_v2uint Function
         %35 = OpLoad %v3uint %launchIndex
         %36 = OpCompositeExtract %uint %35 0
         %37 = OpCompositeExtract %uint %35 1
         %38 = OpCompositeConstruct %v2uint %36 %37
               OpStore %param %38
         %43 = OpAccessChain %_ptr_Function_uint %param %uint_0
         %44 = OpLoad %uint %43
         %45 = OpUGreaterThan %bool %44 %uint_1
         %46 = OpAccessChain %_ptr_Function_uint %param %uint_0
         %47 = OpLoad %uint %46
         %48 = OpULessThan %bool %47 %uint_4
         %49 = OpLogicalAnd %bool %45 %48
               OpSelectionMerge %53 None
               OpBranchConditional %49 %50 %53
         %50 = OpLabel
         %51 = OpLoad %v2uint %param
         %52 = OpExtInst %void %29 1 %27 %51
               OpBranch %53
         %53 = OpLabel
               OpReturn
               OpFunctionEnd

    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("launchIndex 2, 0");
    m_errorMonitor->SetDesiredInfo("launchIndex 3, 0");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MultiDraw) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                }
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
        )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 0;
    buffer_in.Memory().Unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint16_t *ptr = static_cast<uint16_t *>(buffer.Memory().Map());
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    buffer.Memory().Unmap();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 1;
    buffer_in.Memory().Unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here's a smaller float value 3.14");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MeshTaskShadersNV) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    static const char taskShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 32) in;
        uint invocationID = gl_LocalInvocationID.x;
        void main() {
            if (invocationID == 0) {
                gl_TaskCountNV = 1;
                debugPrintfEXT("hello from task shader");
            }
        }
        )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        uint invocationID = gl_LocalInvocationID.x;
        void main() {
            if (invocationID == 0) {
                debugPrintfEXT("hello from mesh shader");
            }
        }
        )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawMeshTasksNV(m_command_buffer.handle(), 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("hello from task shader");
    m_errorMonitor->SetDesiredInfo("hello from mesh shader");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPL) {
    TEST_DESCRIPTION("Verify debugPrintfEXT works with GPL");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                    case 2:
                        debugPrintfEXT("Here's an integer %i with text before and after it", foo);
                        break;
                    case 3:
                        foo = 256;
                        debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
                        break;
                    case 4:
                        debugPrintfEXT("%d is a negative integer", foo);
                        break;
                    case 5:
                        vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
                        debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);
                        break;
                    case 6:
                        debugPrintfEXT("Here's a float in sn %e", myfloat);
                        break;
                    case 7:
                        debugPrintfEXT("Here's a float in sn %1.2e", myfloat);
                        break;
                    case 8:
                        debugPrintfEXT("Here's a float in shortest %g", myfloat);
                        break;
                    case 9:
                        debugPrintfEXT("Here's a float in hex %1.9a", myfloat);
                        break;
                    case 10:
                        debugPrintfEXT("First printf with a %% and no value");
                        debugPrintfEXT("Second printf with a value %i", foo);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    std::vector<char const *> messages;
    messages.emplace_back("Here are two float values 1.000000, 3.141500");
    messages.emplace_back("Here's a smaller float value 3.14");
    messages.emplace_back("Here's an integer -135 with text before and after it");
    messages.emplace_back("Here's an integer in octal 400 and hex 0x100");
    messages.emplace_back("-135 is a negative integer");
    messages.emplace_back("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    messages.emplace_back("Here's a float in sn 3.141500e+00");
    messages.emplace_back("Here's a float in sn 3.14e+00");
    messages.emplace_back("Here's a float in shortest 3.1415");
    messages.emplace_back("Here's a float in hex 0x1.921cac000p+1");
    // Two error messages have to be last in the vector
    messages.emplace_back("First printf with a % and no value");
    messages.emplace_back("Second printf with a value -135");
    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.Memory().Map();
        data[0] = i;
        buffer_in.Memory().Unmap();
        m_errorMonitor->SetDesiredInfo(messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredInfo(messages[i + 1]);
            i++;
        }
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, GPLMultiDraw) {
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source);

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 0;
    buffer_in.Memory().Unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint16_t *ptr = static_cast<uint16_t *>(buffer.Memory().Map());
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    buffer.Memory().Unmap();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 1;
    buffer_in.Memory().Unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here's a smaller float value 3.14");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLInt64) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    if (!m_device->Physical().Features().shaderInt64) {
        GTEST_SKIP() << "shaderInt64 not supported";
    }
    InitRenderTarget();

    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source_int64 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
                        break;
                    case 1:
                        u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
                        debugPrintfEXT("Here's a vector of ul %v4ul", vecul);
                        break;
                    case 2:
                        debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source_int64);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 0;
    buffer_in.Memory().Unmap();
    m_errorMonitor->SetDesiredInfo("Here's an unsigned long 0x2000000000000001");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 1;
    buffer_in.Memory().Unmap();
    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.Memory().Map();
    data[0] = 2;
    buffer_in.Memory().Unmap();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                         "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLFragment) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // "Normal" sets
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_});
    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.Memory().Unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.Memory().Unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    static const char vert_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
                debugPrintfEXT("Vertex shader %i, 0x%x", gl_VertexIndex, t);
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
            debugPrintfEXT("Fragment shader 0x%x\n", t);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vert_shader, frag_shader);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredInfo("Fragment shader 0x2040608");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLFragmentIndependentSets) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // Independent sets
    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto vs_layout = pipeline_layout_vs.handle();
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto fs_layout = pipeline_layout_fs.handle();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.Memory().Unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.Memory().Unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    static const char vertshader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
                debugPrintfEXT("Vertex shader %i, 0x%x", gl_VertexIndex, t);
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pre_raster_lib(*this);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.vp_state_ci_.pViewports = &viewport;
    pre_raster_lib.vp_state_ci_.pScissors = &scissor;
    pre_raster_lib.gp_ci_.layout = vs_layout;
    pre_raster_lib.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
            debugPrintfEXT("Fragment shader 0x%x\n", t);
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper frag_shader_lib(*this);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib.gp_ci_.layout = fs_layout;
    frag_shader_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline pipe(*m_device, exe_pipe_ci);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredInfo("Fragment shader 0x2040608");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectsGraphics) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("vertex %d with value %f", gl_VertexIndex, myfloat);
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
    )glsl";
    const vkt::Shader vs(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vs.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("vertex 0 with value 3.141500");
    m_errorMonitor->SetDesiredInfo("vertex 1 with value 3.141500");
    m_errorMonitor->SetDesiredInfo("vertex 2 with value 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjects) {
    TEST_DESCRIPTION("Verify that all various types of output works as expect with shader object");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            int bar = 256;
            vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);

            debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
            debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
            debugPrintfEXT("Here's an integer %i with text before and after it", foo);
            debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", bar, bar);
            debugPrintfEXT("%d is a negative integer", foo);
            debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);
            debugPrintfEXT("Here's a float in sn %e", myfloat);
            debugPrintfEXT("Here's a float in sn %1.2e", myfloat);
            debugPrintfEXT("Here's a float in shortest %g", myfloat);
            debugPrintfEXT("Here's a float in hex %1.9a", myfloat);
        }
    )glsl";
    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source));

    m_command_buffer.Begin();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, stages, &cs.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Here are two float values 1.000000, 3.141500");
    m_errorMonitor->SetDesiredInfo("Here's a smaller float value 3.14");
    m_errorMonitor->SetDesiredInfo("Here's an integer -135 with text before and after it");
    m_errorMonitor->SetDesiredInfo("Here's an integer in octal 400 and hex 0x100");
    m_errorMonitor->SetDesiredInfo("-135 is a negative integer");
    m_errorMonitor->SetDesiredInfo("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    m_errorMonitor->SetDesiredInfo("Here's a float in sn 3.141500e+00");
    m_errorMonitor->SetDesiredInfo("Here's a float in sn 3.14e+00");
    m_errorMonitor->SetDesiredInfo("Here's a float in shortest 3.1415");
    m_errorMonitor->SetDesiredInfo("Here's a float in hex 0x1.921cac000p+1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectsInt64) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
            u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
            debugPrintfEXT("Here's a vector of ul %v4ul", vecul);
            debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
        }
    )glsl";
    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source));

    m_command_buffer.Begin();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, stages, &cs.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Here's an unsigned long 0x2000000000000001");
    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                         "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectsMultiDraw) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                float myfloat = 3.1415f;
                debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
    )glsl";
    const vkt::Shader vs(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source));

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vs.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint16_t *ptr = static_cast<uint16_t *>(buffer.Memory().Map());
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    buffer.Memory().Unmap();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MeshTaskShaderObjects) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shader objects.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    static const char *taskShaderText = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_debug_printf : enable
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main() {
            debugPrintfEXT("hello from task shader");
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    static const char *meshShaderText = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_debug_printf : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            debugPrintfEXT("hello from mesh shader");
        }
    )glsl";

    const vkt::Shader ts(*m_device, VK_SHADER_STAGE_TASK_BIT_EXT,
                         GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, taskShaderText, SPV_ENV_VULKAN_1_3));
    const vkt::Shader ms(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                         GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, meshShaderText, SPV_ENV_VULKAN_1_3));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT,
                                            VK_SHADER_STAGE_TASK_BIT_EXT,
                                            VK_SHADER_STAGE_MESH_BIT_EXT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                   VK_NULL_HANDLE, ts.handle(),    ms.handle()};
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 7u, stages, shaders);
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("hello from task shader");
    m_errorMonitor->SetDesiredInfo("hello from mesh shader");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, VertexFragmentSeparateShader) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char vert_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        const vec2 vertices[3] = vec2[]( vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(0.0, 1.0) );
        void main() {
            debugPrintfEXT("Vertex value is %i", 4);
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        layout(location = 0) out vec4 c_out;
        void main() {
            debugPrintfEXT("Fragment value is %i", 8);
            c_out = vec4(0.0);
        }
    )glsl";
    VkShaderObj vs(this, vert_shader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, frag_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_.pViewports = &viewport;
    pipe.vp_state_ci_.pScissors = &scissor;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Vertex value is 4");
    }
    m_errorMonitor->SetDesiredInfo("Fragment value is 8");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, VertexFragmentMultiEntrypoint) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // void vert_main() {
    //     debugPrintfEXT("Vertex value is %i", 4);
    //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    // }
    // layout(location = 0) out vec4 c_out;
    // void frag_main() {
    //     debugPrintfEXT("Fragment value is %i", 8);
    //     c_out = vec4(0.0);
    // }
    const char *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %9 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %c_out
               OpEntryPoint Vertex %vert_main "vert_main" %_ %gl_VertexIndex
               OpExecutionMode %frag_main OriginUpperLeft
   %vert_str = OpString "Vertex value is %i"
   %frag_str = OpString "Fragment value is %i"
               OpDecorate %c_out Location 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
      %int_8 = OpConstant %int 8
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2
     %uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %float_n1 = OpConstant %float -1
         %24 = OpConstantComposite %v2float %float_n1 %float_n1
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v2float %float_1 %float_n1
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_1
         %29 = OpConstantComposite %_arr_v2float_uint_3 %24 %26 %28
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
      %int_3 = OpConstant %int 3
%_ptr_Function__arr_v2float_uint_3 = OpTypePointer Function %_arr_v2float_uint_3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %c_out = OpVariable %_ptr_Output_v4float Output
         %16 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %vert_main = OpFunction %void None %3
          %5 = OpLabel
  %indexable = OpVariable %_ptr_Function__arr_v2float_uint_3 Function
         %10 = OpExtInst %void %9 1 %vert_str %int_4
         %32 = OpLoad %int %gl_VertexIndex
         %34 = OpSMod %int %32 %int_3
               OpStore %indexable %29
         %38 = OpAccessChain %_ptr_Function_v2float %indexable %34
         %39 = OpLoad %v2float %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpCompositeExtract %float %39 1
         %42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
         %44 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %44 %42
               OpReturn
               OpFunctionEnd
       %frag_main = OpFunction %void None %3
          %f5 = OpLabel
         %f10 = OpExtInst %void %9 1 %frag_str %int_8
               OpStore %c_out %16
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "vert_main");
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "frag_main");

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_.pViewports = &viewport;
    pipe.vp_state_ci_.pScissors = &scissor;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredInfo("Vertex value is 4");
    }
    m_errorMonitor->SetDesiredInfo("Fragment value is 8");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectFragment) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT,
                                  GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl));
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source));

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    m_command_buffer.BindVertFragShader(vert_shader, frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectCompute) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source));

    m_command_buffer.Begin();
    m_command_buffer.BindCompShader(comp_shader);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, SetupErrorVersion) {
    TEST_DESCRIPTION("Verify DebugPrintF can gracefully fail if not using Vulkan 1.1+");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;
    RETURN_IF_SKIP(InitFramework(&features));

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "Currently disabled for Portability";
    }

    m_errorMonitor->SetDesiredError("requires Vulkan 1.1 or later");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();

    // Still make sure we can use Vulkan as expected without errors

    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
        )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeDebugPrintf, LocalSizeId) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
         %30 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionModeId %main LocalSizeId %8 %9 %10
         %29 = OpString "TEST"
               OpDecorate %8 SpecId 0
               OpDecorate %9 SpecId 1
               OpDecorate %10 SpecId 2
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %8 = OpSpecConstant %uint 1
          %9 = OpSpecConstant %uint 1
         %10 = OpSpecConstant %uint 1
       %bool = OpTypeBool
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_0
         %18 = OpLoad %uint %17
         %19 = OpIEqual %bool %18 %uint_0
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %21
         %20 = OpLabel
         %23 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_1
         %24 = OpLoad %uint %23
         %25 = OpIEqual %bool %24 %uint_0
               OpBranch %21
         %21 = OpLabel
         %26 = OpPhi %bool %19 %6 %25 %20
               OpSelectionMerge %28 None
               OpBranchConditional %26 %27 %28
         %27 = OpLabel
         %31 = OpExtInst %void %30 1 %29
               OpBranch %28
         %28 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    uint32_t workgroup_size[3] = {32, 32, 1};
    VkSpecializationMapEntry entries[3];
    entries[0] = {0, 0, sizeof(uint32_t)};
    entries[1] = {1, sizeof(uint32_t), sizeof(uint32_t)};
    entries[2] = {2, sizeof(uint32_t) * 2, sizeof(uint32_t)};

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 3;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t) * 3;
    specialization_info.pData = workgroup_size;

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM,
                                             &specialization_info);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 32, 32, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("TEST");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, Maintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    std::vector<uint32_t> shader;
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_COMPUTE_BIT, shader_source, shader);

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
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    RETURN_IF_SKIP(InitDebugPrintfFramework(nullptr, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets + 1;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineNotReserved) {
    TEST_DESCRIPTION("Do not reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineGraphics) {
    TEST_DESCRIPTION("Do not reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.gp_ci_.layout = pipe_layout.handle();
        pipe.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.gp_ci_.layout = pipe_layout.handle();
        pipe.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineGPL) {
    TEST_DESCRIPTION("Do not reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        vkt::SimpleGPL pipe(*this, pipe_layout.handle(), shader_source);

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        vkt::SimpleGPL pipe(*this, pipe_layout.handle(), shader_source);

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_UseAllDescriptorSlotsShaderObjectReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework(nullptr, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    auto cs_spirv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source);

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets + 1;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    std::vector<VkDescriptorSetLayout> layouts;
    for (uint32_t i = 0; i < set_limit; i++) {
        layouts.push_back(descriptor_set.layout_.handle());
    }

    // First try to use too many sets in the Shader Object
    {
        m_errorMonitor->SetDesiredWarning(
            "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "VkShaderEXT created with it");

        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit, layouts.data()));
        m_errorMonitor->VerifyFound();

        m_command_buffer.Begin();
        m_command_buffer.BindCompShader(comp_shader);
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit - 1, layouts.data()));

        m_command_buffer.Begin();
        m_command_buffer.BindCompShader(comp_shader);
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_UseAllDescriptorSlotsShaderObjectNotReserved) {
    TEST_DESCRIPTION("Dont reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    auto cs_spirv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source);

    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    std::vector<VkDescriptorSetLayout> layouts;
    for (uint32_t i = 0; i < set_limit; i++) {
        layouts.push_back(descriptor_set.layout_.handle());
    }

    // First try to use too many sets in the Shader Object
    {
        m_errorMonitor->SetDesiredWarning(
            "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "VkShaderEXT created with it");

        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit, layouts.data()));
        m_errorMonitor->VerifyFound();

        m_command_buffer.Begin();
        m_command_buffer.BindCompShader(comp_shader);
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit - 1, layouts.data()));

        m_command_buffer.Begin();
        m_command_buffer.BindCompShader(comp_shader);
        vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredInfo("float == 3.141500");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_ShaderObjectMultiCreate) {
    TEST_DESCRIPTION("Make sure we instrument every index of VkShaderCreateInfoEXT");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkShaderCreateInfoEXT shader_create_infos[2];
    shader_create_infos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    shader_create_infos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2, shader_create_infos, nullptr, shaders);

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 2, stages, shaders);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}

TEST_F(NegativeDebugPrintf, OverflowBuffer) {
    TEST_DESCRIPTION("go over the default VK_LAYER_PRINTF_BUFFER_SIZE limit");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
        void main() {
            debugPrintfEXT("WorkGroup %v3u | Invocation %v3u\n", gl_WorkGroupID, gl_LocalInvocationID);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 4, 4, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredWarning(
        "Debug Printf message was truncated due to a buffer size (1024) being too small for the messages");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, OverflowBufferLoop) {
    TEST_DESCRIPTION("go over the default VK_LAYER_PRINTF_BUFFER_SIZE limit... by a LOT");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
        void main() {
            for (uint i = 0; i < 2048; i++) {
                debugPrintfEXT("WorkGroup %v3u | Invocation %v3u\n", gl_WorkGroupID, gl_LocalInvocationID);
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 4, 4, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredWarning(
        "Debug Printf message was truncated due to a buffer size (1024) being too small for the messages");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

void NegativeDebugPrintf::BasicFormattingTest(const char *shader, bool warning) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(warning ? kWarningBit : kErrorBit, "DEBUG-PRINTF-FORMATTING");
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MisformattedNoVectorSize) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %v");
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedLargeVectorSize) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec4 myVec = vec4(0.0);
            debugPrintfEXT("vector of %v5f", myVec);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedSmallVectorSize) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec4 myVec = vec4(0.0);
            debugPrintfEXT("vector of %v1f", myVec);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNoSpecifier1) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %v3 f", vec3(0));
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNoSpecifier2) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %1.2l", 0);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedUnknown1) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %q");
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedUnknown2) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %U", 3);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedUnknown3) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("vector of %1,2f", 4.0f);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedExtraArguments) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("%d %d", 0, 1, 2, 3);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedNoModifiers) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("test", 3);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedIsloatedPercent) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("test % this");
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNotEnoughArguments) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("test %d %d %d", 3);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNoArguments) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("%d %d");
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNotVectorArg) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec3 foo = vec3(1);
            debugPrintfEXT("%v3f %v3f %v3f", vec3(0), foo, foo.x);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNotVectorParam) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec3 foo = vec3(1);
            debugPrintfEXT("%v3f %v3f %f", vec3(0), foo, foo);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedVectorSmall) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("%v3f", vec2(0));
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedVectorLarge) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("%v3f", vec4(0));
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedFloat1) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float foo = 1.0f;
            debugPrintfEXT("%d", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedFloat2) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            int foo = 4;
            debugPrintfEXT("%f", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedFloatVector1) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            vec3 foo = vec3(1);
            debugPrintfEXT("%v3d", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedFloatVector2) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uvec3 foo = uvec3(1);
            debugPrintfEXT("%v3f", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, Misformatted64Int1) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t foo = 0x2000000000000001ul;
            debugPrintfEXT("%u", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, Misformatted64Int2) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            int foo = 4;
            debugPrintfEXT("%lu", 4);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, Misformatted64IntVector1) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            u64vec2 vecul = u64vec2(bigvar, bigvar);
            debugPrintfEXT("0x%v2x", vecul);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, Misformatted64IntVector2) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            uvec2 foo = uvec2(1);
            debugPrintfEXT("0x%v2lx", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, Misformatted64Bool) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        void main() {
            bool foo = true;
            debugPrintfEXT("%lu", foo);
        }
    )glsl";
    BasicFormattingTest(shader_source, true);
}

TEST_F(NegativeDebugPrintf, MisformattedEmptyString) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("");
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, MisformattedNewLine) {
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            uint x = 3;
            debugPrintfEXT("\n\t%v3f\t\n", x);
        }
    )glsl";
    BasicFormattingTest(shader_source);
}

TEST_F(NegativeDebugPrintf, ValidationAbort) {
    TEST_DESCRIPTION("Verify that aborting DebugPrintf is safe.");
    RETURN_IF_SKIP(InitDebugPrintfFramework());

    PFN_vkSetPhysicalDeviceFeaturesEXT fpvkSetPhysicalDeviceFeaturesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFeaturesEXT fpvkGetOriginalPhysicalDeviceFeaturesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFeaturesEXT, fpvkGetOriginalPhysicalDeviceFeaturesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceFeatures features = {};
    fpvkGetOriginalPhysicalDeviceFeaturesEXT(Gpu(), &features);

    // Disable features so initialization aborts
    features.vertexPipelineStoresAndAtomics = false;
    features.fragmentStoresAndAtomics = false;
    fpvkSetPhysicalDeviceFeaturesEXT(Gpu(), features);
    m_errorMonitor->SetDesiredError("DebugPrintf is being disabled");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();

    // Still make sure we can use Vulkan as expected without errors
    InitRenderTarget();

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeDebugPrintf, DualPipelines) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe_debug(*this);
    pipe_debug.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_debug.CreateComputePipeline();

    CreateComputePipelineHelper pipe_normal(*this);
    pipe_normal.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_normal.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // no print

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_debug.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // print
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // print

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_normal.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // no print

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_debug.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // print
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_errorMonitor->SetDesiredInfo("float == 3.141500");

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DualCommandBufferHalfPrint) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe_debug(*this);
    pipe_debug.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_debug.CreateComputePipeline();

    CreateComputePipelineHelper pipe_normal(*this);
    pipe_normal.CreateComputePipeline();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    cb0.Begin();
    vk::CmdBindPipeline(cb0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_normal.Handle());
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    cb0.End();

    cb1.Begin();
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_debug.Handle());
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    cb1.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");

    VkCommandBuffer cbs[2] = {cb0.handle(), cb1.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 2;
    submit.pCommandBuffers = cbs;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DualCommandBufferBothPrint) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(push_constant) uniform PushConstants { int x; } pc;
        void main() {
            debugPrintfEXT("int == %u", pc.x);
        }
    )glsl";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.pushConstantRangeCount = 1;
    pipe_layout_ci.pPushConstantRanges = &pc_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    uint32_t data = 4;
    cb0.Begin();
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &data);
    vk::CmdBindPipeline(cb0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    cb0.End();

    cb1.Begin();
    data = 8;
    vk::CmdPushConstants(cb1.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &data);
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    cb1.End();

    m_errorMonitor->SetDesiredInfo("int == 4");  // cb0
    m_errorMonitor->SetDesiredInfo("int == 8");  // cb1

    VkCommandBuffer cbs[2] = {cb0.handle(), cb1.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 2;
    submit.pCommandBuffers = cbs;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DualCommandBufferEmpty) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe_debug(*this);
    pipe_debug.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_debug.CreateComputePipeline();

    CreateComputePipelineHelper pipe_normal(*this);
    pipe_normal.CreateComputePipeline();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    vkt::CommandBuffer cb2(*m_device, m_command_pool);

    // Empty to make sure nothing breaks
    cb0.Begin();
    cb0.End();

    cb2.Begin();
    cb2.End();

    cb1.Begin();
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_debug.Handle());
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    cb1.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");

    VkCommandBuffer cbs[3] = {cb0.handle(), cb1.handle(), cb2.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 3;
    submit.pCommandBuffers = cbs;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DispatchIndirect) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    auto indirect_command = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    indirect_command->x = 1;
    indirect_command->y = 1;
    indirect_command->z = 1;
    indirect_buffer.Memory().Unmap();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DispatchBase) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatchBase(m_command_buffer.handle(), 1, 1, 1, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float == 3.141500");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DrawIndexed) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *vs_source = R"glsl(
    #version 450
        #extension GL_EXT_debug_printf : enable
        vec2 vertices[3];
        void main(){
            vertices[0] = vec2(-1.0, -1.0);
            vertices[1] = vec2( 1.0, -1.0);
            vertices[2] = vec2( 0.0,  1.0);
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
            debugPrintfEXT("gl_VertexIndex %u\n", gl_VertexIndex);
        }
    )glsl";
    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;
        void main() {
            if (gl_FragCoord.x > 0 && gl_FragCoord.x < 1 && gl_FragCoord.y > 0 && gl_FragCoord.y < 1) {
                debugPrintfEXT("Hit Fragment\n");
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 1, 2});

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 0");
    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 1");
    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 2");
    m_errorMonitor->SetDesiredInfo("Hit Fragment");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DrawIndexedIndirect) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;
        void main() {
            if (gl_FragCoord.x > 0 && gl_FragCoord.x < 1 && gl_FragCoord.y > 0 && gl_FragCoord.y < 1) {
                debugPrintfEXT("Hit Fragment\n");
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 1, 2});

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    auto indirect_command = static_cast<VkDrawIndexedIndirectCommand *>(indirect_buffer.Memory().Map());
    indirect_command->indexCount = 3;
    indirect_command->instanceCount = 1;
    indirect_command->firstIndex = 1;
    indirect_command->vertexOffset = 1;
    indirect_command->firstInstance = 1;
    indirect_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Hit Fragment");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DrawIndirectCount) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;
        void main() {
            if (gl_FragCoord.x > 0 && gl_FragCoord.x < 1 && gl_FragCoord.y > 0 && gl_FragCoord.y < 1) {
                debugPrintfEXT("Hit Fragment\n");
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    auto indirect_command = static_cast<VkDrawIndexedIndirectCommand *>(indirect_buffer.Memory().Map());
    indirect_command->indexCount = 3;
    indirect_command->instanceCount = 1;
    indirect_command->firstIndex = 1;
    indirect_command->vertexOffset = 1;
    indirect_command->firstInstance = 1;
    indirect_buffer.Memory().Unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 1;
    count_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirectCount(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Hit Fragment");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DrawIndexedIndirectCount) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;
        void main() {
            if (gl_FragCoord.x > 0 && gl_FragCoord.x < 1 && gl_FragCoord.y > 0 && gl_FragCoord.y < 1) {
                debugPrintfEXT("Hit Fragment\n");
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 1, 2});

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    auto indirect_command = static_cast<VkDrawIndexedIndirectCommand *>(indirect_buffer.Memory().Map());
    indirect_command->indexCount = 3;
    indirect_command->instanceCount = 1;
    indirect_command->firstIndex = 1;
    indirect_command->vertexOffset = 1;
    indirect_command->firstInstance = 1;
    indirect_buffer.Memory().Unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 1;
    count_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCount(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Hit Fragment");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DeviceGeneratedCommandsCompute) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    }

    VkIndirectCommandsLayoutTokenEXT token;
    token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("gl_NumWorkGroups %v3u\n", gl_NumWorkGroups);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkDeviceSize pre_process_size = 0;
    {
        VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper(&pipeline_info);
        dgc_mem_reqs.indirectCommandsLayout = command_layout.handle();
        dgc_mem_reqs.indirectExecutionSet = VK_NULL_HANDLE;
        dgc_mem_reqs.maxSequenceCount = 1;
        VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
        vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
        pre_process_size = mem_reqs2.memoryRequirements.size;
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = pre_process_size;
    vkt::Buffer pre_process_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

    VkDispatchIndirectCommand *block_buffer_ptr = (VkDispatchIndirectCommand *)block_buffer.Memory().Map();
    block_buffer_ptr->x = 2;
    block_buffer_ptr->y = 1;
    block_buffer_ptr->z = 1;
    block_buffer.Memory().Unmap();

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&pipeline_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = sizeof(VkDispatchIndirectCommand);
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = pre_process_buffer.Address();
    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("gl_NumWorkGroups 2, 1, 1");
    m_errorMonitor->SetDesiredInfo("gl_NumWorkGroups 2, 1, 1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, DeviceGeneratedCommandsGraphics) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_VERTEX_BIT is not supported.";
    }

    VkIndirectCommandsLayoutTokenEXT token;
    token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    char const *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("gl_VertexIndex %u\n", gl_VertexIndex);
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkDeviceSize pre_process_size = 0;
    {
        VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper(&pipeline_info);
        dgc_mem_reqs.indirectCommandsLayout = command_layout.handle();
        dgc_mem_reqs.indirectExecutionSet = VK_NULL_HANDLE;
        dgc_mem_reqs.maxSequenceCount = 1;
        VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
        vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
        pre_process_size = mem_reqs2.memoryRequirements.size;
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = pre_process_size;
    vkt::Buffer pre_process_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

    VkDrawIndirectCommand *block_buffer_ptr = (VkDrawIndirectCommand *)block_buffer.Memory().Map();
    block_buffer_ptr->vertexCount = 3;
    block_buffer_ptr->instanceCount = 1;
    block_buffer_ptr->firstVertex = 0;
    block_buffer_ptr->firstInstance = 0;
    block_buffer.Memory().Unmap();

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&pipeline_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = sizeof(VkDrawIndirectCommand);
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = pre_process_buffer.Address();
    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 0");
    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 1");
    m_errorMonitor->SetDesiredInfo("gl_VertexIndex 2");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Currently can't use IES with GPU-AV due to us creating invalid Pipeline Layouts
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/8806
TEST_F(NegativeDebugPrintf, DISABLED_DeviceGeneratedCommandsIES) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    }

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_COMPUTE_BIT};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;
    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    tokens[1].offset = sizeof(uint32_t);

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    char const *shader_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("Init Pipeline\n");
        }
    )glsl";
    char const *shader_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("IndirectExecutionSet Pipeline 1\n");
        }
    )glsl";
    char const *shader_source_3 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("IndirectExecutionSet Pipeline 2\n");
        }
    )glsl";

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreateComputePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    init_pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_1(*this, &pipe_flags2);
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, shader_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe_1.CreateComputePipeline();

    CreateComputePipelineHelper pipe_2(*this, &pipe_flags2);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, shader_source_3, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe_2.CreateComputePipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 3);
    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets[2];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 1;
    write_exe_sets[0].pipeline = pipe_1.Handle();
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 2;
    write_exe_sets[1].pipeline = pipe_2.Handle();
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 2, write_exe_sets);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkDeviceSize pre_process_size = 0;
    {
        VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper();
        dgc_mem_reqs.indirectCommandsLayout = command_layout.handle();
        dgc_mem_reqs.indirectExecutionSet = exe_set.handle();
        dgc_mem_reqs.maxSequenceCount = 1;
        VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
        vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
        pre_process_size = mem_reqs2.memoryRequirements.size;
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = pre_process_size;
    vkt::Buffer pre_process_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

    uint32_t *block_buffer_ptr = (uint32_t *)block_buffer.Memory().Map();
    block_buffer_ptr[0] = 2;  // pick pipeline 2
    VkDispatchIndirectCommand *indirect_command_ptr = (VkDispatchIndirectCommand *)(block_buffer_ptr + 1);
    indirect_command_ptr->x = 1;
    indirect_command_ptr->y = 1;
    indirect_command_ptr->z = 1;
    block_buffer.Memory().Unmap();

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = sizeof(uint32_t) + sizeof(VkDispatchIndirectCommand);
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = pre_process_buffer.Address();
    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, init_pipe.Handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("IndirectExecutionSet Pipeline 2");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MultipleComputePasses) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8763");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(binding = 0, set = 0) uniform UBO {
            float x;
        };
        void main() {
            debugPrintfEXT("float x == %f", x);
        }
    )glsl";
    char const *shader_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("float y == %f", 3.14f);
        }
    )glsl";

    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe1(*this);
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, shader_source_1, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe1.cp_ci_.layout = pipeline_layout.handle();
    pipe1.CreateComputePipeline();

    CreateComputePipelineHelper pipe2(*this);
    pipe2.cs_ = std::make_unique<VkShaderObj>(this, shader_source_2, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe2.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe2.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("float x ==");
    m_errorMonitor->SetDesiredInfo("float y ==");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, SpecConstant) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(constant_id = 0) const uint value = 22; // default
        void main() {
            debugPrintfEXT("value is = %u", value);
        }
    )glsl";

    CreateComputePipelineHelper pipe_22(*this);
    pipe_22.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_22.CreateComputePipeline();

    const uint32_t value_44 = 44;
    const uint32_t value_88 = 88;

    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info_44 = {1, &entry, sizeof(uint32_t), &value_44};
    VkSpecializationInfo spec_info_88 = {1, &entry, sizeof(uint32_t), &value_88};

    CreateComputePipelineHelper pipe_44(*this);
    pipe_44.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                SPV_SOURCE_GLSL, &spec_info_44);
    pipe_44.CreateComputePipeline();

    CreateComputePipelineHelper pipe_88(*this);
    pipe_88.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                SPV_SOURCE_GLSL, &spec_info_88);
    pipe_88.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_22.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_44.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_88.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("value is = 22");
    m_errorMonitor->SetDesiredInfo("value is = 44");
    m_errorMonitor->SetDesiredInfo("value is = 88");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, InlineUniformBlock) {
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform UBO0 { uint ubo_0; };
        layout(set = 0, binding = 1) uniform InlineUBO { uint dummy; uint inline_value; };
        layout(set = 0, binding = 2) uniform UBO2 { uint ubo_2; };
        void main() {
            debugPrintfEXT("binding [0] = %u | [1] = %u | [2] = %u", ubo_0, inline_value, ubo_2);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    VkDeviceAddress *buffer_ptr = (VkDeviceAddress *)buffer.Memory().Map();
    buffer_ptr[0] = 3;
    buffer.Memory().Unmap();

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 1;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 8, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, nullptr, 0, nullptr, &pool_inline_info);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    const uint32_t print_value = 5;
    VkWriteDescriptorSetInlineUniformBlock write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &print_value;

    VkWriteDescriptorSet descriptor_writes = vku::InitStructHelper(&write_inline_uniform);
    descriptor_writes.dstSet = descriptor_set.set_;
    descriptor_writes.dstBinding = 1;
    descriptor_writes.dstArrayElement = 4;  // offset 4 bytes
    descriptor_writes.descriptorCount = 4;  // Write 4 bytes
    descriptor_writes.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_writes, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("binding [0] = 3 | [1] = 5 | [2] = 3");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, StorageBufferLength) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(set = 0, binding = 0, scalar) buffer SSBO {
            float a;
            float b;
            vec4 c[]; // offset 8
        };

        void main() {
            debugPrintfEXT("c length = %u", c.length());
        }
    )glsl";

    vkt::Buffer buffer_large(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    // Will have a length of 8
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_large, 0, 136, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("c length = 8");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Will only have a length of 1 vec4
    vkt::Buffer buffer_small(*m_device, 24, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.Clear();
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_small, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // Need to rebind even for the length - https://gitlab.khronos.org/vulkan/vulkan/-/issues/4143
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("c length = 1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, StorageBufferLengthUpdateAfterBind) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_EXT_scalar_block_layout : enable

        layout(set = 0, binding = 0, scalar) buffer SSBO {
            float a;
            float b;
            vec4 c[]; // offset 8
        };

        void main() {
            debugPrintfEXT("c length = %u", c.length());
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    vkt::Buffer buffer_large(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // Will have a length of 8
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_large, 0, 136, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredInfo("c length = 8");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Will only have a length of 1 vec4
    vkt::Buffer buffer_small(*m_device, 24, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.Clear();
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_small, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredInfo("c length = 1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
