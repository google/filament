// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Fragment %main "main" %in_var_A %gl_FragDepth

// CHECK: OpExecutionMode %main DepthReplacing
// CHECK: OpExecutionMode %main DepthGreater

// CHECK: OpDecorate %gl_FragDepth BuiltIn FragDepth

// CHECK: %gl_FragDepth = OpVariable %_ptr_Output_float Output

float main(float input: A) : SV_DepthGreaterEqual {
    return input;
}
