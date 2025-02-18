var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var j : i32;
  var a : f32;
  j = 0;
  loop {
    let x_6 : i32 = j;
    if ((x_6 < 2)) {
    } else {
      break;
    }
    let x_7 : i32 = j;
    if ((x_7 < 1)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    }
    let x_8 : i32 = j;
    if ((x_8 != 3)) {
      let x_9 : i32 = j;
      if ((x_9 != 4)) {
        let x_10 : i32 = j;
        if ((x_10 == 5)) {
          x_GLF_color.x = ldexp(1.0, 2);
        } else {
          a = ldexp(1.0, 2);
          x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
      }
    }

    continuing {
      let x_11 : i32 = j;
      j = (x_11 + 1);
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
