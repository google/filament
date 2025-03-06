// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// CHECK: une
// CHECK: bitcast float {{.*}} to i32
// CHECK: and
// CHECK: bitcast i32 {{.*}} to float

// for float4
// CHECK: une
// CHECK: une
// CHECK: une
// CHECK: une
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: and
// CHECK: and
// CHECK: and
// CHECK: and
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float

// For float2x2
// CHECK: une
// CHECK: une
// CHECK: une
// CHECK: une
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: bitcast float {{.*}} to i32
// CHECK: and
// CHECK: and
// CHECK: and
// CHECK: and
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float
// CHECK: bitcast i32 {{.*}} to float

float4 main(float f : F, float4 vf: VF, float2x2 mf: MF) : SV_Target {
  float ef;
  float4 evf;
  float2x2 emf;
  
  float4 c = frexp(f, ef);
  c += ef;
  c += frexp(vf, evf);
  c += evf;
  float4 tm = mf;

  c += (float4) frexp(mf, emf);
  c += (float4)emf;

  return c;
}