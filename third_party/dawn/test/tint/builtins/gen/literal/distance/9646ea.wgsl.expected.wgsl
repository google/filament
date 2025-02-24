@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn distance_9646ea() -> f32 {
  var res : f32 = distance(vec4<f32>(1.0f), vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = distance_9646ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = distance_9646ea();
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
  out.prevent_dce = distance_9646ea();
  return out;
}
