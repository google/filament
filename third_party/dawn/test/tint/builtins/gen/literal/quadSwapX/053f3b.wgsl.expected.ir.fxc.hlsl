SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapX_053f3b() {
  int3 res = QuadReadAcrossX((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_053f3b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_053f3b()));
}

FXC validation failure:
<scrubbed_path>(4,14-42): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
