@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32uint, read_write>;

fn textureDimensions_eb1249() -> u32 {
  var res : u32 = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_eb1249();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_eb1249();
}
