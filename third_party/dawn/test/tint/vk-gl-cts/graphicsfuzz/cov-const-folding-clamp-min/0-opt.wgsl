struct buf0 {
  zero : f32,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  let x_25 : f32 = x_5.zero;
  let x_28 : f32 = x_5.zero;
  if (any((vec4<f32>(clamp(2.0, x_25, 1.0), clamp(-1.0, 0.0, x_28), 0.0, 1.0) != vec4<f32>(1.0, 0.0, 0.0, 1.0)))) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
