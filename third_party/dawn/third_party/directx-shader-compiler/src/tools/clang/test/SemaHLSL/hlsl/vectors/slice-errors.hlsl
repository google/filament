// RUN: %dxc -T ps_6_9 -E main -verify %s

#include <vector_utils>

RWByteAddressBuffer buf;

float4 main(uint4 id: IN0) : SV_Target
{
  float4 fVec = buf.Load<vector<float, 4> >(0);

  // Ensure out-of-bounds calls raise a compiler error

  float4 lhs = hlsl::slice<4>(fVec);

  // expected-error@+1 {{no matching function for call to 'slice'}}
  float4 rhs = hlsl::slice<2, 4>(fVec);

  // expected-note@vector_utils:23 {{candidate template ignored: disabled by 'enable_if' [with Off = 2, OSz = 4, T = float, ISz = 4]}}
  // expected-note@vector_utils:35 {{candidate template ignored: invalid explicitly-specified argument for template parameter 'T'}}

  return lhs + rhs;
}

