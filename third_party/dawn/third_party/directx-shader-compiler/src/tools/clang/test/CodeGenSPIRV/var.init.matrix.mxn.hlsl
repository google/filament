// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// TODO: optimize to generate constant composite for suitable initializers
// TODO: decompose matrix in initializer

// CHECK-DAG:  [[v3fc0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK-DAG: [[f2_1_2:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK-DAG: [[i2_1_2:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK-DAG:  [[v3fc1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // Constructor
// CHECK:      [[cc00:%[0-9]+]] = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
// CHECK-NEXT: [[cc01:%[0-9]+]] = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
// CHECK-NEXT: [[cc02:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[cc00]] [[cc01]]
// CHECK-NEXT: OpStore %mat1 [[cc02]]
    float2x3 mat1 = float2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    // All elements in a single {}
// CHECK-NEXT: [[cc03:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[cc04:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT: [[cc05:%[0-9]+]] = OpCompositeConstruct %v2float %float_5 %float_6
// CHECK-NEXT: [[cc06:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[cc03]] [[cc04]] [[cc05]]
// CHECK-NEXT: OpStore %mat2 [[cc06]]
    float3x2 mat2 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    // Each vector has its own {}
// CHECK-NEXT: [[cc07:%[0-9]+]] = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
// CHECK-NEXT: [[cc08:%[0-9]+]] = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
// CHECK-NEXT: [[cc09:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[cc07]] [[cc08]]
// CHECK-NEXT: OpStore %mat3 [[cc09]]
    float2x3 mat3 = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    // Wired & complicated {}s
// CHECK-NEXT: [[cc10:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[cc11:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT: [[cc12:%[0-9]+]] = OpCompositeConstruct %v2float %float_5 %float_6
// CHECK-NEXT: [[cc13:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[cc10]] [[cc11]] [[cc12]]
// CHECK-NEXT: OpStore %mat4 [[cc13]]
    float3x2 mat4 = {{1.0}, {2.0, 3.0}, 4.0, {{5.0}, {{6.0}}}};

    float scalar;
    float1 vec1;
    float2 vec2;
    float3 vec3;
    float4 vec4;

    // Mixed scalar and vector
// CHECK-NEXT: [[s:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: [[vec1:%[0-9]+]] = OpLoad %float %vec1
// CHECK-NEXT: [[vec2:%[0-9]+]] = OpLoad %v2float %vec2
// CHECK-NEXT: [[vec3:%[0-9]+]] = OpLoad %v3float %vec3
// CHECK-NEXT: [[vec2a:%[0-9]+]] = OpLoad %v2float %vec2
// CHECK-NEXT: [[vec4:%[0-9]+]] = OpLoad %v4float %vec4
// CHECK-NEXT: [[ce00:%[0-9]+]] = OpCompositeExtract %float [[vec2]] 0
// CHECK-NEXT: [[ce01:%[0-9]+]] = OpCompositeExtract %float [[vec2]] 1
// CHECK-NEXT: [[cc14:%[0-9]+]] = OpCompositeConstruct %v4float [[s]] [[vec1]] [[ce00]] [[ce01]]
// CHECK-NEXT: [[ce02:%[0-9]+]] = OpCompositeExtract %float [[vec3]] 0
// CHECK-NEXT: [[ce03:%[0-9]+]] = OpCompositeExtract %float [[vec3]] 1
// CHECK-NEXT: [[ce04:%[0-9]+]] = OpCompositeExtract %float [[vec3]] 2
// CHECK-NEXT: [[ce05:%[0-9]+]] = OpCompositeExtract %float [[vec2a]] 0
// CHECK-NEXT: [[ce06:%[0-9]+]] = OpCompositeExtract %float [[vec2a]] 1
// CHECK-NEXT: [[cc15:%[0-9]+]] = OpCompositeConstruct %v4float [[ce02]] [[ce03]] [[ce04]] [[ce05]]
// CHECK-NEXT: [[f_1:%[0-9]+]] = OpCompositeExtract %float [[f2_1_2]] 0
// CHECK-NEXT: [[f_2:%[0-9]+]] = OpCompositeExtract %float [[f2_1_2]] 1
// CHECK-NEXT: [[cc16:%[0-9]+]] = OpCompositeConstruct %v4float [[ce06]] [[f_1]] [[f_2]] %float_3
// CHECK-NEXT: [[cc17:%[0-9]+]] = OpCompositeConstruct %mat4v4float [[cc14]] [[cc15]] [[cc16]] [[vec4]]
// CHECK-NEXT:  OpStore %mat5 [[cc17]]
    float4x4 mat5 = {scalar, vec1, vec2,  // [0]
                     vec3, vec2,          // [1] + 1 scalar
                     float2(1., 2.), 3.,  // [2] - 1 scalar
                     vec4                 // [3]
    };

    // From value of the same type
