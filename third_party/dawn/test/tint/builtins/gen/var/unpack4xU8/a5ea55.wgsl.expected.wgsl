@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn unpack4xU8_a5ea55() -> vec4<u32> {
  var arg_0 = 1u;
  var res : vec4<u32> = unpack4xU8(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = unpack4xU8_a5ea55();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = unpack4xU8_a5ea55();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = unpack4xU8_a5ea55();
  return out;
}
