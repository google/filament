struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var height : f32;
  height = 256.0;
  let x_40 : f32 = x_6.injectionSwitch.y;
  if ((x_40 < 0.0)) {
    let x_44 : f32 = height;
    x_GLF_color = mix(vec4<f32>(30.180000305, 8840.0, 469.970001221, 18.239999771), vec4<f32>(9.899999619, 0.100000001, 1169.538696289, 55.790000916), vec4<f32>(7612.9453125, 797.010986328, x_44, 9.0));
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
