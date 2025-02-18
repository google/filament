struct buf0 {
  minusOne : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var minValue : i32;
  var negMinValue : i32;
  minValue = -2147483648;
  let x_25 : i32 = minValue;
  negMinValue = -(x_25);
  let x_27 : i32 = negMinValue;
  let x_28 : i32 = minValue;
  let x_30 : i32 = x_7.minusOne;
  if ((x_27 == (x_28 * x_30))) {
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
