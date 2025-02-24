struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() {
  let x_28 : f32 = x_6.one;
  if ((1.0 > x_28)) {
    discard;
  }
  return;
}

fn main_1() {
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  loop {
    func_();
    if (false) {
    } else {
      break;
    }
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
