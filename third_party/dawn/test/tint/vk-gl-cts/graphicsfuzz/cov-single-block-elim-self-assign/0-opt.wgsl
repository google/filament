struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> g : i32;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  g = 0;
  loop {
    let x_8 : i32 = g;
    let x_46 : f32 = x_6.injectionSwitch.x;
    if ((x_8 < i32((x_46 + 2.0)))) {
    } else {
      break;
    }
    let x_9 : i32 = g;
    g = (x_9 + 1);
  }
  let x_11 : i32 = g;
  a = x_11;
  loop {
    let x_12 : i32 = g;
    let x_56 : f32 = x_6.injectionSwitch.y;
    if ((x_12 < i32(x_56))) {
    } else {
      break;
    }
    let x_13 : i32 = g;
    g = (x_13 + 1);
  }
  let x_15 : i32 = a;
  a = x_15;
  let x_16 : i32 = a;
  if ((x_16 == 2)) {
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
