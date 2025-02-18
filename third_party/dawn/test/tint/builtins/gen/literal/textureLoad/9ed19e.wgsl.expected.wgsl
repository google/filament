@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_2d;

fn textureLoad_9ed19e() -> f32 {
  var res : f32 = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_9ed19e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_9ed19e();
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
  out.prevent_dce = textureLoad_9ed19e();
  return out;
}
