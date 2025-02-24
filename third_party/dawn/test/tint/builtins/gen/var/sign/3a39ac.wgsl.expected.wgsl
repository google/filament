fn sign_3a39ac() {
  const arg_0 = 1;
  var res = sign(arg_0);
}

@fragment
fn fragment_main() {
  sign_3a39ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_3a39ac();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sign_3a39ac();
  return out;
}
