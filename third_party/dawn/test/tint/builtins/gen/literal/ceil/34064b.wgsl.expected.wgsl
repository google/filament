@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn ceil_34064b() -> vec3<f32> {
  var res : vec3<f32> = ceil(vec3<f32>(1.5f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = ceil_34064b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = ceil_34064b();
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
  out.prevent_dce = ceil_34064b();
  return out;
}
