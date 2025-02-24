fn reflect_bba2d0() {
  var res = reflect(vec2(1.0), vec2(1.0));
}

@fragment
fn fragment_main() {
  reflect_bba2d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_bba2d0();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  reflect_bba2d0();
  return out;
}
