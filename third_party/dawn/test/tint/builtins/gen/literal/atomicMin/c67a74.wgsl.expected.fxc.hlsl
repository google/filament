//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


uint atomicMin_c67a74() {
  uint res = sb_rwatomicMin(0u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicMin_c67a74()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


uint atomicMin_c67a74() {
  uint res = sb_rwatomicMin(0u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicMin_c67a74()));
  return;
}
