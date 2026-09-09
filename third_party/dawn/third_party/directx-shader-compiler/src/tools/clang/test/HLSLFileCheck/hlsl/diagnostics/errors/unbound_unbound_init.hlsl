// RUN: %dxc -T ps_6_0  %s | FileCheck %s

// Verify that error for unbounded array struct member is produced
// even if the struct is initialized with a resource that is an unbounded array
// Previously, this would hang the compiler

Texture2D unboundResource[] : register(t0);

// CHECK:  error: array dimensions of struct/class members must be explicit
static const struct {
  Texture2D unboundField[];
} unboundStruct = { unboundResource };

float4 main() {
  return 0.0;
}
