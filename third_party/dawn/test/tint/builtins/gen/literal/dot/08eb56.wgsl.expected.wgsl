fn dot_08eb56() {
  var res = dot(vec4(1.0), vec4(1.0));
}

@fragment
fn fragment_main() {
  dot_08eb56();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_08eb56();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  dot_08eb56();
  return out;
}
