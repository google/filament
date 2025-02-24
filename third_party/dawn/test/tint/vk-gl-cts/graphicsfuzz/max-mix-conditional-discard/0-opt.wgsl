struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> gv : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var temp : i32;
  let x_39 : f32 = x_5.injectionSwitch.x;
  if ((x_39 > 2.0)) {
    let x_8 : vec4<f32> = gv;
    temp = i32(max(mix(vec4<f32>(1.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0), x_8), vec4<f32>(8.600000381, 8.600000381, 8.600000381, 8.600000381)).y);
    let x_44 : i32 = temp;
    if ((x_44 < 150)) {
      discard;
    }
    let x_48 : i32 = temp;
    if ((x_48 < 180)) {
      discard;
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
