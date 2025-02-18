fn faceForward_fe522b() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = vec3(1.0);
  var res = faceForward(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  faceForward_fe522b();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_fe522b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  faceForward_fe522b();
  return out;
}
