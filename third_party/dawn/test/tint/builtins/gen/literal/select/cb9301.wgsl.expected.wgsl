@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_cb9301() -> i32 {
  var res : vec2<bool> = select(vec2<bool>(true), vec2<bool>(true), vec2<bool>(true));
  return select(0, 1, all((res == vec2<bool>())));
}

@fragment
fn fragment_main() {
  prevent_dce = select_cb9301();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_cb9301();
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
  out.prevent_dce = select_cb9301();
  return out;
}
