// RUN: %dxc -Zi -E main -T ps_6_0 %s -O0 | FileCheck %s

// CHECK: FileThatDoesntExist.hlsl:4:3: error: Could not unroll loop. Loop bound could not be deduced at compile time.

// Regression test to confirm #line directives are reflected in the error messages.

[RootSignature("")]
float main(int a : TEXCOORD) : SV_Target {
#line 1 "FileThatDoesntExist.hlsl"

  float x = 0;
  [unroll]
  for (uint i = 0; i < a; i++)
    x++;

  return x;
}

