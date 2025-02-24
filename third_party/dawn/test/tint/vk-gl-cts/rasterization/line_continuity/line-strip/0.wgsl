var<private> color : vec4<f32>;

fn main_1() {
  color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  return;
}

struct main_out {
  @location(0)
  color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(color);
}
