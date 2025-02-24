SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupXor_7f6672() {
  uint2 res = WaveActiveBitXor((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupXor_7f6672());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupXor_7f6672());
}

FXC validation failure:
<scrubbed_path>(4,15-39): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
