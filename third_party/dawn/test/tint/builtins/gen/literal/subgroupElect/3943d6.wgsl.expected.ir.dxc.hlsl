//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupElect_3943d6() {
  bool res = WaveIsFirstLane();
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupElect_3943d6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupElect_3943d6() {
  bool res = WaveIsFirstLane();
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupElect_3943d6()));
}

