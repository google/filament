fn max_de6b87() {
  var res = max(vec2(1.0), vec2(1.0));
}

@fragment
fn fragment_main() {
  max_de6b87();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_de6b87();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_de6b87();
  return out;
}
