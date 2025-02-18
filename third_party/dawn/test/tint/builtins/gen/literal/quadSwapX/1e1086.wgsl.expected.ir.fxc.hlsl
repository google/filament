SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapX_1e1086() {
  int res = QuadReadAcrossX(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

FXC validation failure:
<scrubbed_path>(4,13-35): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
