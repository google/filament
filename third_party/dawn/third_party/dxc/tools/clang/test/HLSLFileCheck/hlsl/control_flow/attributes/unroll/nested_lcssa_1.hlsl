// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: @main

// Regression test for a validation error caused by nested LCSSA formation uses
// the wrong set of blocks when constructing LCSSA PHI's for a parent loop. This
// causes wrong assumptions to be made about which users are actually in the
// loop.

// [unroll] on both loops causes validator failure with uninitialized value
// when reading problemValue after the early conditional return.
// error: validation errors
// Instructions should not read uninitialized value

float g_float;
RWTexture2D<float4> g_Output;

float doStuff()
{
  float res = 0;
  float problemValue = 0;
  float sum = 0.0f;

  [unroll]
  for (uint i = 0; i < 3; ++i)
  {
    sum += g_float;
  }

  if (0.0 == sum)
  {
    return 0; // early conditional return that causes issue
  }

  problemValue = 1.0f * sum;

  [unroll]
  for (uint j = 0; j < 3; ++j)
  {
    res += g_float;
  }

  res *= problemValue;
  return res;
}

[numthreads(8, 8, 1)]
void main(uint2 tid : SV_DispatchThreadID)
{
  float res = 0;
  for (uint i = 0; i < 1; ++i)
  {
    res = doStuff();
  }
  g_Output[tid] = res;
}

