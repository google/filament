SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 quadSwapX_07f1fc() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, quadSwapX_07f1fc());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, quadSwapX_07f1fc());
}

FXC validation failure:
<scrubbed_path>(5,15-36): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
