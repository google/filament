SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapY_b9d9e7() {
  float4 res = QuadReadAcrossY((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
}

FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
