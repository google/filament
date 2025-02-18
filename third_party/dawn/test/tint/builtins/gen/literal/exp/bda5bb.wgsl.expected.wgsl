fn exp_bda5bb() {
  var res = exp(vec3(1.0));
}

@fragment
fn fragment_main() {
  exp_bda5bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_bda5bb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp_bda5bb();
  return out;
}
