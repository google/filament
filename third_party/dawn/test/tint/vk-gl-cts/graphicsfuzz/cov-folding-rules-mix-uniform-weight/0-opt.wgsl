struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec3<f32>;
  var d : f32;
  let x_36 : f32 = x_6.one;
  v = mix(vec3<f32>(5.0, 8.0, -12.199999809), vec3<f32>(1.0, 4.900000095, -2.099999905), vec3<f32>(x_36, x_36, x_36));
  let x_39 : vec3<f32> = v;
  d = distance(x_39, vec3<f32>(1.0, 4.900000095, -2.099999905));
  let x_41 : f32 = d;
  if ((x_41 < 0.100000001)) {
    let x_47 : f32 = v.x;
    x_GLF_color = vec4<f32>(x_47, 0.0, 0.0, 1.0);
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
