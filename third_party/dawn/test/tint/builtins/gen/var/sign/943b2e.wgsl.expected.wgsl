fn sign_943b2e() {
  const arg_0 = vec2(1);
  var res = sign(arg_0);
}

@fragment
fn fragment_main() {
  sign_943b2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_943b2e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sign_943b2e();
  return out;
}
