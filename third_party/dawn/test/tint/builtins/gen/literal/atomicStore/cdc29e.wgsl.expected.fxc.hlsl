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
}

void fragment_main() {
  atomicStore_cdc29e();
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
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_cdc29e();
  return;
}
