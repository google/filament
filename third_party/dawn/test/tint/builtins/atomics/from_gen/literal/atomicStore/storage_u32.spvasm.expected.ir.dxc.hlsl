//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, 1u, v);
}

void fragment_main_1() {
  atomicStore_cdc29e();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, 1u, v);
}

void compute_main_1() {
  atomicStore_cdc29e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

