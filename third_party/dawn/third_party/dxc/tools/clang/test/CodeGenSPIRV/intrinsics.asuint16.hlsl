// RUN: %dxc -T vs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Int16
// CHECK: OpCapability Float16

void main() {
    uint16_t result;
    uint16_t4 result4;

    // CHECK: [[a:%[0-9]+]] = OpLoad %short %a
    // CHECK-NEXT: [[a_as_ushort:%[0-9]+]] = OpBitcast %ushort [[a]]
    // CHECK-NEXT: OpStore %result [[a_as_ushort]]
    int16_t a;
    result = asuint16(a);

    // CHECK: [[b:%[0-9]+]] = OpLoad %ushort %b
    // CHECK-NEXT: OpStore %result [[b]]
    uint16_t b;
    result = asuint16(b);

    // CHECK: [[c:%[0-9]+]] = OpLoad %half %c
    // CHECK-NEXT: [[c_as_ushort:%[0-9]+]] = OpBitcast %ushort [[c]]
    // CHECK-NEXT: OpStore %result [[c_as_ushort]]
    float16_t c;
    result = asuint16(c);

    // CHECK: [[d:%[0-9]+]] = OpLoad %v4short %d
    // CHECK-NEXT: [[d_as_ushort:%[0-9]+]] = OpBitcast %v4ushort [[d]]
    // CHECK-NEXT: OpStore %result4 [[d_as_ushort]]
    int16_t4 d;
    result4 = asuint16(d);

    // CHECK: [[e:%[0-9]+]] = OpLoad %v4ushort %e
    // CHECK-NEXT: OpStore %result4 [[e]]
    uint16_t4 e;
    result4 = asuint16(e);

    // CHECK: [[f:%[0-9]+]] = OpLoad %v4half %f
    // CHECK-NEXT: [[f_as_ushort:%[0-9]+]] = OpBitcast %v4ushort [[f]]
    // CHECK-NEXT: OpStore %result4 [[f_as_ushort]]
    float16_t4 f;
    result4 = asuint16(f);

    float16_t2x3 floatMat;
    int16_t2x3 intMat;

    // CHECK:       [[floatMat:%[0-9]+]] = OpLoad %mat2v3half %floatMat
    // CHECK-NEXT: [[floatMat0:%[0-9]+]] = OpCompositeExtract %v3half [[floatMat]] 0
    // CHECK-NEXT:      [[row0:%[0-9]+]] = OpBitcast %v3ushort [[floatMat0]]
    // CHECK-NEXT: [[floatMat1:%[0-9]+]] = OpCompositeExtract %v3half [[floatMat]] 1
    // CHECK-NEXT:      [[row1:%[0-9]+]] = OpBitcast %v3ushort [[floatMat1]]
    // CHECK-NEXT:         [[g:%[0-9]+]] = OpCompositeConstruct %_arr_v3ushort_uint_2 [[row0]] [[row1]]
    // CHECK-NEXT:                      OpStore %g [[g]]
    uint16_t2x3 g = asuint16(floatMat);

    // CHECK:       [[intMat:%[0-9]+]] = OpLoad %_arr_v3short_uint_2 %intMat
    // CHECK-NEXT: [[intMat0:%[0-9]+]] = OpCompositeExtract %v3short [[intMat]] 0
    // CHECK-NEXT:      [[row0_0:%[0-9]+]] = OpBitcast %v3ushort [[intMat0]]
    // CHECK-NEXT: [[intMat1:%[0-9]+]] = OpCompositeExtract %v3short [[intMat]] 1
    // CHECK-NEXT:      [[row1_0:%[0-9]+]] = OpBitcast %v3ushort [[intMat1]]
    // CHECK-NEXT:         [[h:%[0-9]+]] = OpCompositeConstruct %_arr_v3ushort_uint_2 [[row0_0]] [[row1_0]]
    // CHECK-NEXT:                      OpStore %h [[h]]
    uint16_t2x3 h = asuint16(intMat);
}
