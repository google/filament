fn round_184d5a() {
  var res = round(vec4(3.5));
}

@fragment
fn fragment_main() {
  round_184d5a();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_184d5a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  round_184d5a();
  return out;
}
