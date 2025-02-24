SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 quadSwapY_d1ab4d() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-37): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
