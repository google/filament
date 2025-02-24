fn max_c023dd() {
  var res = max(1.0, 1.0);
}

@fragment
fn fragment_main() {
  max_c023dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_c023dd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_c023dd();
  return out;
}
