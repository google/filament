fn exp_dad791() {
  var res = exp(vec4(1.0));
}

@fragment
fn fragment_main() {
  exp_dad791();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_dad791();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp_dad791();
  return out;
}
