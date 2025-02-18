@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_c31f9e() -> i32 {
  var res : bool = select(true, true, true);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = select_c31f9e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_c31f9e();
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
  out.prevent_dce = select_c31f9e();
  return out;
}
