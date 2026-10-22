// RUN: %dxc -T as_6_5 %s | FileCheck %s

// Ensure groupshared payload still accepted when initialized with method.
// CHECK: @[[g_payload:.*]] = addrspace(3) global
// CHECK: store i32 {{.*}} i32 addrspace(3)*
// CHECK: store i32 {{.*}} i32 addrspace(3)*
// CHECK: call void @dx.op.dispatchMesh
// CHECK-SAME: addrspace(3)* nonnull @[[g_payload]]

struct SharedPayload
{
  uint2 m_a;

  void Foo( in float3 v3 )
  {
    uint3 f16Vec3 = f32tof16(v3);
    m_a.x = f16Vec3.x | (f16Vec3.y<<16);
    m_a.y = f16Vec3.z;
  }
};

groupshared SharedPayload g_payload;

[numthreads(8, 8, 1)]
void main()
{
  g_payload.Foo( 1.0.xxx );
  DispatchMesh(1,1,1,g_payload);
}
