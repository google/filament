// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

//CHECK: [[name_2d_arr:%[0-9]+]] = OpString "@type.2d.image.array"
//CHECK: [[name_1d_arr:%[0-9]+]] = OpString "@type.1d.image.array"
//CHECK: [[name_3d:%[0-9]+]] = OpString "@type.3d.image"
//CHECK: [[name_1d:%[0-9]+]] = OpString "@type.1d.image"

//CHECK: [[re_2daf_comp:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugTypeComposite [[name_2d_arr]] Class [[src:%[0-9]+]] 0 0 [[cu:%[0-9]+]] {{%[0-9]+}} [[info_none:%[0-9]+]]
//CHECK: [[tem_p6:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[re_2daf:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[re_2daf_comp]] [[tem_p6]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[re_2daf]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t8

//CHECK: [[re_1dai_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_1d_arr]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p3:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[re_1dai:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[re_1dai_comp]] [[tem_p3]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[re_1dai]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t5

//CHECK: [[re_3df_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_3d]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p2:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[re_3df:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[re_3df_comp]] [[tem_p2]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[re_3df]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t3

//CHECK: [[re_1di_comp:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name_1d]] Class [[src]] 0 0 [[cu]] {{%[0-9]+}} [[info_none]]
//CHECK: [[tem_p0:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter
//CHECK: [[re_1di:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplate [[re_1di_comp]] [[tem_p0]]
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[re_1di]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %t1

RWTexture1D   <int>    t1 ;
RWTexture3D   <float3> t3 ;
RWTexture1DArray<int>    t5;
RWTexture2DArray<float4> t8;

void main() {}
