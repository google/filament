@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d<r32sint, read_write>;

fn textureLoad_5cd3fc() -> vec4<i32> {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_5cd3fc();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_5cd3fc();
}
