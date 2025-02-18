@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureLoad2d(texture : texture_2d<i32>, coords : vec2<i32>, level : i32) -> vec4<i32> {
  return textureLoad(texture, coords, level);
}

fn doTextureLoad() {
  var res : vec4<i32> = textureLoad2d(arg_0, vec2<i32>(), 0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  doTextureLoad();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  doTextureLoad();
}

@compute @workgroup_size(1)
fn compute_main() {
  doTextureLoad();
}
