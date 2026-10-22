// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Fragment %main "main" %gl_SampleID
// CHECK: OpDecorate %gl_SampleID BuiltIn SampleId
// CHECK: OpDecorate %gl_SampleID Flat

// CHECK: %gl_SampleID = OpVariable %_ptr_Input_int Input

[[vk::ext_builtin_input(/* SampleID */ 18)]]
static const int gl_SampleID;

void main() {
  // CHECK: {{%[0-9]+}} = OpLoad %int %gl_SampleID
  int sID = gl_SampleID;
}
