fn acosh_3433e8() {
  var res = acosh(1.54308063479999990619);
}

@fragment
fn fragment_main() {
  acosh_3433e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_3433e8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  acosh_3433e8();
  return out;
}
