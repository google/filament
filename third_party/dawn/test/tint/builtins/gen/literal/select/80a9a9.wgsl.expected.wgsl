@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_80a9a9() -> i32 {
  var res : vec3<bool> = select(vec3<bool>(true), vec3<bool>(true), vec3<bool>(true));
  return select(0, 1, all((res == vec3<bool>())));
}

@fragment
fn fragment_main() {
  prevent_dce = select_80a9a9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_80a9a9();
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
  out.prevent_dce = select_80a9a9();
  return out;
}
