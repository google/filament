@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r32uint, read_write>;

fn textureDimensions_9944d5() -> u32 {
  var res : u32 = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_9944d5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_9944d5();
}
