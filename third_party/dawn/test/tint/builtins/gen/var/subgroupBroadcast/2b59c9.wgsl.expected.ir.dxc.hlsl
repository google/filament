//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcast_2b59c9() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_2b59c9()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcast_2b59c9() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_2b59c9()));
}

