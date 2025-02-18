//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAny_cddda0() {
  bool arg_0 = true;
  bool res = WaveActiveAnyTrue(arg_0);
  return (all((res == false)) ? 1 : 0);
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAny_cddda0() {
  bool arg_0 = true;
  bool res = WaveActiveAnyTrue(arg_0);
  return (all((res == false)) ? 1 : 0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
  return;
}
