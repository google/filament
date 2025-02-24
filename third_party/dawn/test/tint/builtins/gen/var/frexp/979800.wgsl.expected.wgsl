fn frexp_979800() {
  var arg_0 = vec3<f32>(1.0f);
  var res = frexp(arg_0);
}

@fragment
fn fragment_main() {
  frexp_979800();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_979800();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_979800();
  return out;
}
