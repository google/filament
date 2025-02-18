SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 quadBroadcast_9d802c() {
  float4 res = QuadReadLaneAt((1.0f).xxxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_9d802c()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_9d802c()));
}

FXC validation failure:
<scrubbed_path>(4,16-50): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
