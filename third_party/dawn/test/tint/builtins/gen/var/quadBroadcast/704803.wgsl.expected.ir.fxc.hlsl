SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadBroadcast_704803() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
}

FXC validation failure:
<scrubbed_path>(5,14-42): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
