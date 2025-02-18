struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicCompareExchangeWeak_1bd40a() {
  var res = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1i, 1i);
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}
