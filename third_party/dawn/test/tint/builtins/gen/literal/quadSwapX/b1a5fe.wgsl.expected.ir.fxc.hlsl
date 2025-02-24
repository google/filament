SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapX_b1a5fe() {
  int2 res = QuadReadAcrossX((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_b1a5fe()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_b1a5fe()));
}

FXC validation failure:
<scrubbed_path>(4,14-41): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
