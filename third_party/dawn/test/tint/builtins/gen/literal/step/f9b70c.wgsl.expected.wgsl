fn step_f9b70c() {
  var res = step(1.0, 1.0);
}

@fragment
fn fragment_main() {
  step_f9b70c();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_f9b70c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  step_f9b70c();
  return out;
}
