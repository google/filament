fn sign_c8289c() {
  var res = sign(1.0);
}

@fragment
fn fragment_main() {
  sign_c8289c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_c8289c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sign_c8289c();
  return out;
}
