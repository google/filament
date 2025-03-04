// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Signature: ret asuint(arg)
//    arg component type = {float, int}
//    arg template  type = {scalar, vector, matrix}
//    ret template  type = same as arg template type.
//    ret component type = uint

// Signature:
//           void asuint(
//           in  double value,
//           out uint lowbits,
//           out uint highbits
//           )

void main() {
    uint result;
    uint4 result4;

    // CHECK:      [[b:%[0-9]+]] = OpLoad %int %b
    // CHECK-NEXT: [[b_as_uint:%[0-9]+]] = OpBitcast %uint [[b]]
    // CHECK-NEXT: OpStore %result [[b_as_uint]]
    int b;
    result = asuint(b);

    // CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %float %c
    // CHECK-NEXT: [[c_as_uint:%[0-9]+]] = OpBitcast %uint [[c]]
    // CHECK-NEXT: OpStore %result [[c_as_uint]]
    float c;
    result = asuint(c);

    // CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %int %e
    // CHECK-NEXT: [[e_as_uint:%[0-9]+]] = OpBitcast %uint [[e]]
    // CHECK-NEXT: OpStore %result [[e_as_uint]]
    int1 e;
    result = asuint(e);

    // CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %float %f
    // CHECK-NEXT: [[f_as_uint:%[0-9]+]] = OpBitcast %uint [[f]]
    // CHECK-NEXT: OpStore %result [[f_as_uint]]
    float1 f;
    result = asuint(f);

    // CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %v4int %h
    // CHECK-NEXT: [[h_as_uint:%[0-9]+]] = OpBitcast %v4uint [[h]]
    // CHECK-NEXT: OpStore %result4 [[h_as_uint]]
    int4 h;
    result4 = asuint(h);

    // CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v4float %i
    // CHECK-NEXT: [[i_as_uint:%[0-9]+]] = OpBitcast %v4uint [[i]]
    // CHECK-NEXT: OpStore %result4 [[i_as_uint]]
    float4 i;
    result4 = asuint(i);

    float2x3 floatMat;
    int2x3 intMat;
    
// CHECK:       [[floatMat:%[0-9]+]] = OpLoad %mat2v3float %floatMat
// CHECK-NEXT: [[floatMat0:%[0-9]+]] = OpCompositeExtract %v3float [[floatMat]] 0
// CHECK-NEXT:      [[row0:%[0-9]+]] = OpBitcast %v3uint [[floatMat0]]
// CHECK-NEXT: [[floatMat1:%[0-9]+]] = OpCompositeExtract %v3float [[floatMat]] 1
// CHECK-NEXT:      [[row1:%[0-9]+]] = OpBitcast %v3uint [[floatMat1]]
// CHECK-NEXT:         [[j:%[0-9]+]] = OpCompositeConstruct %_arr_v3uint_uint_2 [[row0]] [[row1]]
// CHECK-NEXT:                      OpStore %j [[j]]
    uint2x3 j = asuint(floatMat);
// CHECK:       [[intMat:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %intMat
// CHECK-NEXT: [[intMat0:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 0
// CHECK-NEXT:    [[row0_0:%[0-9]+]] = OpBitcast %v3uint [[intMat0]]
// CHECK-NEXT: [[intMat1:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 1
// CHECK-NEXT:    [[row1_0:%[0-9]+]] = OpBitcast %v3uint [[intMat1]]
// CHECK-NEXT:       [[k:%[0-9]+]] = OpCompositeConstruct %_arr_v3uint_uint_2 [[row0_0]] [[row1_0]]
// CHECK-NEXT:                    OpStore %k [[k]]
    uint2x3 k = asuint(intMat);

    double value;
    uint lowbits;
    uint highbits;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %double %value
// CHECK-NEXT:  [[resultVec:%[0-9]+]] = OpBitcast %v2uint [[value]]
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec]] 0
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec]] 1
// CHECK-NEXT:                       OpStore %lowbits [[resultVec0]]
// CHECK-NEXT:                       OpStore %highbits [[resultVec1]]
    asuint(value, lowbits, highbits);

    double3 value3;
    uint3 lowbits3;
    uint3 highbits3;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %v3double %value3
