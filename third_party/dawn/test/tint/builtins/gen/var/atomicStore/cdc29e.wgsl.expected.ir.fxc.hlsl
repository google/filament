//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, arg_1, v);
}

void fragment_main() {
  atomicStore_cdc29e();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, arg_1, v);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_cdc29e();
}

