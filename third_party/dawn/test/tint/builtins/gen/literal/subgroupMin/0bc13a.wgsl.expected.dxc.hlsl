//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMin_0bc13a() {
  int2 res = WaveActiveMin((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_0bc13a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMin_0bc13a() {
  int2 res = WaveActiveMin((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_0bc13a()));
  return;
}
