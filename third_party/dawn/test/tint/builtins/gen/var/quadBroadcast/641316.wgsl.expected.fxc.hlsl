SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadBroadcast_641316() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_641316()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_641316()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
