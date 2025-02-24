@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_cube;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_ea7030() -> f32 {
  var arg_2 = vec3<f32>(1.0f);
  var res : f32 = textureSample(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSample_ea7030();
}
