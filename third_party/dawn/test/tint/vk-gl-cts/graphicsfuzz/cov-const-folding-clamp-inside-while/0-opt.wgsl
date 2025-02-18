var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var j : i32;
  i = 0;
  j = 1;
  loop {
    let x_28 : i32 = i;
    let x_29 : i32 = j;
    if ((x_28 < clamp(x_29, 5, 9))) {
    } else {
      break;
    }
    let x_33 : i32 = i;
    i = (x_33 + 1);
    let x_35 : i32 = j;
    j = (x_35 + 1);
  }
  let x_37 : i32 = i;
  let x_39 : i32 = j;
  if (((x_37 == 9) & (x_39 == 10))) {
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
