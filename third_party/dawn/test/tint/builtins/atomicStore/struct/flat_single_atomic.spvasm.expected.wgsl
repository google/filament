struct S_atomic {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : atomic<u32>,
  /* @offset(8) */
  y : u32,
}

struct S {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : u32,
  /* @offset(8) */
  y : u32,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : S_atomic;

fn compute_main_inner(local_invocation_index_2 : u32) {
  wg.x = 0i;
  atomicStore(&(wg.a), 0u);
  wg.y = 0u;
  workgroupBarrier();
  atomicStore(&(wg.a), 1u);
  return;
}

fn compute_main_1() {
  let x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
