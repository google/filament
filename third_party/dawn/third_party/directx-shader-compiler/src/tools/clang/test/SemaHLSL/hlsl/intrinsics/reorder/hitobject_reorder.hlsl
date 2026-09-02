// RUN: %dxc -T lib_6_9 -E main %s -verify

// expected-no-diagnostics

[shader("raygeneration")] void main() {
  dx::HitObject hit;
  dx::MaybeReorderThread(hit);
  dx::MaybeReorderThread(hit, 0xf1, 3);
  dx::MaybeReorderThread(0xf2, 7);
}
