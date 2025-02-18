@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_193203() -> vec4<f32> {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec2<f32>(1.0f), 1u, vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSample_193203();
}
