struct buf0 {
  two : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

fn main_1() {
  var a : i32;
  var i : i32;
  a = 0;
  i = 0;
  loop {
    let x_30 : i32 = i;
    if ((x_30 < 2)) {
    } else {
      break;
    }
    let x_33 : i32 = i;
    a = ((x_33 | -2) - 1);

    continuing {
      let x_36 : i32 = i;
      i = (x_36 + 1);
    }
  }
  let x_38 : i32 = a;
  if ((x_38 == -2)) {
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
