// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s

// TODO: Implement lowering for dx::MaybeReorderThread

// CHECK-NOT:   call

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  dx::MaybeReorderThread(hit);
  dx::MaybeReorderThread(hit, 0xf1, 3);
  dx::MaybeReorderThread(0xf2, 7);
}
