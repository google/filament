var<workgroup> arg_0 : atomic<i32>;

fn atomicStore_8bea94() {
  var arg_1 = 1i;
  atomicStore(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_8bea94();
}
