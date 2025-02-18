fn asin_0bac07() {
  var res = asin(vec3(0.4794255386040000011));
}

@fragment
fn fragment_main() {
  asin_0bac07();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_0bac07();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  asin_0bac07();
  return out;
}
