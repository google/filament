struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_i1_(x : ptr<function, i32>) -> i32 {
  let x_35 : i32 = x_7.one;
  if ((x_35 == 1)) {
    let x_39 : i32 = *(x);
    return x_39;
  }
  let x_41 : i32 = x_7.one;
  return x_41;
}

fn main_1() {
  var param : i32;
  param = -1;
  let x_28 : i32 = func_i1_(&(param));
  if ((x_28 <= 0)) {
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
