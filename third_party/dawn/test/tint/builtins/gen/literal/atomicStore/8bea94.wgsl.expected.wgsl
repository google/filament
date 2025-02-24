var<workgroup> arg_0 : atomic<i32>;

fn atomicStore_8bea94() {
  atomicStore(&(arg_0), 1i);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_8bea94();
}
