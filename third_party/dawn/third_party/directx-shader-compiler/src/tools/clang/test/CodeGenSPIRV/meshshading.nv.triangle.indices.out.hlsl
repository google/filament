// RUN: %dxc -T ms_6_5 -E outie -fcgl  %s -spirv | FileCheck %s
// RUN: %dxc -T ms_6_5 -E innie -fcgl  %s -spirv | FileCheck %s

// CHECK-DAG: [[v4_n05_05_0_1:%[0-9]+]] = OpConstantComposite %v4float %float_n0_5 %float_0_5 %float_0 %float_1
// CHECK-DAG:  [[v4_05_05_0_1:%[0-9]+]] = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0 %float_1
// CHECK-DAG:  [[v4_0_n05_0_1:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_n0_5 %float_0 %float_1
// CHECK-DAG:  [[v3_1_0_0:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_0 %float_0
// CHECK-DAG:  [[v3_0_1_0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_1 %float_0
// CHECK-DAG:  [[v3_0_0_1:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_1
// CHECK-DAG:  [[u3_0_1_2:%[0-9]+]] = OpConstantComposite %v3uint %uint_0 %uint_1 %uint_2

// CHECK-DAG:  OpDecorate [[indices:%[0-9]+]] BuiltIn PrimitiveIndicesNV

struct MeshOutput {
  float4 position : SV_Position;
  float3 color : COLOR0;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void innie(out indices uint3 triangles[1], out vertices MeshOutput verts[3]) {
    SetMeshOutputCounts(3, 2);

    triangles[0] = uint3(0, 1, 2);
// CHECK: [[off:%[0-9]+]] = OpIMul %uint %uint_0 %uint_3
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_uint [[indices]] [[off]]
// CHECK: [[tmp:%[0-9]+]] = OpCompositeExtract %uint [[u3_0_1_2]] 0
// CHECK:                   OpStore [[ptr]] [[tmp]]
// CHECK: [[idx:%[0-9]+]] = OpIAdd %uint [[off]] %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_uint [[indices]] [[idx]]
// CHECK: [[tmp:%[0-9]+]] = OpCompositeExtract %uint [[u3_0_1_2]] 1
// CHECK:                   OpStore [[ptr]] [[tmp]]
// CHECK: [[idx:%[0-9]+]] = OpIAdd %uint [[off]] %uint_2
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_uint [[indices]] [[idx]]
// CHECK: [[tmp:%[0-9]+]] = OpCompositeExtract %uint [[u3_0_1_2]] 2
// CHECK:                   OpStore [[ptr]] [[tmp]]

    verts[0].position = float4(-0.5, 0.5, 0.0, 1.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v4float %gl_Position %int_0
// CHECK:                   OpStore [[ptr]] [[v4_n05_05_0_1]]
    verts[0].color = float3(1.0, 0.0, 0.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_COLOR0 %int_0
// CHECK:                   OpStore [[ptr]] [[v3_1_0_0]]

    verts[1].position = float4(0.5, 0.5, 0.0, 1.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v4float %gl_Position %int_1
// CHECK:                   OpStore [[ptr]] [[v4_05_05_0_1]]
    verts[1].color = float3(0.0, 1.0, 0.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_COLOR0 %int_1
// CHECK:                   OpStore [[ptr]] [[v3_0_1_0]]

    verts[2].position = float4(0.0, -0.5, 0.0, 1.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v4float %gl_Position %int_2
// CHECK:                   OpStore [[ptr]] [[v4_0_n05_0_1]]
    verts[2].color = float3(0.0, 0.0, 1.0);
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_COLOR0 %int_2
// CHECK:                   OpStore [[ptr]] [[v3_0_0_1]]

}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void outie(out indices uint3 triangles[1], out vertices MeshOutput verts[3]) {
	innie(triangles, verts);
}
