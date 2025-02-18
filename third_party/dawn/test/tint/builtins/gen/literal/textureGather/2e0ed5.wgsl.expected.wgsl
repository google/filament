@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureGather_2e0ed5() -> vec4<f32> {
  var res : vec4<f32> = textureGather(arg_0, arg_1, vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureGather_2e0ed5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureGather_2e0ed5();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureGather_2e0ed5();
  return out;
}
