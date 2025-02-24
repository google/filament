fn faceForward_2c4d14() {
  var res = faceForward(vec4(1.0), vec4(1.0), vec4(1.0));
}

@fragment
fn fragment_main() {
  faceForward_2c4d14();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_2c4d14();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  faceForward_2c4d14();
  return out;
}
