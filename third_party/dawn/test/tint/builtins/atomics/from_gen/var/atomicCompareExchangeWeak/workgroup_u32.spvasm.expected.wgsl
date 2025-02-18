struct x__atomic_compare_exchange_resultu32 {
  /* @offset(0) */
  old_value : u32,
  /* @offset(4) */
  exchanged : bool,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicCompareExchangeWeak_83580d() {
  var arg_1 = 0u;
  var arg_2 = 0u;
  var res = x__atomic_compare_exchange_resultu32(0u, false);
  arg_1 = 1u;
  arg_2 = 1u;
  let x_21 = arg_2;
  let x_22 = arg_1;
  let old_value_1 = atomicCompareExchangeWeak(&(arg_0), x_22, x_21).old_value;
  let x_23 = old_value_1;
  res = x__atomic_compare_exchange_resultu32(x_23, (x_23 == x_21));
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicCompareExchangeWeak_83580d();
  return;
}

fn compute_main_1() {
  let x_40 = local_invocation_index_1;
  compute_main_inner(x_40);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
