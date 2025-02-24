struct x__atomic_compare_exchange_resulti32 {
  /* @offset(0) */
  old_value : i32,
  /* @offset(4) */
  exchanged : bool,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicCompareExchangeWeak_e88938() {
  var arg_1 = 0i;
  var arg_2 = 0i;
  var res = x__atomic_compare_exchange_resulti32(0i, false);
  arg_1 = 1i;
  arg_2 = 1i;
  let x_22 = arg_2;
  let x_23 = arg_1;
  let old_value_1 = atomicCompareExchangeWeak(&(arg_0), x_23, x_22).old_value;
  let x_24 = old_value_1;
  res = x__atomic_compare_exchange_resulti32(x_24, (x_24 == x_22));
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicCompareExchangeWeak_e88938();
  return;
}

fn compute_main_1() {
  let x_41 = local_invocation_index_1;
  compute_main_inner(x_41);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
