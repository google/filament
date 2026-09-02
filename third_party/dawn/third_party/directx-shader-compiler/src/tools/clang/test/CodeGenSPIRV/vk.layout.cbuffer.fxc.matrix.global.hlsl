// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[arr_f2:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 0 Offset 0
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 Offset 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 2 Offset 36

// CHECK: [[arr_f2]] = OpTypeArray %float %uint_2
// CHECK: %type__Globals = OpTypeStruct %float [[arr_f2]] %float
// CHECK: %_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals

// CHECK: [[Globals_clone:%[a-zA-Z0-9_]+]] = OpTypeStruct %float %v2float %float
// CHECK: [[ptr_Globals_clone:%[a-zA-Z0-9_]+]] = OpTypePointer Private [[Globals_clone]]

// CHECK: %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
// CHECK:             OpVariable [[ptr_Globals_clone]] Private

float dummy0;                      // Offset:    0 Size:     4 [unused]
float1x2 foo;                      // Offset:   16 Size:    20 [unused]
float end;                         // Offset:   36 Size:     4

float4 main(float4 color : COLOR) : SV_TARGET
{
  color.x += end;
  return color;
}
