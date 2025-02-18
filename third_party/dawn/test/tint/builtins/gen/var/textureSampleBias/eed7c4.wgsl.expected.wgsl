@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_cube_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBias_eed7c4() -> vec4<f32> {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1i;
  var arg_4 = 1.0f;
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, arg_2, arg_3, arg_4);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleBias_eed7c4();
}
