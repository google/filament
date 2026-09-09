// RUN: %dxc -DMIDX=1 -DVIDX=2 -T hs_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=2 -T hs_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=j -T hs_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=j -T hs_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=2 -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=2 -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=j -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=j -T lib_6_3 %s | FileCheck %s

// Specific test for subscript operation on matrix array inputs in patch functions

struct MatStruct {
  int2 uv : TEXCOORD0;
  float3x4  m_ObjectToWorld : TEXCOORD1;
};

struct Output {
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

// Instruction order here is a bit inconsistent.
// So we can't test for all the outputs
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call void @dx.op.storePatchConstant.f32
// CHECK: call void @dx.op.storePatchConstant.f32
Output Patch(InputPatch<MatStruct, 3> inputs)
{
  Output ret;
  int i = inputs[0].uv.x;
  int j = inputs[0].uv.y;

  ret.edges[0] = inputs[MIDX].m_ObjectToWorld[VIDX][0];
  ret.edges[1] = inputs[MIDX].m_ObjectToWorld[VIDX][1];
  ret.edges[2] = inputs[MIDX].m_ObjectToWorld[VIDX][2];
  ret.inside = 1.0f;
  return ret;
}


// CHECK: call float @dx.op.loadInput.f32
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call float @dx.op.loadInput.f32
// CHECK: call void @dx.op.storeOutput.f32
// CHECK: call void @dx.op.storeOutput.f32
// CHECK: call void @dx.op.storeOutput.f32
// CHECK: call void @dx.op.storeOutput.f32
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("Patch")]
[outputcontrolpoints(3)]
[shader("hull")]
float4 main(InputPatch<MatStruct, 3> inputs) : SV_Position
{
  int i = inputs[0].uv.x;
  int j = inputs[0].uv.y;
  return inputs[MIDX].m_ObjectToWorld[VIDX];
}
