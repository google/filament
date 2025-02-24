var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var b : bool;
  var i : i32;
  var a : f32;
  b = false;
  i = 1;
  loop {
    let x_7 : i32 = i;
    if ((x_7 > 0)) {
    } else {
      break;
    }
    let x_8 : i32 = i;
    a = (3.0 - f32(x_8));
    let x_40 : f32 = a;
    if (((2.0 - x_40) == 0.0)) {
      b = true;
    }

    continuing {
      let x_9 : i32 = i;
      i = (x_9 - 1);
    }
  }
  let x_45 : bool = b;
  if (x_45) {
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
