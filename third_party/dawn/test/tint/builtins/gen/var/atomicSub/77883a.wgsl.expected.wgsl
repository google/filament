@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicSub_77883a() -> i32 {
  var arg_1 = 1i;
  var res : i32 = atomicSub(&(arg_0), arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicSub_77883a();
}
