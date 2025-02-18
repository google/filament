enable f16;

fn modf_45005f() {
  var res = modf(vec3<f16>(-(1.5h)));
}

@fragment
fn fragment_main() {
  modf_45005f();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_45005f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_45005f();
  return out;
}
