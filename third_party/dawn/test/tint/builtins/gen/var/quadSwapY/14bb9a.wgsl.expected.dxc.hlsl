//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 quadSwapY_14bb9a() {
  int4 arg_0 = (1).xxxx;
  int4 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 quadSwapY_14bb9a() {
  int4 arg_0 = (1).xxxx;
  int4 res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_14bb9a()));
  return;
}
