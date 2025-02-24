@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn countTrailingZeros_1dc84a() -> vec4<i32> {
  var res : vec4<i32> = countTrailingZeros(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = countTrailingZeros_1dc84a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = countTrailingZeros_1dc84a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = countTrailingZeros_1dc84a();
  return out;
}
