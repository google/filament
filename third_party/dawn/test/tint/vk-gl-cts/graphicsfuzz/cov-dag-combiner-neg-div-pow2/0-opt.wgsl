struct buf1 {
  ten : i32,
}

struct buf0 {
  minusEight : i32,
}

@group(0) @binding(1) var<uniform> x_8 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var b : i32;
  var i : i32;
  a = 0;
  b = 0;
  i = 0;
  loop {
    let x_36 : i32 = i;
    let x_38 : i32 = x_8.ten;
    if ((x_36 < x_38)) {
    } else {
      break;
    }
    let x_41 : i32 = a;
    if ((x_41 > 5)) {
      break;
    }
    let x_46 : i32 = x_10.minusEight;
    let x_48 : i32 = a;
    a = (x_48 + (x_46 / -4));
    let x_50 : i32 = b;
    b = (x_50 + 1);

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  let x_54 : i32 = b;
  if ((x_54 == 3)) {
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
