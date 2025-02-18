enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn acos_f47057() -> vec3<f16> {
  var res : vec3<f16> = acos(vec3<f16>(0.96875h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = acos_f47057();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = acos_f47057();
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
  out.prevent_dce = acos_f47057();
  return out;
}
