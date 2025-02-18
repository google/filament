SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapX_bddb9f() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_bddb9f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_bddb9f()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-36): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
