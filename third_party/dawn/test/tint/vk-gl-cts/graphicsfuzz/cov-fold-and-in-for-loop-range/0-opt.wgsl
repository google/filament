struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> i32 {
  var ret : i32;
  var i : i32;
  ret = 0;
  i = 3;
  loop {
    let x_39 : i32 = i;
    let x_40 : i32 = i;
    if ((x_39 > (x_40 & 1))) {
    } else {
      break;
    }
    let x_44 : i32 = ret;
    ret = (x_44 + 1);

    continuing {
      let x_47 : i32 = x_8.one;
      let x_48 : i32 = i;
      i = (x_48 - x_47);
    }
  }
  let x_50 : i32 = ret;
  return x_50;
}

fn main_1() {
  let x_29 : i32 = func_();
  if ((x_29 == 2)) {
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
