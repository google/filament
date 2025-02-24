struct Array {
  values : array<i32, 2u>,
}

struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : Array;
  var b : Array;
  var one : f32;
  let x_10 : i32 = x_7.zero;
  a.values[x_10] = 1;
  let x_35 : Array = a;
  b = x_35;
  one = 0.0;
  let x_11 : i32 = x_7.zero;
  let x_12 : i32 = b.values[x_11];
  if ((x_12 == 1)) {
    one = 1.0;
  }
  let x_41 : f32 = one;
  x_GLF_color = vec4<f32>(x_41, 0.0, 0.0, 1.0);
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
