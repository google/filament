fn exp2_18aa76() {
  var res = exp2(vec2(1.0));
}

@fragment
fn fragment_main() {
  exp2_18aa76();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_18aa76();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp2_18aa76();
  return out;
}
