// RUN: %dxc -T ps_6_0 -verify %s 
// This file tests that functions with unspecified linkage will default to 
// internal for non-library shaders, so we expect no errors on unreachable 
// functions that are recursive. Even `export` on non-library shaders will
// not emit diagnostics on unreachable recursive functions.

// expected-no-diagnostics
void unreachable_unexported_recurse_external(inout float4 f, float a) 
{
    if (a > 1)
      unreachable_unexported_recurse_external(f, a-1);
    f = abs(f+a);
}

static void unreachable_unexported_recurse(inout float4 f, float a) 
{
    if (a > 1)
      unreachable_unexported_recurse(f, a-1);
    f = abs(f+a);
}

static void unexported_recurse(inout float4 f, float a) 
{
    if (a > 1)
      unexported_recurse(f, a-1);
    f = abs(f+a);
}

export void exported_recurse(inout float4 f, float a) 
{
    if (a > 1)
      exported_recurse(f, a-1);
    f = abs(f+a);
}

export void exported_recurse_2(inout float4 f, float a) 
{
    if (a > 1)
      unexported_recurse(f, a-1);
    f = abs(f+a);
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  float4 f = b;
  return f;
}
