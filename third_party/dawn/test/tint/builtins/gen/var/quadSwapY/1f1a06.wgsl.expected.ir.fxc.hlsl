SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapY_1f1a06() {
  float2 arg_0 = (1.0f).xx;
  float2 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_1f1a06()));
}

FXC validation failure:
<scrubbed_path>(5,16-37): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
