SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapY_b9d9e7() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadAcrossY(arg_0);
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
<scrubbed_path>(5,16-37): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
