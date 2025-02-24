fn acosh_9f213e() {
  const arg_0 = vec3(1.54308063479999990619);
  var res = acosh(arg_0);
}

@fragment
fn fragment_main() {
  acosh_9f213e();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_9f213e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  acosh_9f213e();
  return out;
}
