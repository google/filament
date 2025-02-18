struct buf0 {
  three : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  a = 0;
  i = 0;
  loop {
    let x_31 : i32 = i;
    let x_33 : i32 = x_7.three;
    if ((x_31 < (7 + x_33))) {
    } else {
      break;
    }
    let x_37 : i32 = i;
    switch(x_37) {
      case 7, 8: {
        let x_40 : i32 = a;
        a = (x_40 + 1);
      }
      default: {
      }
    }

    continuing {
      let x_42 : i32 = i;
      i = (x_42 + 1);
    }
  }
  let x_44 : i32 = a;
  if ((x_44 == 2)) {
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
