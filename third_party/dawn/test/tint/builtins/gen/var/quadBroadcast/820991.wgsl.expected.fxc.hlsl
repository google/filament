SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 quadBroadcast_820991() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_820991()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_820991()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-40): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
