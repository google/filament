struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  let x_25 : f32 = x_5.injectionSwitch.y;
  switch(i32(x_25)) {
    case -1: {
      let x_30 : f32 = x_5.injectionSwitch.y;
      let x_32 : f32 = x_5.injectionSwitch.x;
      if ((x_30 > x_32)) {
        discard;
      }
    }
    default: {
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
