struct buf0 {
  twoandthree : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec2<f32>;
  var b : vec2<f32>;
  var x_46 : bool;
  var x_47_phi : bool;
  let x_32 : vec2<f32> = x_6.twoandthree;
  a = x_32;
  let x_34 : f32 = a.x;
  let x_35 : vec2<f32> = a;
  b = vec2<f32>(x_34, clamp(x_35, vec2<f32>(1.0, 1.0), vec2<f32>(1.0, 1.0)).y);
  let x_40 : f32 = b.x;
  let x_41 : bool = (x_40 == 2.0);
  x_47_phi = x_41;
  if (x_41) {
    let x_45 : f32 = b.y;
    x_46 = (x_45 == 1.0);
    x_47_phi = x_46;
  }
  let x_47 : bool = x_47_phi;
  if (x_47) {
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
