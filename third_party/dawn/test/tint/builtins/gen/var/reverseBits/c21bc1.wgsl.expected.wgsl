@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn reverseBits_c21bc1() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = reverseBits(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = reverseBits_c21bc1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = reverseBits_c21bc1();
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
  out.prevent_dce = reverseBits_c21bc1();
  return out;
}
