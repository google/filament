struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  a = 0;
  i = 0;
  loop {
    let x_33 : i32 = i;
    let x_35 : f32 = x_7.injectionSwitch.y;
    if ((x_33 < i32(x_35))) {
    } else {
      break;
    }
    let x_39 : i32 = a;
    if ((x_39 > 0)) {
      break;
    }
    let x_44 : f32 = x_7.injectionSwitch.y;
    a = ((i32(x_44) * 2) / 2);

    continuing {
      let x_48 : i32 = i;
      i = (x_48 + 1);
    }
  }
  let x_50 : i32 = a;
  if ((x_50 == 1)) {
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
