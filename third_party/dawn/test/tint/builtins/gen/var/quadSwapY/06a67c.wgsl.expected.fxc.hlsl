SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapY_06a67c() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_06a67c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_06a67c()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-36): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
