@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn min_93cfc4() -> vec3<f32> {
  var res : vec3<f32> = min(vec3<f32>(1.0f), vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = min_93cfc4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = min_93cfc4();
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
  out.prevent_dce = min_93cfc4();
  return out;
}
