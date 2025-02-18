struct buf0 {
  threeandfour : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  v = vec4<f32>(2.0, 3.0, 4.0, 5.0);
  let x_40 : f32 = x_6.threeandfour.y;
  let x_42 : vec2<f32> = select(vec2<f32>(2.0, 6.0), vec2<f32>(1.0, x_40), vec2<bool>(true, false));
  let x_43 : vec4<f32> = v;
  v = vec4<f32>(x_42.x, x_42.y, x_43.z, x_43.w);
  let x_45 : vec4<f32> = v;
  if (all((x_45 == vec4<f32>(1.0, 6.0, 4.0, 5.0)))) {
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
