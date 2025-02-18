fn sign_55339e() {
  var res = sign(vec3(1));
}

@fragment
fn fragment_main() {
  sign_55339e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_55339e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sign_55339e();
  return out;
}
