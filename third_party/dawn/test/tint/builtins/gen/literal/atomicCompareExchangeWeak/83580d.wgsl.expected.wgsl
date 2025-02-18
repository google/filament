var<workgroup> arg_0 : atomic<u32>;

fn atomicCompareExchangeWeak_83580d() {
  var res = atomicCompareExchangeWeak(&(arg_0), 1u, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_83580d();
}