// CHECK-NEXT:     [[value0:%[0-9]+]] = OpCompositeExtract %double [[value]] 0
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpBitcast %v2uint [[value0]]
// CHECK-NEXT:       [[low0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 0
// CHECK-NEXT:      [[high0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 1
// CHECK-NEXT:     [[value1:%[0-9]+]] = OpCompositeExtract %double [[value]] 1
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpBitcast %v2uint [[value1]]
// CHECK-NEXT:       [[low1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 0
// CHECK-NEXT:      [[high1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 1
// CHECK-NEXT:     [[value2:%[0-9]+]] = OpCompositeExtract %double [[value]] 2
// CHECK-NEXT: [[resultVec2:%[0-9]+]] = OpBitcast %v2uint [[value2]]
// CHECK-NEXT:       [[low2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 0
// CHECK-NEXT:      [[high2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 1
// CHECK-NEXT:        [[low:%[0-9]+]] = OpCompositeConstruct %v3uint [[low0]] [[low1]] [[low2]]
// CHECK-NEXT:       [[high:%[0-9]+]] = OpCompositeConstruct %v3uint [[high0]] [[high1]] [[high2]]
// CHECK-NEXT:                          OpStore %lowbits3 [[low]]
// CHECK-NEXT:                          OpStore %highbits3 [[high]]
    asuint(value3, lowbits3, highbits3);

    double2x2 value2x2;
    uint2x2 lowbits2x2;
    uint2x2 highbits2x2;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %mat2v2double %value2x2
// CHECK-NEXT:       [[row0:%[0-9]+]] = OpCompositeExtract %v2double [[value]] 0
// CHECK-NEXT:     [[value0:%[0-9]+]] = OpCompositeExtract %double [[row0]] 0
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpBitcast %v2uint [[value0]]
// CHECK-NEXT:       [[low0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 0
// CHECK-NEXT:      [[high0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 1
// CHECK-NEXT:     [[value1:%[0-9]+]] = OpCompositeExtract %double [[row0]] 1
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpBitcast %v2uint [[value1]]
// CHECK-NEXT:       [[low1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 0
// CHECK-NEXT:      [[high1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 1
// CHECK-NEXT:    [[lowRow0:%[0-9]+]] = OpCompositeConstruct %v2uint [[low0]] [[low1]]
// CHECK-NEXT:   [[highRow0:%[0-9]+]] = OpCompositeConstruct %v2uint [[high0]] [[high1]]
// CHECK-NEXT:       [[row1:%[0-9]+]] = OpCompositeExtract %v2double [[value]] 1
// CHECK-NEXT:     [[value2:%[0-9]+]] = OpCompositeExtract %double [[row1]] 0
// CHECK-NEXT: [[resultVec2:%[0-9]+]] = OpBitcast %v2uint [[value2]]
// CHECK-NEXT:       [[low2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 0
// CHECK-NEXT:      [[high2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 1
// CHECK-NEXT:     [[value3:%[0-9]+]] = OpCompositeExtract %double [[row1]] 1
// CHECK-NEXT: [[resultVec3:%[0-9]+]] = OpBitcast %v2uint [[value3]]
// CHECK-NEXT:       [[low3:%[0-9]+]] = OpCompositeExtract %uint [[resultVec3]] 0
// CHECK-NEXT:      [[high3:%[0-9]+]] = OpCompositeExtract %uint [[resultVec3]] 1
// CHECK-NEXT:    [[lowRow1:%[0-9]+]] = OpCompositeConstruct %v2uint [[low2]] [[low3]]
// CHECK-NEXT:   [[highRow1:%[0-9]+]] = OpCompositeConstruct %v2uint [[high2]] [[high3]]
// CHECK-NEXT:        [[low:%[0-9]+]] = OpCompositeConstruct %_arr_v2uint_uint_2 [[lowRow0]] [[lowRow1]]
// CHECK-NEXT:       [[high:%[0-9]+]] = OpCompositeConstruct %_arr_v2uint_uint_2 [[highRow0]] [[highRow1]]
// CHECK-NEXT:                          OpStore %lowbits2x2 [[low]]
// CHECK-NEXT:                          OpStore %highbits2x2 [[high]]
    asuint(value2x2, lowbits2x2, highbits2x2);

    double3x2 value3x2;
    uint3x2 lowbits3x2;
    uint3x2 highbits3x2;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %mat3v2double %value3x2
