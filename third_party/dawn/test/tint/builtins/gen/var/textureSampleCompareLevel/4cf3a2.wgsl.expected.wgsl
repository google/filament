@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_4cf3a2() -> f32 {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1i;
  var arg_4 = 1.0f;
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleCompareLevel_4cf3a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureSampleCompareLevel_4cf3a2();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureSampleCompareLevel_4cf3a2();
  return out;
}
