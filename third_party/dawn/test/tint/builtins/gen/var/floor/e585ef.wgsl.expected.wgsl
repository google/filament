fn floor_e585ef() {
  const arg_0 = vec2(1.5);
  var res = floor(arg_0);
}

@fragment
fn fragment_main() {
  floor_e585ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_e585ef();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  floor_e585ef();
  return out;
}
