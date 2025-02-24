struct buf0 {
  one : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var mixed : vec2<f32>;
  let x_30 : vec2<f32> = x_6.one;
  mixed = mix(vec2<f32>(1.0, 1.0), x_30, vec2<f32>(0.5, 0.5));
  let x_33 : vec2<f32> = mixed;
  if (all((x_33 == vec2<f32>(1.0, 1.0)))) {
    let x_40 : f32 = mixed.x;
    x_GLF_color = vec4<f32>(x_40, 0.0, 0.0, 1.0);
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
