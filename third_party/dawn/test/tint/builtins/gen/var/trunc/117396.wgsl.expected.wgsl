fn trunc_117396() {
  const arg_0 = vec3(1.5);
  var res = trunc(arg_0);
}

@fragment
fn fragment_main() {
  trunc_117396();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_117396();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  trunc_117396();
  return out;
}
