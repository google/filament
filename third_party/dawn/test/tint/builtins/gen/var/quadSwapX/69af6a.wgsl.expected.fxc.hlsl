SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 quadSwapX_69af6a() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_69af6a()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_69af6a()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-37): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
