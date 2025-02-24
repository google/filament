@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn select_416e14() -> f32 {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var arg_2 = true;
  var res : f32 = select(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_416e14();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_416e14();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = select_416e14();
  return out;
}
