fn atan_749e1b() {
  const arg_0 = vec3(1.0);
  var res = atan(arg_0);
}

@fragment
fn fragment_main() {
  atan_749e1b();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_749e1b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atan_749e1b();
  return out;
}
