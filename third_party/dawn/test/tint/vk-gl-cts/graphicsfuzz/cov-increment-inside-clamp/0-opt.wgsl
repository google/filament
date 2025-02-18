struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : array<i32, 3u>;
  var b : i32;
  var c : i32;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  b = 0;
  let x_38 : i32 = x_8.one;
  let x_40 : i32 = a[x_38];
  c = x_40;
  let x_41 : i32 = c;
  if ((x_41 > 1)) {
    x_GLF_color = vec4<f32>(0.0, 1.0, 1.0, 0.0);
    let x_45 : i32 = b;
    b = (x_45 + 1);
  }
  let x_47 : i32 = b;
  let x_48 : i32 = (x_47 + 1);
  b = x_48;
  let x_50_save = clamp(x_48, 0, 2);
  let x_51 : i32 = a[x_50_save];
  a[x_50_save] = (x_51 + 1);
  let x_54 : i32 = a[2];
  if ((x_54 == 4)) {
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
