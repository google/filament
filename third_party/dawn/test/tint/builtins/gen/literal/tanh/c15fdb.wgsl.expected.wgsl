@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn tanh_c15fdb() -> f32 {
  var res : f32 = tanh(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = tanh_c15fdb();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = tanh_c15fdb();
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
  out.prevent_dce = tanh_c15fdb();
  return out;
}
