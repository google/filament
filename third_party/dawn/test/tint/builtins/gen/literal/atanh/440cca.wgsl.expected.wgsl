@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn atanh_440cca() -> vec3<f32> {
  var res : vec3<f32> = atanh(vec3<f32>(0.5f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atanh_440cca();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atanh_440cca();
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
  out.prevent_dce = atanh_440cca();
  return out;
}
