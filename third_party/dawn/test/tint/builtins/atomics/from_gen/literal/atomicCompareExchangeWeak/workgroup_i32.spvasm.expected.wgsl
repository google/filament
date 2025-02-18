struct x__atomic_compare_exchange_resulti32 {
  /* @offset(0) */
  old_value : i32,
  /* @offset(4) */
  exchanged : bool,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicCompareExchangeWeak_e88938() {
  var res = x__atomic_compare_exchange_resulti32(0i, false);
  let old_value_1 = atomicCompareExchangeWeak(&(arg_0), 1i, 1i).old_value;
  let x_18 = old_value_1;
  res = x__atomic_compare_exchange_resulti32(x_18, (x_18 == 1i));
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicCompareExchangeWeak_e88938();
  return;
}

fn compute_main_1() {
  let x_36 = local_invocation_index_1;
  compute_main_inner(x_36);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
