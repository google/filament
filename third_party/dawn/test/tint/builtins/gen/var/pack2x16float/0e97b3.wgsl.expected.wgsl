@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn pack2x16float_0e97b3() -> u32 {
  var arg_0 = vec2<f32>(1.0f);
  var res : u32 = pack2x16float(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = pack2x16float_0e97b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pack2x16float_0e97b3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = pack2x16float_0e97b3();
  return out;
}
