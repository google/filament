fn faceForward_b42ef3() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = vec2(1.0);
  var res = faceForward(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  faceForward_b42ef3();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_b42ef3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  faceForward_b42ef3();
  return out;
}
