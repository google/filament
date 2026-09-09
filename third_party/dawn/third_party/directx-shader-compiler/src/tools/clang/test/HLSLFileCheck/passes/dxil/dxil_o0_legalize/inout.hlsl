// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// Regression test for some `store float undef` not getting cleaned up in O0.

// CHECK: @main

Texture2D t0 : register( t0 );
SamplerState s0 : register( s0 );

struct Out {
  float4 color;
};

struct In {
  float2 uv;
  float4 colors[9];
};

void FillColors(inout In input) {
  static const int2 offsets[9] = {
   int2( -1, -1 ),
   int2(  0, -1 ),
   int2(  1, -1 ),
   int2( -1,  0 ),
   int2(  0,  0 ),
   int2(  1,  0 ),
   int2( -1,  1 ),
   int2(  0,  1 ),
   int2(  1,  1 ),
  };

  [unroll]
  for(uint i = 0; i < 9; i++) {
    input.colors[ i ] = t0.Sample(s0, input.uv, offsets[i]);
  }
}

Out GetColor(inout In input) {
  Out output;
  output.color = input.colors[0];
  return output;
}

Out Compute(In input) {
 FillColors(input);
 return GetColor(input);
}

float4 main(float2 uv : UV)  : SV_Target {
 In input;
 input.uv = uv;
 return Compute(input).color;
}

