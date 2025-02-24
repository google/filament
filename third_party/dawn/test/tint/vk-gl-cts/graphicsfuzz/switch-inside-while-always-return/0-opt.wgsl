struct buf0 {
  zero : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn merge_() -> i32 {
  let x_40 : f32 = x_6.zero;
  if ((x_40 < 0.0)) {
    return 0;
  }
  return 0;
}

fn main_1() {
  var res : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  loop {
    let x_32 : f32 = x_6.zero;
    switch(i32(x_32)) {
      case 0: {
        return;
      }
      default: {
      }
    }

    continuing {
      break if !(false);
    }
  }
  let x_8 : i32 = merge_();
  res = x_8;
  let x_9 : i32 = res;
  let x_36 : f32 = f32(x_9);
  x_GLF_color = vec4<f32>(x_36, x_36, x_36, x_36);
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
