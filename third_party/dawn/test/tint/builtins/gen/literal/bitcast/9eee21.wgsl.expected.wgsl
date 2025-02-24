@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn bitcast_9eee21() -> vec3<i32> {
  var res : vec3<i32> = bitcast<vec3<i32>>(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_9eee21();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_9eee21();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_9eee21();
  return out;
}
