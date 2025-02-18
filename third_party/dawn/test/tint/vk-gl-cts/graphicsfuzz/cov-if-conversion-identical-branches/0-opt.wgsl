struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_25 : i32 = x_6.zero;
  a = x_25;
  let x_26 : i32 = a;
  if ((x_26 == 0)) {
    let x_31 : i32 = a;
    a = (x_31 + 1);
  } else {
    let x_33 : i32 = a;
    a = (x_33 + 1);
  }
  let x_35 : i32 = a;
  if ((x_35 == 1)) {
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
