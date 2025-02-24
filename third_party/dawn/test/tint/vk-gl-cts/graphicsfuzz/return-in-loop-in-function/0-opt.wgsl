var<private> x_GLF_color : vec4<f32>;

fn f_() -> f32 {
  var i : i32;
  i = 1;
  loop {
    let x_8 : i32 = i;
    if ((x_8 < 10)) {
    } else {
      break;
    }
    let x_9 : i32 = i;
    if ((f32(x_9) >= 1.0)) {
      return 1.0;
    } else {
      continue;
    }

    continuing {
      let x_10 : i32 = i;
      i = (x_10 + 1);
    }
  }
  return 1.0;
}

fn main_1() {
  var c : vec4<f32>;
  var i_1 : i32;
  c = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i_1 = 0;
  loop {
    let x_12 : i32 = i_1;
    if ((x_12 < 1)) {
    } else {
      break;
    }

    continuing {
      let x_39 : f32 = f_();
      c.x = x_39;
      let x_13 : i32 = i_1;
      i_1 = (x_13 + 1);
    }
  }
  let x_41 : vec4<f32> = c;
  x_GLF_color = x_41;
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
