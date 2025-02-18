@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_4db25c() -> f32 {
  var res : f32 = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_4db25c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_4db25c();
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
  out.prevent_dce = textureLoad_4db25c();
  return out;
}
