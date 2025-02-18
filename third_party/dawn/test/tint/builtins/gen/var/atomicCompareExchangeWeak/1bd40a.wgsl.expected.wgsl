struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicCompareExchangeWeak_1bd40a() {
  var arg_1 = 1i;
  var arg_2 = 1i;
  var res = atomicCompareExchangeWeak(&(sb_rw.arg_0), arg_1, arg_2);
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}
