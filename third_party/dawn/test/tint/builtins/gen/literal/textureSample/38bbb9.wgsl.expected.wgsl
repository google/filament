@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_38bbb9() -> f32 {
  var res : f32 = textureSample(arg_0, arg_1, vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSample_38bbb9();
}
