// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                         OpDecorate %gl_SampleMask BuiltIn SampleMask
// CHECK:                         OpDecorate %gl_SampleMask_0 BuiltIn SampleMask

// CHECK:        %gl_SampleMask = OpVariable %_ptr_Input__arr_uint_uint_1 Input
// CHECK:      %gl_SampleMask_0 = OpVariable %_ptr_Output__arr_uint_uint_1 Output

// CHECK:      %param_var_inCov = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:     [[val:%[0-9]+]] = OpLoad %_arr_uint_uint_1 %gl_SampleMask
// CHECK-NEXT: [[element:%[0-9]+]] = OpCompositeExtract %uint [[val]] 0
// CHECK-NEXT:                    OpStore %param_var_inCov [[element]]
// CHECK-NEXT:     [[ret:%[0-9]+]] = OpFunctionCall %uint %src_main %param_var_inCov
// CHECK-NEXT:     [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_uint %gl_SampleMask_0 %uint_0
// CHECK-NEXT:                    OpStore [[ptr]] [[ret]]

uint main(uint inCov : SV_Coverage) : SV_Coverage {
    return inCov;
}
