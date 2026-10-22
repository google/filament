// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: RasterizerOrdered objects are only allowed in 5.0+ pixel shaders
RasterizerOrderedBuffer<float4> r;

min16float min16float_val;

float4 main(float4 p: Position) : SV_Position {
  // TODO: enable a C H E C K: Instructions marked precise may not refer to minprecision values
  precise min16float min16float_local = min16float_val;
  if (min16float_local != 0) {
    return 1;
  }
  return r[0];
}
