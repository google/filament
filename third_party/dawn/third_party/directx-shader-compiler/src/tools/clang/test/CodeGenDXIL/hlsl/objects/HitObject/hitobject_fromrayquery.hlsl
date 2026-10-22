// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL

// DXIL: %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32 263, i32 %[[RQ:[^ ]+]])  ; HitObject_FromRayQuery(rayQueryHandle)
// DXIL: %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %[[RQ]], i32 16, %struct.CustomAttrs* nonnull %{{[^ ]+}})  ; HitObject_FromRayQueryWithAttrs(rayQueryHandle,HitKind,CommittedAttribs)

RaytracingAccelerationStructure RTAS;
RWStructuredBuffer<float> UAV : register(u0);

RayDesc MakeRayDesc() {
  RayDesc desc;
  desc.Origin = float3(0, 0, 0);
  desc.Direction = float3(1, 0, 0);
  desc.TMin = 0.0f;
  desc.TMax = 9999.0;
  return desc;
}

struct CustomAttrs {
  float x;
  float y;
};

void Use(in dx::HitObject hit) {
  dx::MaybeReorderThread(hit);
}

[shader("raygeneration")]
void main() {
  RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
  RayDesc ray = MakeRayDesc();
  q.TraceRayInline(RTAS, RAY_FLAG_NONE, 0xFF, ray);

  Use(dx::HitObject::FromRayQuery(q));

  CustomAttrs attrs = {1.f, 2.f};
  Use(dx::HitObject::FromRayQuery(q, 16, attrs));
}
