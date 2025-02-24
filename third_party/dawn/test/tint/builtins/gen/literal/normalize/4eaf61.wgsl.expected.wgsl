fn normalize_4eaf61() {
  var res = normalize(vec4(1.0));
}

@fragment
fn fragment_main() {
  normalize_4eaf61();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_4eaf61();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  normalize_4eaf61();
  return out;
}