// CHECK-NEXT: [[mat5:%[0-9]+]] = OpLoad %mat4v4float %mat5
// CHECK-NEXT: OpStore %mat6 [[mat5]]
    float4x4 mat6 = float4x4(mat5);

    int intScalar;
    uint uintScalar;
    bool boolScalar;
    int1 intVec1;
    uint2 uintVec2;
    bool3 boolVec3;

    // Casting
// CHECK-NEXT: [[intvec1:%[0-9]+]] = OpLoad %int %intVec1
// CHECK-NEXT: [[uintscalar:%[0-9]+]] = OpLoad %uint %uintScalar
// CHECK-NEXT: [[convert1:%[0-9]+]] = OpConvertUToF %float [[uintscalar]]
// CHECK-NEXT: [[uintvec2:%[0-9]+]] = OpLoad %v2uint %uintVec2
// CHECK-NEXT: [[intscalar:%[0-9]+]] = OpLoad %int %intScalar
// CHECK-NEXT: [[convert4:%[0-9]+]] = OpConvertSToF %float [[intscalar]]
// CHECK-NEXT: [[boolscalar:%[0-9]+]] = OpLoad %bool %boolScalar
// CHECK-NEXT: [[convert5:%[0-9]+]] = OpSelect %float [[boolscalar]] %float_1 %float_0
// CHECK-NEXT: [[boolvec3:%[0-9]+]] = OpLoad %v3bool %boolVec3
// CHECK-NEXT: [[convert0:%[0-9]+]] = OpConvertSToF %float [[intvec1]]
// CHECK-NEXT: [[ce07:%[0-9]+]] = OpCompositeExtract %uint [[uintvec2]] 0
// CHECK-NEXT: [[ce08:%[0-9]+]] = OpCompositeExtract %uint [[uintvec2]] 1
// CHECK-NEXT: [[convert2:%[0-9]+]] = OpConvertUToF %float [[ce07]]
// CHECK-NEXT: [[cc18:%[0-9]+]] = OpCompositeConstruct %v3float [[convert0]] [[convert1]] [[convert2]]

// CHECK-NEXT: [[convert3:%[0-9]+]] = OpConvertUToF %float [[ce08]]
// CHECK-NEXT: [[cc19:%[0-9]+]] = OpCompositeConstruct %v3float [[convert3]] [[convert4]] [[convert5]]

// CHECK-NEXT: [[convert6:%[0-9]+]] = OpSelect %v3float [[boolvec3]] [[v3fc1]] [[v3fc0]]
// CHECK-NEXT: [[cc20:%[0-9]+]] = OpCompositeConstruct %mat3v3float [[cc18]] [[cc19]] [[convert6]]

// CHECK-NEXT: OpStore %mat7 [[cc20]]
    float3x3 mat7 = {intVec1, uintScalar, uintVec2, // [0] + 1 scalar
                     intScalar, boolScalar,         // [1] - 1 scalar
                     boolVec3                       // [2]
    };

    // Decomposing matrices
    float2x2 mat8;
    float2x4 mat9;
    float4x1 mat10;
    // TODO: Optimization opportunity. We are extracting all elements in each
    // vector and then reconstructing the original vector. Optimally we should
    // extract vectors from matrices directly.

