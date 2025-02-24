struct buf0 {
  five : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  let x_26 : i32 = x_6.five;
  i = x_26;
  loop {
    let x_31 : i32 = i;
    if ((x_31 > 0)) {
    } else {
      break;
    }
    let x_34 : i32 = i;
    i = (x_34 - 1);
    let x_36 : i32 = i;
    i = (x_36 - 1);
  }
  let x_38 : i32 = i;
  if ((x_38 == -1)) {
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
