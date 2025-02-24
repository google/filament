// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tcs_6_8 -verify %s

// Test the ordinary compute shader model case with local call-site errors.
// Since these errors are emitted before we do diagnostics for reachable calls,
// and because those diagnostics are skipped if an error is already produced,
// we must do these tests separately from the ones based on reachability.

[Shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  // expected-error@+1{{invalid MemoryTypeFlags for Barrier operation; expected 0, ALL_MEMORY, or some combination of UAV_MEMORY, GROUP_SHARED_MEMORY, NODE_INPUT_MEMORY, NODE_OUTPUT_MEMORY flags}}
  Barrier(16, 0);
  // expected-error@+1{{invalid MemoryTypeFlags for Barrier operation; expected 0, ALL_MEMORY, or some combination of UAV_MEMORY, GROUP_SHARED_MEMORY, NODE_INPUT_MEMORY, NODE_OUTPUT_MEMORY flags}}
  Barrier(-1, 0);
  // expected-error@+1{{invalid SemanticFlags for Barrier operation; expected 0 or some combination of GROUP_SYNC, GROUP_SCOPE, DEVICE_SCOPE flags}}
  Barrier(0, 8);
  // expected-error@+1{{invalid SemanticFlags for Barrier operation; expected 0 or some combination of GROUP_SYNC, GROUP_SCOPE, DEVICE_SCOPE flags}}
  Barrier(0, -1);
}