// CHECK-NEXT: [[mat8:%[0-9]+]] = OpLoad %mat2v2float %mat8
// CHECK-NEXT: [[mat9:%[0-9]+]] = OpLoad %mat2v4float %mat9
// CHECK-NEXT: [[mat10:%[0-9]+]] = OpLoad %v4float %mat10

// CHECK-NEXT: [[mat8_00:%[0-9]+]] = OpCompositeExtract %float [[mat8]] 0 0
// CHECK-NEXT: [[mat8_01:%[0-9]+]] = OpCompositeExtract %float [[mat8]] 0 1
// CHECK-NEXT: [[mat8_10:%[0-9]+]] = OpCompositeExtract %float [[mat8]] 1 0
// CHECK-NEXT: [[mat8_11:%[0-9]+]] = OpCompositeExtract %float [[mat8]] 1 1
// CHECK-NEXT: [[cc21:%[0-9]+]] = OpCompositeConstruct %v4float [[mat8_00]] [[mat8_01]] [[mat8_10]] [[mat8_11]]

// CHECK-NEXT: [[mat9_00:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 0 0
// CHECK-NEXT: [[mat9_01:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 0 1
// CHECK-NEXT: [[mat9_02:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 0 2
// CHECK-NEXT: [[mat9_03:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 0 3
// CHECK-NEXT: [[mat9_10:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 1 0
// CHECK-NEXT: [[mat9_11:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 1 1
// CHECK-NEXT: [[mat9_12:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 1 2
// CHECK-NEXT: [[mat9_13:%[0-9]+]] = OpCompositeExtract %float [[mat9]] 1 3
// CHECK-NEXT: [[cc22:%[0-9]+]] = OpCompositeConstruct %v4float [[mat9_00]] [[mat9_01]] [[mat9_02]] [[mat9_03]]
// CHECK-NEXT: [[cc23:%[0-9]+]] = OpCompositeConstruct %v4float [[mat9_10]] [[mat9_11]] [[mat9_12]] [[mat9_13]]

// CHECK-NEXT: [[cc25:%[0-9]+]] = OpCompositeConstruct %mat4v4float [[cc21]] [[cc22]] [[cc23]] [[mat10]]
// CHECK-NEXT: OpStore %mat11 [[cc25]]
    float4x4 mat11 = {mat8, mat9, mat10};


    // Non-floating point matrices


    // Constructor
// CHECK:      [[cc00_0:%[0-9]+]] = OpCompositeConstruct %v3int %int_1 %int_2 %int_3
// CHECK-NEXT: [[cc01_0:%[0-9]+]] = OpCompositeConstruct %v3int %int_4 %int_5 %int_6
// CHECK-NEXT: [[cc02_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[cc00_0]] [[cc01_0]]
// CHECK-NEXT: OpStore %imat1 [[cc02_0]]
    int2x3 imat1 = int2x3(1, 2, 3, 4, 5, 6);
    // All elements in a single {}
// CHECK-NEXT: [[cc03_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_1 %int_2
// CHECK-NEXT: [[cc04_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_3 %int_4
// CHECK-NEXT: [[cc05_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_5 %int_6
// CHECK-NEXT: [[cc06_0:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[cc03_0]] [[cc04_0]] [[cc05_0]]
// CHECK-NEXT: OpStore %imat2 [[cc06_0]]
    int3x2 imat2 = {1, 2, 3, 4, 5, 6};
    // Each vector has its own {}
// CHECK-NEXT: [[cc07_0:%[0-9]+]] = OpCompositeConstruct %v3int %int_1 %int_2 %int_3
// CHECK-NEXT: [[cc08_0:%[0-9]+]] = OpCompositeConstruct %v3int %int_4 %int_5 %int_6
// CHECK-NEXT: [[cc09_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[cc07_0]] [[cc08_0]]
// CHECK-NEXT: OpStore %imat3 [[cc09_0]]
    int2x3 imat3 = {{1, 2, 3}, {4, 5, 6}};
    // Wired & complicated {}s
