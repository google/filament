@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn max_b1b73a() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var arg_1 = vec3<u32>(1u);
  var res : vec3<u32> = max(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = max_b1b73a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = max_b1b73a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = max_b1b73a();
  return out;
}
