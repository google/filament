struct buf0 {
  zeroOne : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec2<f32>;
  var d : f32;
  let x_37 : vec2<f32> = x_6.zeroOne;
  v = mix(vec2<f32>(2.0, 3.0), vec2<f32>(4.0, 5.0), x_37);
  let x_39 : vec2<f32> = v;
  d = distance(x_39, vec2<f32>(2.0, 5.0));
  let x_41 : f32 = d;
  if ((x_41 < 0.100000001)) {
    let x_47 : f32 = v.x;
    let x_50 : f32 = v.y;
    x_GLF_color = vec4<f32>((x_47 - 1.0), (x_50 - 5.0), 0.0, 1.0);
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
