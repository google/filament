@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn countOneBits_65d2ae() -> vec3<i32> {
  var res : vec3<i32> = countOneBits(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = countOneBits_65d2ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = countOneBits_65d2ae();
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
  out.prevent_dce = countOneBits_65d2ae();
  return out;
}
