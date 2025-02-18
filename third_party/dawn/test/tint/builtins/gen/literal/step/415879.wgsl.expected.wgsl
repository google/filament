fn step_415879() {
  var res = step(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  step_415879();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_415879();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  step_415879();
  return out;
}
