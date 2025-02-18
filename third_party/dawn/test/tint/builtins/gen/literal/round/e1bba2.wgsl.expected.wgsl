enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn round_e1bba2() -> vec3<f16> {
  var res : vec3<f16> = round(vec3<f16>(3.5h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = round_e1bba2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = round_e1bba2();
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
  out.prevent_dce = round_e1bba2();
  return out;
}
