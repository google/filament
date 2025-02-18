enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

var<workgroup> arg_0 : f16;

fn workgroupUniformLoad_e07d08() -> f16 {
  var res : f16 = workgroupUniformLoad(&(arg_0));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = workgroupUniformLoad_e07d08();
}
