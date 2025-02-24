@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn select_51b047() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var arg_1 = vec2<u32>(1u);
  var arg_2 = true;
  var res : vec2<u32> = select(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_51b047();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_51b047();
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
  out.prevent_dce = select_51b047();
  return out;
}
