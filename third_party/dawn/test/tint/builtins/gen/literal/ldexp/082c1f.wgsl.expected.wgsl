enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn ldexp_082c1f() -> f16 {
  var res : f16 = ldexp(1.0h, 1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = ldexp_082c1f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = ldexp_082c1f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = ldexp_082c1f();
  return out;
}
