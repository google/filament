var<workgroup> arg_0 : atomic<i32>;

@compute @workgroup_size(1)
fn compute_main() {
  var res : i32 = atomicSub(&(arg_0), -(1));
}
