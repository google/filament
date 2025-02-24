// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                      OpDecorate %testShared RelaxedPrecision
// CHECK-NEXT:                 OpDecorate %test RelaxedPrecision
// CHECK-NEXT:                 OpDecorate %testTypedef RelaxedPrecision
// CHECK-NOT:                  OpDecorate %notMin16 RelaxedPrecision
// CHECK-NEXT:                 OpDecorate %a RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[compositeConstr1:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[compositeConstr2:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadFromTest1:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadFromTest2:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadFromTestTypedef:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[multiply1:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadVariable1:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[multiply2:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadFromShared:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[loadVariable2:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:                 OpDecorate [[multiply3:%[0-9]+]] RelaxedPrecision

typedef min16float ArrayOfMin16Float0[2];
typedef ArrayOfMin16Float0 ArrayOfMin16Float;

groupshared min16float testShared[2];

[numthreads(1, 1, 1)] void main()
{
// CHECK: [[compositeConstr2_0:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_2 {{%[0-9]+}} {{%[0-9]+}}
    min16float2 test[2] = { float2(1.0, 1.0), float2(1.0, 1.0) };
// CHECK: [[compositeConstr1_0:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_2 %float_1 %float_1
    ArrayOfMin16Float testTypedef = { 1.0, 1.0 };
    float notMin16[2] = { 1.0, 1.0 };
// CHECK: [[loadFromTest1_0:%[0-9]+]] = OpLoad %float {{%[0-9]+}}
    testShared[0] = test[0].x;

    min16float a = 1.0;
// CHECK: [[loadFromTest2_0:%[0-9]+]] = OpLoad %float {{%[0-9]+}}
// CHECK: [[loadFromTestTypedef_0:%[0-9]+]] = OpLoad %float {{%[0-9]+}}
// CHECK: [[multiply1_0:%[0-9]+]] = OpFMul %float {{%[0-9]+}} {{%[0-9]+}}
// CHECK: [[loadVariable1_0:%[0-9]+]] = OpLoad %float %a
// CHECK: [[multiply2_0:%[0-9]+]] = OpFMul %float {{%[0-9]+}} {{%[0-9]+}}
    a *= (test[0].x * testTypedef[0]);
// CHECK: [[loadFromShared_0:%[0-9]+]] = OpLoad %float {{%[0-9]+}}
// CHECK: [[loadVariable2_0:%[0-9]+]] = OpLoad %float %a
// CHECK: [[multiply3_0:%[0-9]+]] = OpFMul %float {{%[0-9]+}} {{%[0-9]+}}
    a *= testShared[0];
}
