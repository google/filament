@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_a3ca7e() -> f32 {
  var res : f32 = textureSampleCompare(arg_0, arg_1, vec3<f32>(1.0f), 1i, 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleCompare_a3ca7e();
}
