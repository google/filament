@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_36f0d3() -> f32 {
  var res : f32 = textureSampleLevel(arg_0, arg_1, vec2<f32>(1.0f), 1i, 1u, vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleLevel_36f0d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureSampleLevel_36f0d3();
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
  out.prevent_dce = textureSampleLevel_36f0d3();
  return out;
}
