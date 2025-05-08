// RUN: %dxc -T vs_6_9 %s -verify

// expected-note@+1{{entry function defined here}}
float main(RayDesc rayDesc: RAYDESC) : OUT {
// expected-error@+1{{dx::HitObject is unavailable in shader stage 'vertex' (requires 'raygeneration', 'closesthit' or 'miss')}}
  dx::HitObject::MakeNop();
  return 0.f;
}
