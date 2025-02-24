var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicLoad_afcc03() {
  var res = 0i;
  let x_11 = atomicLoad(&(arg_0));
  res = x_11;
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicLoad_afcc03();
  return;
}

fn compute_main_1() {
  let x_30 = local_invocation_index_1;
  compute_main_inner(x_30);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
