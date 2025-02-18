var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var i : i32;
  i = 0;
  loop {
    let x_35 : i32 = i;
    if ((x_35 < 10)) {
    } else {
      break;
    }
    let x_38 : i32 = i;
    if ((x_38 > 5)) {
      let x_42 : i32 = i;
      i = (x_42 + 1);
    }
    let x_44 : i32 = i;
    if ((x_44 > 8)) {
      return 0.0;
    }

    continuing {
      let x_48 : i32 = i;
      i = (x_48 + 1);
    }
  }
  return 1.0;
}

fn main_1() {
  if (false) {
    let x_28 : f32 = func_();
    x_GLF_color = vec4<f32>(x_28, x_28, x_28, x_28);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
