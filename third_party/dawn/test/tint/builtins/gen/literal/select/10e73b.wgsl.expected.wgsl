enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn select_10e73b() -> f16 {
  var res : f16 = select(1.0h, 1.0h, true);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_10e73b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_10e73b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = select_10e73b();
  return out;
}
