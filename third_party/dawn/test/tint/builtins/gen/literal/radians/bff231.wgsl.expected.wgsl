fn radians_bff231() {
  var res = radians(1.0);
}

@fragment
fn fragment_main() {
  radians_bff231();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_bff231();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  radians_bff231();
  return out;
}
