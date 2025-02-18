enable f16;

fn frexp_3dd21e() {
  var res = frexp(vec4<f16>(1.0h));
}

@fragment
fn fragment_main() {
  frexp_3dd21e();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_3dd21e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_3dd21e();
  return out;
}
