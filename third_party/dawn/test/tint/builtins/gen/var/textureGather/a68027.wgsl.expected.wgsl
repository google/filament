@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureGather_a68027() -> vec4<f32> {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1u;
  const arg_4 = vec2<i32>(1i);
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureGather_a68027();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureGather_a68027();
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
  out.prevent_dce = textureGather_a68027();
  return out;
}
