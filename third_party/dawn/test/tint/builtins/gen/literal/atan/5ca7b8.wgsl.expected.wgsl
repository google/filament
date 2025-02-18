fn atan_5ca7b8() {
  var res = atan(vec2(1.0));
}

@fragment
fn fragment_main() {
  atan_5ca7b8();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_5ca7b8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atan_5ca7b8();
  return out;
}
