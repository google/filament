var<workgroup> arg_0 : atomic<u32>;

fn atomicStore_726882() {
  var arg_1 = 1u;
  atomicStore(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_726882();
}
