fn select_e381c3() {
  var res = select(vec4(1), vec4(1), true);
}

@fragment
fn fragment_main() {
  select_e381c3();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_e381c3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_e381c3();
  return out;
}
