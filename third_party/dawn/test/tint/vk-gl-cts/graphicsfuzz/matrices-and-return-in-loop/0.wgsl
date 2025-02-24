var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn f_mf22_(m : ptr<function, mat2x2<f32>>) -> vec3<f32> {
  loop {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
}

fn main_1() {
  var param : mat2x2<f32>;
  var x_38_phi : mat2x2<f32>;
  let x_34 : f32 = gl_FragCoord.x;
  x_38_phi = mat2x2<f32>(vec2<f32>(0.0, 0.0), vec2<f32>(0.0, 0.0));
  if ((x_34 >= 0.0)) {
    x_38_phi = mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0));
  }
  let x_38 : mat2x2<f32> = x_38_phi;
  param = (x_38 * x_38);
  let x_40 : vec3<f32> = f_mf22_(&(param));
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
