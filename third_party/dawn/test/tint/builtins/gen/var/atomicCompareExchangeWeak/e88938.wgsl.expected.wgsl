var<workgroup> arg_0 : atomic<i32>;

fn atomicCompareExchangeWeak_e88938() {
  var arg_1 = 1i;
  var arg_2 = 1i;
  var res = atomicCompareExchangeWeak(&(arg_0), arg_1, arg_2);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicCompareExchangeWeak_e88938();
}
