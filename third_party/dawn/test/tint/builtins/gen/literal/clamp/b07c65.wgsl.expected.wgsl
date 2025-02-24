@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn clamp_b07c65() -> i32 {
  var res : i32 = clamp(1i, 1i, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = clamp_b07c65();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = clamp_b07c65();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = clamp_b07c65();
  return out;
}
