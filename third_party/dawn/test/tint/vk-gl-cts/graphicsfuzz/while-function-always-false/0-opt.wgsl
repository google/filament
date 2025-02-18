struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var j : i32;
  var x_41 : f32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  loop {
    let x_32 : f32 = x_6.injectionSwitch.x;
    j = i32(x_32);
    loop {
      let x_8 : i32 = j;
      if ((x_8 < 2)) {
      } else {
        break;
      }
      return;
    }

    continuing {
      x_41 = x_6.injectionSwitch.y;
      break if !(0.0 > x_41);
    }
  }
  let x_43 : i32 = i32(x_41);
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
