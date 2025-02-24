//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAll_c962bd() {
  bool res = WaveActiveAllTrue(true);
  return (all((res == false)) ? 1 : 0);
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAll_c962bd() {
  bool res = WaveActiveAllTrue(true);
  return (all((res == false)) ? 1 : 0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
  return;
}