// CHECK-NEXT: [[cc10_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_1 %int_2
// CHECK-NEXT: [[cc11_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_3 %int_4
// CHECK-NEXT: [[cc12_0:%[0-9]+]] = OpCompositeConstruct %v2int %int_5 %int_6
// CHECK-NEXT: [[cc13_0:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[cc10_0]] [[cc11_0]] [[cc12_0]]
// CHECK-NEXT: OpStore %imat4 [[cc13_0]]
    int3x2 imat4 = {{1}, {2, 3}, 4, {{5}, {{6}}}};

    int2 intVec2;
    int3 intVec3;
    int4 intVec4;

    // Mixed scalar and vector
// CHECK:         [[s_0:%[0-9]+]] = OpLoad %int %intScalar
// CHECK-NEXT: [[vec1_0:%[0-9]+]] = OpLoad %int %intVec1
// CHECK-NEXT: [[vec2_0:%[0-9]+]] = OpLoad %v2int %intVec2
// CHECK-NEXT: [[vec3_0:%[0-9]+]] = OpLoad %v3int %intVec3
// CHECK-NEXT:[[vec2a_0:%[0-9]+]] = OpLoad %v2int %intVec2
// CHECK-NEXT: [[vec4_0:%[0-9]+]] = OpLoad %v4int %intVec4

// CHECK-NEXT: [[ce00_0:%[0-9]+]] = OpCompositeExtract %int [[vec2_0]] 0
// CHECK-NEXT: [[ce01_0:%[0-9]+]] = OpCompositeExtract %int [[vec2_0]] 1
// CHECK-NEXT: [[cc14_0:%[0-9]+]] = OpCompositeConstruct %v4int [[s_0]] [[vec1_0]] [[ce00_0]] [[ce01_0]]

// CHECK-NEXT: [[ce02_0:%[0-9]+]] = OpCompositeExtract %int [[vec3_0]] 0
// CHECK-NEXT: [[ce03_0:%[0-9]+]] = OpCompositeExtract %int [[vec3_0]] 1
// CHECK-NEXT: [[ce04_0:%[0-9]+]] = OpCompositeExtract %int [[vec3_0]] 2
// CHECK-NEXT: [[ce05_0:%[0-9]+]] = OpCompositeExtract %int [[vec2a_0]] 0
// CHECK-NEXT: [[ce06_0:%[0-9]+]] = OpCompositeExtract %int [[vec2a_0]] 1
// CHECK-NEXT: [[cc15_0:%[0-9]+]] = OpCompositeConstruct %v4int [[ce02_0]] [[ce03_0]] [[ce04_0]] [[ce05_0]]
// CHECK-NEXT: [[i_1:%[0-9]+]] = OpCompositeExtract %int [[i2_1_2]] 0
// CHECK-NEXT: [[i_2:%[0-9]+]] = OpCompositeExtract %int [[i2_1_2]] 1
// CHECK-NEXT: [[cc16_0:%[0-9]+]] = OpCompositeConstruct %v4int [[ce06_0]] [[i_1]] [[i_2]] %int_3
// CHECK-NEXT: [[cc17_0:%[0-9]+]] = OpCompositeConstruct %_arr_v4int_uint_4 [[cc14_0]] [[cc15_0]] [[cc16_0]] [[vec4_0]]
// CHECK-NEXT:  OpStore %imat5 [[cc17_0]]
    int4x4 imat5 = {intScalar, intVec1, intVec2, // [0]
                    intVec3,   intVec2,          // [1] + 1 scalar
                     int2(1, 2), 3,              // [2] - 1 scalar
                     intVec4                     // [3]
    };

    // From value of the same type
// CHECK-NEXT: [[imat5:%[0-9]+]] = OpLoad %_arr_v4int_uint_4 %imat5
// CHECK-NEXT:                  OpStore %imat6 [[imat5]]
    int4x4 imat6 = int4x4(imat5);

    // Casting
    float floatScalar;
