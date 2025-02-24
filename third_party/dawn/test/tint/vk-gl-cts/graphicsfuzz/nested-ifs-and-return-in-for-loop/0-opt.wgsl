struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var i : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i = 0;
  loop {
    let x_7 : i32 = i;
    if ((x_7 < 10)) {
    } else {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    let x_39 : f32 = x_6.injectionSwitch.y;
    if ((1.0 > x_39)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
      if (true) {
        return;
      }
    }

    continuing {
      let x_8 : i32 = i;
      i = (x_8 + 1);
    }
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
