struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicStore_cdc29e() {
  var arg_1 = 1u;
  atomicStore(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicStore_cdc29e();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_cdc29e();
}
