struct buf0 {
  zero : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var x_9 : array<i32, 1u>;
  var x_10_phi : i32;
  let x_33 : array<i32, 1u> = x_9;
  let x_6 : i32 = x_33[0u];
  loop {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    let x_7 : i32 = x_5.zero;
    let x_8 : i32 = x_9[x_7];
    if ((x_8 == x_6)) {
      x_10_phi = 1;
      break;
    }
    x_10_phi = 2;
    break;
  }
  let x_10 : i32 = x_10_phi;
  if (((x_10 == 1) | (x_10 == 2))) {
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
