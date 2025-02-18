@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16uint, read_write>;

fn textureLoad_02c48d() -> vec4<u32> {
  var res : vec4<u32> = textureLoad(arg_0, vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_02c48d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_02c48d();
}
