// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

//CHECK: [[name_2d_arr:%[0-9]+]] = OpString "@type.2d.image.array"
//CHECK: [[name_2d:%[0-9]+]] = OpString "@type.2d.image"
//CHECK: [[name_cb_arr:%[0-9]+]] = OpString "@type.cube.image.array"

//CHECK: [[info_none:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugInfoNone
//CHECK: [[t3_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_2d_arr]] Class [[src:%[0-9]+]] 0 0 [[cu:%[0-9]+]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p3:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[t3_ty:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[t3_comp]] [[tem_p3]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[t3_ty]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t3

//CHECK: [[t2_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_2d]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p2:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[t2_ty:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[t2_comp]] [[tem_p2]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[t2_ty]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t2

//CHECK: [[t1_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_cb_arr]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p1:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[t1_ty:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[t1_comp]] [[tem_p1]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[t1_ty]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t1

//CHECK: [[t0_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_2d]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p0:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[t0_ty:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[t0_comp]] [[tem_p0]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[t0_ty]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t0

Texture2D   <int4>   t0 : register(t0);
TextureCubeArray <float4> t1 : register(t1);
Texture2DMS      <int3>   t2 : register(t2);

Texture2DArray<float2> t3;

void main() {
}
