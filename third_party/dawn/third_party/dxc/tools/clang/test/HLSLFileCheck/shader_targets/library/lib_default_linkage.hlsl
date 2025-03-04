// RUN: %dxc -auto-binding-space 13 -T lib_6_3 %s | FileCheck %s

// always has export_fn
// CHECK: define float @"\01?export_fn
// never has static_fn
// CHECK-NOT: static_fn
// default_fn depends on target (6.3 default is internal)
// CHECK-NOT: define float @"\01?defaut_fn

// Always has entry points:
// CHECK: define void @"\01?AnyHit
// CHECK: define void @"\01?Callable
// CHECK: define void @PSMain

export float export_fn() { return 2.0; }
static float static_fn() { return 1.0; }
float defaut_fn() { return 3.0; }

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

