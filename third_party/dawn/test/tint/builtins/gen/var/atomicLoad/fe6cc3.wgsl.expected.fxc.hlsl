//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicLoad(uint offset) {
  uint value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


uint atomicLoad_fe6cc3() {
  uint res = sb_rwatomicLoad(0u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_fe6cc3()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicLoad(uint offset) {
  uint value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


uint atomicLoad_fe6cc3() {
  uint res = sb_rwatomicLoad(0u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_fe6cc3()));
  return;
}
