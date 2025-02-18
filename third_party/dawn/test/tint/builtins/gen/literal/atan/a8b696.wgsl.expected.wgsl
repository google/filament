@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn atan_a8b696() -> vec4<f32> {
  var res : vec4<f32> = atan(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atan_a8b696();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atan_a8b696();
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
  out.prevent_dce = atan_a8b696();
  return out;
}
