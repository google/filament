fn atanh_70d5bd() {
  var res = atanh(vec2(0.5));
}

@fragment
fn fragment_main() {
  atanh_70d5bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_70d5bd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atanh_70d5bd();
  return out;
}
