fn vec4f() -> i32 {
  return 0;
}

fn vec2f(i : i32) -> f32 {
  return f32(i);
}

fn vec2i(f : f32) -> bool {
  return bool(f);
}

@vertex
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
  return select(vec4<f32>(), vec4<f32>(1), vec2i(vec2f(vec4f())));
}
