struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  let x_28 : f32 = x_6.one;
  f = (4.0 * (2.0 / x_28));
  let x_31 : f32 = f;
  let x_33 : f32 = f;
  if (((x_31 > 7.900000095) & (x_33 < 8.100000381))) {
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
