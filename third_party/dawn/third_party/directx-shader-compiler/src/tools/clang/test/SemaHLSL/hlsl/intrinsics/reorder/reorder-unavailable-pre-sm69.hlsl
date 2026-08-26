// RUN: %dxc -T lib_6_8 %s -verify

// Check that inciwMaybeReorderThread is unavailable pre SM 6.9.

[shader("raygeneration")]
void main() {
  // expected-error@+1{{intrinsic dx::MaybeReorderThread potentially used by ''main'' requires shader model 6.9 or greater}}
  dx::MaybeReorderThread(15u, 4u);
}
