SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapX_053f3b() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = QuadReadAcrossX(arg_0);
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
<scrubbed_path>(5,14-35): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
