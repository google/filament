fn acos_15d35b() {
  var res = acos(vec2(0.96891242171000002692));
}

@fragment
fn fragment_main() {
  acos_15d35b();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_15d35b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  acos_15d35b();
  return out;
}
