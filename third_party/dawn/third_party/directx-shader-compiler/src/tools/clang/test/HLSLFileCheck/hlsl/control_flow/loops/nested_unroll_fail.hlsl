// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: error: Could not unroll loop.

[RootSignature("")]
float main(float foo : FOO) : SV_Target {
  float result = 0;
  [unroll]
  for (uint k = 0; k < 3; k++) {

    [loop]
    for (uint i = 0; i < 3; i++) {

      // This will fail, since the middle loop is not
      // unrolled.
      [unroll]
      for (uint j = 0; j <= i; j++) {
        result += cos(k + j + i + foo);
      }
    }
  }

  return result;
}

