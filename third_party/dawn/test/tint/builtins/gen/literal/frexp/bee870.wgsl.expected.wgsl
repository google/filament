fn frexp_bee870() {
  var res = frexp(1.0);
}

@fragment
fn fragment_main() {
  frexp_bee870();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_bee870();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_bee870();
  return out;
}
