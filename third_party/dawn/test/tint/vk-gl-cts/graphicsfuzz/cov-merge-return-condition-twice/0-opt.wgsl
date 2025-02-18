struct buf0 {
  three : f32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var b : f32;
  var x_34 : f32;
  var x_34_phi : f32;
  var x_48_phi : f32;
  b = 2.0;
  x_34_phi = 2.0;
  loop {
    x_34 = x_34_phi;
    let x_39 : f32 = x_7.three;
    if ((x_39 == 0.0)) {
      x_48_phi = x_34;
      break;
    }
    let x_44 : f32 = x_7.three;
    if ((x_44 == 0.0)) {
      return 1.0;
    }
    b = 1.0;

    continuing {
      x_34_phi = 1.0;
      x_48_phi = 1.0;
      break if !(false);
    }
  }
  let x_48 : f32 = x_48_phi;
  return x_48;
}

fn main_1() {
  let x_27 : f32 = func_();
  if ((x_27 == 1.0)) {
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
