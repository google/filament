alias Arr = array<u32, 1u>;

alias Arr_1 = array<Arr, 2u>;

alias Arr_2 = array<Arr_1, 3u>;

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : array<array<array<atomic<u32>, 1u>, 2u>, 3u>;

fn compute_main_inner(local_invocation_index_2 : u32) {
  var idx = 0u;
  idx = local_invocation_index_2;
  loop {
    if (!((idx < 6u))) {
      break;
    }
    let x_31 = idx;
    let x_33 = idx;
    let x_35 = idx;
    atomicStore(&(wg[(x_31 / 2u)][(x_33 % 2u)][(x_35 % 1u)]), 0u);

    continuing {
      idx = (idx + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg[2i][1i][0i]), 1u);
  return;
}

fn compute_main_1() {
  let x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