// CHECK-NEXT:       [[row0:%[0-9]+]] = OpCompositeExtract %v2double [[value]] 0
// CHECK-NEXT:     [[value0:%[0-9]+]] = OpCompositeExtract %double [[row0]] 0
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpBitcast %v2uint [[value0]]
// CHECK-NEXT:       [[low0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 0
// CHECK-NEXT:      [[high0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 1
// CHECK-NEXT:     [[value1:%[0-9]+]] = OpCompositeExtract %double [[row0]] 1
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpBitcast %v2uint [[value1]]
// CHECK-NEXT:       [[low1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 0
// CHECK-NEXT:      [[high1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 1
// CHECK-NEXT:    [[lowRow0:%[0-9]+]] = OpCompositeConstruct %v2uint [[low0]] [[low1]]
// CHECK-NEXT:   [[highRow0:%[0-9]+]] = OpCompositeConstruct %v2uint [[high0]] [[high1]]
// CHECK-NEXT:       [[row1:%[0-9]+]] = OpCompositeExtract %v2double [[value]] 1
// CHECK-NEXT:     [[value2:%[0-9]+]] = OpCompositeExtract %double [[row1]] 0
// CHECK-NEXT: [[resultVec2:%[0-9]+]] = OpBitcast %v2uint [[value2]]
// CHECK-NEXT:       [[low2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 0
// CHECK-NEXT:      [[high2:%[0-9]+]] = OpCompositeExtract %uint [[resultVec2]] 1
// CHECK-NEXT:     [[value3:%[0-9]+]] = OpCompositeExtract %double [[row1]] 1
// CHECK-NEXT: [[resultVec3:%[0-9]+]] = OpBitcast %v2uint [[value3]]
// CHECK-NEXT:       [[low3:%[0-9]+]] = OpCompositeExtract %uint [[resultVec3]] 0
// CHECK-NEXT:      [[high3:%[0-9]+]] = OpCompositeExtract %uint [[resultVec3]] 1
// CHECK-NEXT:    [[lowRow1:%[0-9]+]] = OpCompositeConstruct %v2uint [[low2]] [[low3]]
// CHECK-NEXT:   [[highRow1:%[0-9]+]] = OpCompositeConstruct %v2uint [[high2]] [[high3]]
// CHECK-NEXT:       [[row2:%[0-9]+]] = OpCompositeExtract %v2double [[value]] 2
// CHECK-NEXT:     [[value4:%[0-9]+]] = OpCompositeExtract %double [[row2]] 0
// CHECK-NEXT: [[resultVec4:%[0-9]+]] = OpBitcast %v2uint [[value4]]
// CHECK-NEXT:       [[low4:%[0-9]+]] = OpCompositeExtract %uint [[resultVec4]] 0
// CHECK-NEXT:      [[high4:%[0-9]+]] = OpCompositeExtract %uint [[resultVec4]] 1
// CHECK-NEXT:     [[value5:%[0-9]+]] = OpCompositeExtract %double [[row2]] 1
// CHECK-NEXT: [[resultVec5:%[0-9]+]] = OpBitcast %v2uint [[value5]]
// CHECK-NEXT:       [[low5:%[0-9]+]] = OpCompositeExtract %uint [[resultVec5]] 0
// CHECK-NEXT:      [[high5:%[0-9]+]] = OpCompositeExtract %uint [[resultVec5]] 1
// CHECK-NEXT:    [[lowRow2:%[0-9]+]] = OpCompositeConstruct %v2uint [[low4]] [[low5]]
// CHECK-NEXT:   [[highRow2:%[0-9]+]] = OpCompositeConstruct %v2uint [[high4]] [[high5]]
// CHECK-NEXT:        [[low:%[0-9]+]] = OpCompositeConstruct %_arr_v2uint_uint_3 [[lowRow0]] [[lowRow1]] [[lowRow2]]
// CHECK-NEXT:       [[high:%[0-9]+]] = OpCompositeConstruct %_arr_v2uint_uint_3 [[highRow0]] [[highRow1]] [[highRow2]]
// CHECK-NEXT:                          OpStore %lowbits3x2 [[low]]
// CHECK-NEXT:                          OpStore %highbits3x2 [[high]]
    asuint(value3x2, lowbits3x2, highbits3x2);

    double2x1 value2x1;
    uint2x1 lowbits2x1;
    uint2x1 highbits2x1;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %v2double %value2x1
