SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 quadSwapY_14bb9a() {
  int4 res = QuadReadAcrossY((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
}

FXC validation failure:
<scrubbed_path>(4,14-43): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
