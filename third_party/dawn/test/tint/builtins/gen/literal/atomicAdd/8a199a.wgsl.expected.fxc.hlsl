//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


uint atomicAdd_8a199a() {
  uint res = sb_rwatomicAdd(0u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicAdd_8a199a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


uint atomicAdd_8a199a() {
  uint res = sb_rwatomicAdd(0u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicAdd_8a199a()));
  return;
}
