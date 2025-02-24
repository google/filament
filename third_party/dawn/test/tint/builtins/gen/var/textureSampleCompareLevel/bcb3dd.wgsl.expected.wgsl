@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_bcb3dd() -> f32 {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1u;
  var arg_4 = 1.0f;
  const arg_5 = vec2<i32>(1i);
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleCompareLevel_bcb3dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureSampleCompareLevel_bcb3dd();
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
  out.prevent_dce = textureSampleCompareLevel_bcb3dd();
  return out;
}
