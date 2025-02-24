@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn countTrailingZeros_1ad138() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = countTrailingZeros(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = countTrailingZeros_1ad138();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = countTrailingZeros_1ad138();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = countTrailingZeros_1ad138();
  return out;
}
