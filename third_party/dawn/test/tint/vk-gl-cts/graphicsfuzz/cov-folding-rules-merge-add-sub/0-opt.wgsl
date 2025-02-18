var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  i = 5;
  loop {
    let x_5 : i32 = i;
    if ((x_5 >= 0)) {
    } else {
      break;
    }
    let x_6 : i32 = i;
    i = (x_6 - 3);
    let x_8 : i32 = i;
    i = (x_8 + 1);
  }
  let x_10 : i32 = i;
  if ((x_10 == -1)) {
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
