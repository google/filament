struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicCompareExchangeWeak_63d8e6() {
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res = atomicCompareExchangeWeak(&(sb_rw.arg_0), arg_1, arg_2);
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}
