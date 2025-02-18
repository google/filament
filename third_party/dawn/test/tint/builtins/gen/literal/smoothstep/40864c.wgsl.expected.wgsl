@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn smoothstep_40864c() -> vec4<f32> {
  var res : vec4<f32> = smoothstep(vec4<f32>(2.0f), vec4<f32>(4.0f), vec4<f32>(3.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = smoothstep_40864c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = smoothstep_40864c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = smoothstep_40864c();
  return out;
}
