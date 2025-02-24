struct buf0 {
  two : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var r : i32;
  i = 0;
  r = 0;
  loop {
    let x_35 : i32 = r;
    let x_37 : i32 = x_7.two;
    if ((x_35 < (x_37 * 4))) {
    } else {
      break;
    }
    let x_41 : i32 = r;
    let x_43 : i32 = x_7.two;
    let x_46 : i32 = i;
    i = (x_46 + vec4<i32>(1, 2, 3, 4)[(x_41 / x_43)]);

    continuing {
      let x_48 : i32 = r;
      r = (x_48 + 2);
    }
  }
  let x_50 : i32 = i;
  if ((x_50 == 10)) {
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
