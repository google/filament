alias Arr = array<u32, 10u>;

struct S_atomic {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : array<atomic<u32>, 10u>,
  /* @offset(44) */
  y : u32,
}

struct S {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : Arr,
  /* @offset(44) */
  y : u32,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : S_atomic;

fn compute_main_inner(local_invocation_index_2 : u32) {
  var idx = 0u;
  wg.x = 0i;
  wg.y = 0u;
  idx = local_invocation_index_2;
  loop {
    if (!((idx < 10u))) {
      break;
    }
    let x_35 = idx;
    atomicStore(&(wg.a[x_35]), 0u);

    continuing {
      idx = (idx + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg.a[4i]), 1u);
  return;
}

fn compute_main_1() {
  let x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
