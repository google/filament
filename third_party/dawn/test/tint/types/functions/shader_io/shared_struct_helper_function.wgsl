struct VertexOutput {
  @builtin(position) pos : vec4<f32>,
  @location(0) @interpolate(flat) loc0 : i32,
};

fn foo(x : f32) -> VertexOutput {
  return VertexOutput(vec4<f32>(x, x, x, 1.0), 42);
}

@vertex
fn vert_main1() -> VertexOutput {
  return foo(0.5);
}

@vertex
fn vert_main2() -> VertexOutput {
  return foo(0.25);
}
