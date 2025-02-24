fn step_38cd79() {
  var res = step(vec4(1.0), vec4(1.0));
}

@fragment
fn fragment_main() {
  step_38cd79();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_38cd79();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  step_38cd79();
  return out;
}
