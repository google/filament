@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

var<workgroup> arg_0 : f32;

fn workgroupUniformLoad_7a857c() -> f32 {
  var res : f32 = workgroupUniformLoad(&(arg_0));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = workgroupUniformLoad_7a857c();
}
