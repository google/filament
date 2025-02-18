@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn ldexp_593ff3() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  const arg_1 = vec3(1);
  var res : vec3<f32> = ldexp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = ldexp_593ff3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = ldexp_593ff3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = ldexp_593ff3();
  return out;
}
