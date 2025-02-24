@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(1) var arg_1 : texture_2d<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_d8e958() -> vec4<f32> {
  const arg_0 = 1u;
  var arg_3 = vec2<f32>(1.0f);
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureGather_d8e958();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureGather_d8e958();
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
  out.prevent_dce = textureGather_d8e958();
  return out;
}
