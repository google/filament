struct buf0 {
  four : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  let x_27 : f32 = x_6.four;
  a = (2.0 / (1.0 / x_27));
  let x_30 : f32 = a;
  let x_32 : f32 = a;
  if (((x_30 > 7.900000095) & (x_32 < 8.100000381))) {
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