// CHECK:                      [[intVec1:%[0-9]+]] = OpLoad %int %intVec1
// CHECK-NEXT:              [[uintScalar:%[0-9]+]] = OpLoad %uint %uintScalar
// CHECK-NEXT:               [[intScalar:%[0-9]+]] = OpBitcast %int [[uintScalar]]
// CHECK-NEXT:                [[uintVec2:%[0-9]+]] = OpLoad %v2uint %uintVec2
// CHECK-NEXT:             [[floatScalar:%[0-9]+]] = OpLoad %float %floatScalar
// CHECK-NEXT: [[convert_floatScalar_int:%[0-9]+]] = OpConvertFToS %int [[floatScalar]]
// CHECK-NEXT:              [[boolScalar:%[0-9]+]] = OpLoad %bool %boolScalar
// CHECK-NEXT:  [[convert_boolScalar_int:%[0-9]+]] = OpSelect %int [[boolScalar]] %int_1 %int_0
// CHECK-NEXT:                  [[v3bool:%[0-9]+]] = OpLoad %v3bool %boolVec3
// CHECK-NEXT:              [[uintVec2e0:%[0-9]+]] = OpCompositeExtract %uint [[uintVec2]] 0
// CHECK-NEXT:              [[uintVec2e1:%[0-9]+]] = OpCompositeExtract %uint [[uintVec2]] 1
// CHECK-NEXT:  [[convert_uintVec2e0_int:%[0-9]+]] = OpBitcast %int [[uintVec2e0]]
// CHECK-NEXT:                [[imat7_r0:%[0-9]+]] = OpCompositeConstruct %v3int [[intVec1]] [[intScalar]] [[convert_uintVec2e0_int]]
// CHECK-NEXT:  [[convert_uintVec2e1_int:%[0-9]+]] = OpBitcast %int [[uintVec2e1]]
// CHECK-NEXT:                [[imat7_r1:%[0-9]+]] = OpCompositeConstruct %v3int [[convert_uintVec2e1_int]] [[convert_floatScalar_int]] [[convert_boolScalar_int]]
// CHECK-NEXT:                [[imat7_r2:%[0-9]+]] = OpSelect %v3int [[v3bool]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                         {{%[0-9]+}} = OpCompositeConstruct %_arr_v3int_uint_3 [[imat7_r0]] [[imat7_r1]] [[imat7_r2]]
    int3x3 imat7 = {intVec1, uintScalar, uintVec2, // [0] + 1 scalar
                    floatScalar, boolScalar,       // [1] - 1 scalar
                    boolVec3                       // [2]
    };

    // Decomposing matrices
    int2x2 imat8;
    int2x4 imat9;
    int4x1 imat10;
    // TODO: Optimization opportunity. We are extracting all elements in each
    // vector and then reconstructing the original vector. Optimally we should
    // extract vectors from matrices directly.

// CHECK:         [[imat8:%[0-9]+]] = OpLoad %_arr_v2int_uint_2 %imat8
// CHECK-NEXT:    [[imat9:%[0-9]+]] = OpLoad %_arr_v4int_uint_2 %imat9
// CHECK-NEXT:   [[imat10:%[0-9]+]] = OpLoad %v4int %imat10

// CHECK-NEXT: [[imat8_00:%[0-9]+]] = OpCompositeExtract %int [[imat8]] 0 0
// CHECK-NEXT: [[imat8_01:%[0-9]+]] = OpCompositeExtract %int [[imat8]] 0 1
// CHECK-NEXT: [[imat8_10:%[0-9]+]] = OpCompositeExtract %int [[imat8]] 1 0
// CHECK-NEXT: [[imat8_11:%[0-9]+]] = OpCompositeExtract %int [[imat8]] 1 1
// CHECK-NEXT:     [[cc21_0:%[0-9]+]] = OpCompositeConstruct %v4int [[imat8_00]] [[imat8_01]] [[imat8_10]] [[imat8_11]]

