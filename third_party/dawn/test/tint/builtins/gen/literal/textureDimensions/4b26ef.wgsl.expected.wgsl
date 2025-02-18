@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8unorm, write>;

fn textureDimensions_4b26ef() -> vec3<u32> {
  var res : vec3<u32> = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_4b26ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_4b26ef();
}
