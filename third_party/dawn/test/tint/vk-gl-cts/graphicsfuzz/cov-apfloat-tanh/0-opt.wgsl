var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  var dist1 : f32;
  var dist2 : f32;
  v = vec4<f32>(1.0, 2.0, 3.0, 4.0);
  let x_30 : vec4<f32> = v;
  let x_32 : vec4<f32> = v;
  let x_34 : vec4<f32> = v;
  dist1 = distance(tanh(x_30), (sinh(x_32) / cosh(x_34)));
  let x_38 : vec4<f32> = v;
  dist2 = distance(tanh(x_38), vec4<f32>(0.761590004, 0.964030027, 0.995050013, 0.999329984));
  let x_41 : f32 = dist1;
  let x_43 : f32 = dist2;
  if (((x_41 < 0.100000001) & (x_43 < 0.100000001))) {
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
