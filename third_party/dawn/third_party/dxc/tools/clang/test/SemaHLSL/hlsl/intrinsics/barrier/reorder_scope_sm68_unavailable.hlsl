// RUN: %dxc -Tlib_6_8 -verify %s

[Shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  // expected-error@+1{{invalid SemanticFlags for Barrier operation; expected 0 or some combination of GROUP_SYNC, GROUP_SCOPE, DEVICE_SCOPE flags}}
  Barrier(0, REORDER_SCOPE);
}
