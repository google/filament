@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

var<workgroup> arg_0 : u32;

fn workgroupUniformLoad_37307c() -> u32 {
  var res : u32 = workgroupUniformLoad(&(arg_0));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = workgroupUniformLoad_37307c();
}
