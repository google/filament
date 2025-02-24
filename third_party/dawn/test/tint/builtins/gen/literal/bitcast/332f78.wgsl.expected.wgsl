@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn bitcast_332f78() -> vec3<f32> {
  var res : vec3<f32> = bitcast<vec3<f32>>(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_332f78();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_332f78();
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
  out.prevent_dce = bitcast_332f78();
  return out;
}
