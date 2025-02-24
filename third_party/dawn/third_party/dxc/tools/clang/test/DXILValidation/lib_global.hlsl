// Make constructor for static global variable is called.

Texture2D    g_txDiffuse;
SamplerState    g_samLinear;

cbuffer X {
  // Use min-precision type to force conversion of constant buffer type for legacy.
  // This has to happen at link time at the moment, so this will break unless type
  // annotations are retained for library.
  min16float f;
}

static float g[2] = { 1, f };

[shader("pixel")]
float4 test(float2 c : C) : SV_TARGET
{
  float4 x = g_txDiffuse.Sample( g_samLinear, c );
  return x + g[1];
}

void update() {
  g[1]++;
}