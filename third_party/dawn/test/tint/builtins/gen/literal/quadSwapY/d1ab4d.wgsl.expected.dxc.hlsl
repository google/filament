//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 quadSwapY_d1ab4d() {
  float3 res = QuadReadAcrossY((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 quadSwapY_d1ab4d() {
  float3 res = QuadReadAcrossY((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
  return;
}
