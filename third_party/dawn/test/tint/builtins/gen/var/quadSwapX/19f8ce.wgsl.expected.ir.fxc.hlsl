SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadSwapX_19f8ce() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadSwapX_19f8ce());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadSwapX_19f8ce());
}

FXC validation failure:
<scrubbed_path>(5,15-36): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
