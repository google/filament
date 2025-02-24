SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapY_0d05a8() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

FXC validation failure:
<scrubbed_path>(5,14-35): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
