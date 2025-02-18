struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<u32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : u32,
}

struct x__atomic_compare_exchange_resultu32 {
  /* @offset(0) */
  old_value : u32,
  /* @offset(4) */
  exchanged : bool,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicCompareExchangeWeak_63d8e6() {
  var res = x__atomic_compare_exchange_resultu32(0u, false);
  let old_value_1 = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1u, 1u).old_value;
  let x_17 = old_value_1;
  res = x__atomic_compare_exchange_resultu32(x_17, (x_17 == 1u));
  return;
}

fn fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
