fn refract_cf1629() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = 1.0;
  var res = refract(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  refract_cf1629();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_cf1629();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  refract_cf1629();
  return out;
}