// CHECK-NEXT:     [[value0:%[0-9]+]] = OpCompositeExtract %double [[value]] 0
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpBitcast %v2uint [[value0]]
// CHECK-NEXT:       [[low0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 0
// CHECK-NEXT:      [[high0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 1
// CHECK-NEXT:     [[value1:%[0-9]+]] = OpCompositeExtract %double [[value]] 1
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpBitcast %v2uint [[value1]]
// CHECK-NEXT:       [[low1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 0
// CHECK-NEXT:      [[high1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 1
// CHECK-NEXT:        [[low:%[0-9]+]] = OpCompositeConstruct %v2uint [[low0]] [[low1]]
// CHECK-NEXT:       [[high:%[0-9]+]] = OpCompositeConstruct %v2uint [[high0]] [[high1]]
// CHECK-NEXT:                          OpStore %lowbits2x1 [[low]]
// CHECK-NEXT:                          OpStore %highbits2x1 [[high]]
    asuint(value2x1, lowbits2x1, highbits2x1);

    double1x2 value1x2;
    uint1x2 lowbits1x2;
    uint1x2 highbits1x2;
// CHECK-NEXT:      [[value:%[0-9]+]] = OpLoad %v2double %value1x2
// CHECK-NEXT:     [[value0:%[0-9]+]] = OpCompositeExtract %double [[value]] 0
// CHECK-NEXT: [[resultVec0:%[0-9]+]] = OpBitcast %v2uint [[value0]]
// CHECK-NEXT:       [[low0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 0
// CHECK-NEXT:      [[high0:%[0-9]+]] = OpCompositeExtract %uint [[resultVec0]] 1
// CHECK-NEXT:     [[value1:%[0-9]+]] = OpCompositeExtract %double [[value]] 1
// CHECK-NEXT: [[resultVec1:%[0-9]+]] = OpBitcast %v2uint [[value1]]
// CHECK-NEXT:       [[low1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 0
// CHECK-NEXT:      [[high1:%[0-9]+]] = OpCompositeExtract %uint [[resultVec1]] 1
// CHECK-NEXT:        [[low:%[0-9]+]] = OpCompositeConstruct %v2uint [[low0]] [[low1]]
// CHECK-NEXT:       [[high:%[0-9]+]] = OpCompositeConstruct %v2uint [[high0]] [[high1]]
// CHECK-NEXT:                          OpStore %lowbits1x2 [[low]]
// CHECK-NEXT:                          OpStore %highbits1x2 [[high]]
    asuint(value1x2, lowbits1x2, highbits1x2);
}
