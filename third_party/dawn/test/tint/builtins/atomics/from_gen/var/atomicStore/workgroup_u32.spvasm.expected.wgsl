var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicStore_726882() {
  var arg_1 = 0u;
  arg_1 = 1u;
  let x_18 = arg_1;
  atomicStore(&(arg_0), x_18);
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicStore_726882();
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
