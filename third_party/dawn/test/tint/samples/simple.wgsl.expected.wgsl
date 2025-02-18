fn bar() {
}

@fragment
fn main() -> @location(0) vec4<f32> {
  var a : vec2<f32> = vec2<f32>();
  bar();
  return vec4<f32>(0.4000000000000000222, 0.4000000000000000222, 0.80000000000000004441, 1.0);
}
