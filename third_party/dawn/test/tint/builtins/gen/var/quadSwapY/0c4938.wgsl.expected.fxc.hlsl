SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint quadSwapY_0c4938() {
  uint arg_0 = 1u;
  uint res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_0c4938()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_0c4938()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-35): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
