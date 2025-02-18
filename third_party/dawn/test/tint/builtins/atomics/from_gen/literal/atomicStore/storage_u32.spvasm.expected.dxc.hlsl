//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, uint value) {
  uint ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_cdc29e() {
  sb_rwatomicStore(0u, 1u);
  return;
}

void fragment_main_1() {
  atomicStore_cdc29e();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, uint value) {
  uint ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_cdc29e() {
  sb_rwatomicStore(0u, 1u);
  return;
}

void compute_main_1() {
  atomicStore_cdc29e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
