var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec2<f32>;
  var b : vec2<f32>;
  a = vec2<f32>(1.0, 1.0);
  let x_25 : f32 = a.x;
  a.x = (x_25 + 0.5);
  let x_28 : vec2<f32> = a;
  b = fract(x_28);
  let x_31 : f32 = b.x;
  if ((x_31 == 0.5)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
