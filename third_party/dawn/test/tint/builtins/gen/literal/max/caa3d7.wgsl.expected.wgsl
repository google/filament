fn max_caa3d7() {
  var res = max(1, 1);
}

@fragment
fn fragment_main() {
  max_caa3d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_caa3d7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_caa3d7();
  return out;
}
