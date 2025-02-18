fn asin_0bac07() {
  const arg_0 = vec3(0.4794255386040000011);
  var res = asin(arg_0);
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
