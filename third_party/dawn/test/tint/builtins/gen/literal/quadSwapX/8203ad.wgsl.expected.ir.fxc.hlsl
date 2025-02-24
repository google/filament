SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapX_8203ad() {
  uint res = QuadReadAcrossX(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadSwapX_8203ad());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadSwapX_8203ad());
}

FXC validation failure:
<scrubbed_path>(4,14-32): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
