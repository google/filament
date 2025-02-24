//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicAnd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


uint atomicAnd_85a8d9() {
  uint res = sb_rwatomicAnd(0u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicAnd_85a8d9()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicAnd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


uint atomicAnd_85a8d9() {
  uint res = sb_rwatomicAnd(0u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicAnd_85a8d9()));
  return;
}