// CHECK-NEXT: [[imat9_00:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 0 0
// CHECK-NEXT: [[imat9_01:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 0 1
// CHECK-NEXT: [[imat9_02:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 0 2
// CHECK-NEXT: [[imat9_03:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 0 3
// CHECK-NEXT: [[imat9_10:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 1 0
// CHECK-NEXT: [[imat9_11:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 1 1
// CHECK-NEXT: [[imat9_12:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 1 2
// CHECK-NEXT: [[imat9_13:%[0-9]+]] = OpCompositeExtract %int [[imat9]] 1 3
// CHECK-NEXT:     [[cc22_0:%[0-9]+]] = OpCompositeConstruct %v4int [[imat9_00]] [[imat9_01]] [[imat9_02]] [[imat9_03]]
// CHECK-NEXT:     [[cc23_0:%[0-9]+]] = OpCompositeConstruct %v4int [[imat9_10]] [[imat9_11]] [[imat9_12]] [[imat9_13]]

// CHECK-NEXT: [[cc25_0:%[0-9]+]] = OpCompositeConstruct %_arr_v4int_uint_4 [[cc21_0]] [[cc22_0]] [[cc23_0]] [[imat10]]
// CHECK-NEXT: OpStore %imat11 [[cc25_0]]
    int4x4 imat11 = {imat8, imat9, imat10};

    // Boolean matrices
// CHECK:      [[cc00_1:%[0-9]+]] = OpCompositeConstruct %v3bool %false %true %false
// CHECK-NEXT: [[cc01_1:%[0-9]+]] = OpCompositeConstruct %v3bool %true %true %false
// CHECK-NEXT: [[cc02_1:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[cc00_1]] [[cc01_1]]
// CHECK-NEXT:                 OpStore %bmat1 [[cc02_1]]
    bool2x3 bmat1 = bool2x3(false, true, false, true, true, false);
    // All elements in a single {}
// CHECK-NEXT: [[cc03_1:%[0-9]+]] = OpCompositeConstruct %v2bool %false %true
// CHECK-NEXT: [[cc04_1:%[0-9]+]] = OpCompositeConstruct %v2bool %false %true
// CHECK-NEXT: [[cc05_1:%[0-9]+]] = OpCompositeConstruct %v2bool %true %false
// CHECK-NEXT: [[cc06_1:%[0-9]+]] = OpCompositeConstruct %_arr_v2bool_uint_3 [[cc03_1]] [[cc04_1]] [[cc05_1]]
// CHECK-NEXT:                 OpStore %bmat2 [[cc06_1]]
    bool3x2 bmat2 = {false, true, false, true, true, false};
    // Each vector has its own {}
// CHECK-NEXT: [[cc07_1:%[0-9]+]] = OpCompositeConstruct %v3bool %false %true %false
// CHECK-NEXT: [[cc08_1:%[0-9]+]] = OpCompositeConstruct %v3bool %true %true %false
// CHECK-NEXT: [[cc09_1:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[cc07_1]] [[cc08_1]]
// CHECK-NEXT:                 OpStore %bmat3 [[cc09_1]]
    bool2x3 bmat3 = {{false, true, false}, {true, true, false}};
    // Wired & complicated {}s
// CHECK-NEXT: [[cc10_1:%[0-9]+]] = OpCompositeConstruct %v2bool %false %true
// CHECK-NEXT: [[cc11_1:%[0-9]+]] = OpCompositeConstruct %v2bool %false %true
// CHECK-NEXT: [[cc12_1:%[0-9]+]] = OpCompositeConstruct %v2bool %true %false
// CHECK-NEXT: [[cc13_1:%[0-9]+]] = OpCompositeConstruct %_arr_v2bool_uint_3 [[cc10_1]] [[cc11_1]] [[cc12_1]]
// CHECK-NEXT:                 OpStore %bmat4 [[cc13_1]]
    bool3x2 bmat4 = {{false}, {true, false}, true, {{true}, {{false}}}};
}
