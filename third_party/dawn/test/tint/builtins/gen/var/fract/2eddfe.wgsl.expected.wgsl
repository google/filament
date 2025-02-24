fn fract_2eddfe() {
  const arg_0 = 1.25;
  var res = fract(arg_0);
}

@fragment
fn fragment_main() {
  fract_2eddfe();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_2eddfe();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fract_2eddfe();
  return out;
}
