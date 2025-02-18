var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicMax_a89cc3() {
  var res = 0i;
  let x_11 = atomicMax(&(arg_0), 1i);
  res = x_11;
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicMax_a89cc3();
  return;
}

fn compute_main_1() {
  let x_31 = local_invocation_index_1;
  compute_main_inner(x_31);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
