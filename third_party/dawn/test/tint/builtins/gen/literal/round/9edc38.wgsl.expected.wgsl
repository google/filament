@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn round_9edc38() -> f32 {
  var res : f32 = round(3.5f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = round_9edc38();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = round_9edc38();
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
  out.prevent_dce = round_9edc38();
  return out;
}
