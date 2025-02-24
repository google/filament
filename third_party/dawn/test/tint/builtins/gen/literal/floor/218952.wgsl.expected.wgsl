fn floor_218952() {
  var res = floor(vec4(1.5));
}

@fragment
fn fragment_main() {
  floor_218952();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_218952();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  floor_218952();
  return out;
}
