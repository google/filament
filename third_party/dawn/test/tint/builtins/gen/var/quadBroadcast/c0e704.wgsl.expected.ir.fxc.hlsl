SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_c0e704() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_c0e704()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_c0e704()));
}

FXC validation failure:
<scrubbed_path>(5,14-42): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
