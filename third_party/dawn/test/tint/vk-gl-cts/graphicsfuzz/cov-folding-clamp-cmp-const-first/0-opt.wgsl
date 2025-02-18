struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  f = 1.0;
  loop {
    let x_31 : f32 = x_6.one;
    let x_32 : f32 = f;
    f = (x_32 + x_31);

    continuing {
      let x_34 : f32 = f;
      let x_36 : f32 = x_6.one;
      break if !(10.0 > clamp(x_34, 8.0, (9.0 + x_36)));
    }
  }
  let x_40 : f32 = f;
  if ((x_40 == 10.0)) {
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
