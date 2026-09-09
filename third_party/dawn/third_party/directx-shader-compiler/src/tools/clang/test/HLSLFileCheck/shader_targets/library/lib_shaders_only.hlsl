// RUN: %dxc -auto-binding-space 13 -T lib_6_3 -export-shaders-only %s | FileCheck %s

// CHECK: %struct.Payload
// CHECK-NOT: unused
// CHECK: define void @"\01?AnyHit
// CHECK: define void @"\01?Callable
// CHECK: define void @PSMain()

Buffer<float> T_unused;

Buffer<float> GetBuffer_unused() { return T_unused; }
float unused_nonshader_export() { return 1.0; }
float unused_nonshader_export2() { return unused_nonshader_export(); }


struct Payload {
  float f;
};

[shader("anyhit")]
void AnyHit(inout Payload p, BuiltInTriangleIntersectionAttributes a) {
  p.f += a.barycentrics.x;
  if (p.f > 1.0)
    AcceptHitAndEndSearch();
  p.f += a.barycentrics.y;
}

[shader("callable")]
void Callable(inout Payload p) {
  p.f += 0.2;
}

[shader("pixel")]
float4 PSMain(float2 coord : TEXCOORD) : SV_Target {
  return coord.xyxy;
}

