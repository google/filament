struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<i32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : i32,
}

struct x__atomic_compare_exchange_resulti32 {
  /* @offset(0) */
  old_value : i32,
  /* @offset(4) */
  exchanged : bool,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicCompareExchangeWeak_1bd40a() {
  var arg_1 = 0i;
  var arg_2 = 0i;
  var res = x__atomic_compare_exchange_resulti32(0i, false);
  arg_1 = 1i;
  arg_2 = 1i;
  let x_23 = arg_2;
  let x_24 = arg_1;
  let old_value_1 = atomicCompareExchangeWeak(&(sb_rw.arg_0), x_24, x_23).old_value;
  let x_25 = old_value_1;
  res = x__atomic_compare_exchange_resulti32(x_25, (x_25 == x_23));
  return;
}

fn fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
