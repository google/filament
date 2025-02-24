var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicStore_8bea94() {
  var arg_1 = 0i;
  arg_1 = 1i;
  let x_19 = arg_1;
  atomicStore(&(arg_0), x_19);
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicStore_8bea94();
  return;
}

fn compute_main_1() {
  let x_32 = local_invocation_index_1;
  compute_main_inner(x_32);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
