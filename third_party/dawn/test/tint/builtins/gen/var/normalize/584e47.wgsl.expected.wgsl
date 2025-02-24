fn normalize_584e47() {
  const arg_0 = vec2(1.0);
  var res = normalize(arg_0);
}

@fragment
fn fragment_main() {
  normalize_584e47();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_584e47();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  normalize_584e47();
  return out;
}
