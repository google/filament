@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn distance_aa4055() -> f32 {
  var res : f32 = distance(vec2<f32>(1.0f), vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = distance_aa4055();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = distance_aa4055();
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
  out.prevent_dce = distance_aa4055();
  return out;
}
