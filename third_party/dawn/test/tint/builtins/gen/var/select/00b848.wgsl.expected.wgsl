@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn select_00b848() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = vec2<i32>(1i);
  var arg_2 = vec2<bool>(true);
  var res : vec2<i32> = select(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_00b848();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_00b848();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = select_00b848();
  return out;
}
