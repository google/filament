SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 quadSwapX_edfa1f() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_edfa1f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_edfa1f()));
}

FXC validation failure:
<scrubbed_path>(5,14-35): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
