var<workgroup> arg_0 : atomic<u32>;

fn atomicCompareExchangeWeak_83580d() {
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res = atomicCompareExchangeWeak(&(arg_0), arg_1, arg_2);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_83580d();
}
