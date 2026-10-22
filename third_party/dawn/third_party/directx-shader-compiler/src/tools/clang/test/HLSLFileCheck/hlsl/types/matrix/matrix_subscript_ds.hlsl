// RUN: %dxc -DMIDX=1 -DVIDX=2 -T ds_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=2 -T ds_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=j -T ds_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=j -T ds_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=2 -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=2 -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=j -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=j -T lib_6_3 %s | FileCheck %s

// Specific test for subscript operation on OutputPatch matrix data

struct MatStruct {
 float4x4 mtx : M;
};

float4 GetRow(const OutputPatch<MatStruct, 3> tri, int i, int j)
{
  return tri[MIDX].mtx[VIDX];
}

[domain("tri")]
[shader("domain")]
float4 main(int i : I, int j : J, const OutputPatch<MatStruct, 3> tri) : SV_Position {

  float4 ret = 0;

  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 {{%?[0-9]*}}, i8 2, i32 {{%?[0-9]*}})
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 {{%?[0-9]*}}, i8 2, i32 {{%?[0-9]*}})
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 {{%?[0-9]*}}, i8 2, i32 {{%?[0-9]*}})
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 {{%?[0-9]*}}, i8 2, i32 {{%?[0-9]*}})
  ret += tri[MIDX].mtx[VIDX];

  ret += GetRow(tri, MIDX, VIDX);

  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %{{.*}})
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %{{.*}})
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %{{.*}})
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %{{.*}})
  return ret;
}
