enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn smoothstep_586e12() -> f16 {
  var arg_0 = 2.0h;
  var arg_1 = 4.0h;
  var arg_2 = 3.0h;
  var res : f16 = smoothstep(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = smoothstep_586e12();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = smoothstep_586e12();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = smoothstep_586e12();
  return out;
}
