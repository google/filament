enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn sinh_0908c1() -> vec3<f16> {
  var res : vec3<f16> = sinh(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = sinh_0908c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = sinh_0908c1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = sinh_0908c1();
  return out;
}
