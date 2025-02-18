fn acos_069188() {
  var res = acos(vec3(0.96891242171000002692));
}

@fragment
fn fragment_main() {
  acos_069188();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_069188();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  acos_069188();
  return out;
}
