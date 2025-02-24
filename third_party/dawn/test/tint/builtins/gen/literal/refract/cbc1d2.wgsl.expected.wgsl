@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn refract_cbc1d2() -> vec3<f32> {
  var res : vec3<f32> = refract(vec3<f32>(1.0f), vec3<f32>(1.0f), 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = refract_cbc1d2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = refract_cbc1d2();
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
  out.prevent_dce = refract_cbc1d2();
  return out;
}
