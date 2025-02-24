fn acosh_17260e() {
  const arg_0 = vec2(1.54308063479999990619);
  var res = acosh(arg_0);
}

@fragment
fn fragment_main() {
  acosh_17260e();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_17260e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  acosh_17260e();
  return out;
}
