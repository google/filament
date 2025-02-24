struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  var res : vec4<f32>;
  v = vec4<f32>(8.399999619, -864.664978027, 945.41998291, 1.0);
  let x_31 : f32 = x_7.one;
  let x_37 : vec4<f32> = v;
  res = (mat4x4<f32>(vec4<f32>(x_31, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_31, 0.0, 0.0), vec4<f32>(0.0, 0.0, x_31, 0.0), vec4<f32>(0.0, 0.0, 0.0, x_31)) * x_37);
  let x_39 : vec4<f32> = v;
  let x_40 : vec4<f32> = res;
  if ((distance(x_39, x_40) < 0.01)) {
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
