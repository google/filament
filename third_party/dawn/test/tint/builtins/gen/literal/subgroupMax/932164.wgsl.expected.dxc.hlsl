//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMax_932164() {
  int2 res = WaveActiveMax((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_932164()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMax_932164() {
  int2 res = WaveActiveMax((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_932164()));
  return;
}
