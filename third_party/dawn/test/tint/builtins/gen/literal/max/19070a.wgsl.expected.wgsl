fn max_19070a() {
  var res = max(vec4(1), vec4(1));
}

@fragment
fn fragment_main() {
  max_19070a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_19070a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_19070a();
  return out;
}
