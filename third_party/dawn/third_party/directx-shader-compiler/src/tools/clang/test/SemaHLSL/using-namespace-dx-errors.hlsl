// RUN: %dxc -T lib_6_9 %s -verify

RaytracingAccelerationStructure Scene : register(t0, space0);

struct[raypayload] RayPayload {
  float4 color : write(caller) : read(closesthit);
};

[shader("raygeneration")] void MyRaygenShader() {
  // Set the ray's extents.
  RayDesc ray;
  ray.Origin = float3(0, 0, 1);
  ray.Direction = float3(1, 0, 0);
  ray.TMin = 0.001;
  ray.TMax = 10000.0;

  RayPayload payload = {float4(0, 0, 0, 0)};

  {
    using namespace dx;
    HitObject hit =
        HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0,
                            ray, payload);

    int sortKey = 1;
    MaybeReorderThread(sortKey, 1);
  }

  {
    int sortKey = 1;
    MaybeReorderThread(sortKey, 1); // expected-error{{use of undeclared identifier 'MaybeReorderThread'}}
  }

  int sortKey = 1;
  MaybeReorderThread(sortKey, 1); // expected-error{{use of undeclared identifier 'MaybeReorderThread'}}

  HitObject hit = // expected-error{{unknown type name 'HitObject'}}
        HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0,
                            ray, payload);

  HitObject::Invoke(hit, payload); // expected-error{{use of undeclared identifier 'HitObject'}}
}
