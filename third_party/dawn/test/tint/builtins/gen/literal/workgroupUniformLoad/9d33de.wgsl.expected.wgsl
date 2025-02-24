@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : i32;

fn workgroupUniformLoad_9d33de() -> i32 {
  var res : i32 = workgroupUniformLoad(&(arg_0));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = workgroupUniformLoad_9d33de();
}
