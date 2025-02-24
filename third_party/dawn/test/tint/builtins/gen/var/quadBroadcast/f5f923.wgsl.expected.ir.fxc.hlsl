SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_f5f923() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
}

FXC validation failure:
<scrubbed_path>(5,14-38): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
