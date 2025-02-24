fn select_43741e() {
  var res = select(vec4(1.0), vec4(1.0), vec4<bool>(true));
}

@fragment
fn fragment_main() {
  select_43741e();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_43741e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_43741e();
  return out;
}
