SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapY_be4e72() {
  int3 res = QuadReadAcrossY((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
}

FXC validation failure:
<scrubbed_path>(4,14-42): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
