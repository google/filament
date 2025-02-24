fn floor_dcd5a2() {
  var res = floor(1.5);
}

@fragment
fn fragment_main() {
  floor_dcd5a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_dcd5a2();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  floor_dcd5a2();
  return out;
}
