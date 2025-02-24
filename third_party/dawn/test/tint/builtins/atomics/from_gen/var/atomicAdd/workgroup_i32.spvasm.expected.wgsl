var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicAdd_794055() {
  var arg_1 = 0i;
  var res = 0i;
  arg_1 = 1i;
  let x_19 = arg_1;
  let x_15 = atomicAdd(&(arg_0), x_19);
  res = x_15;
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicAdd_794055();
  return;
}

fn compute_main_1() {
  let x_33 = local_invocation_index_1;
  compute_main_inner(x_33);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
