SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveAdd_7ed675() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

FXC validation failure:
<scrubbed_path>(6,15-30): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
