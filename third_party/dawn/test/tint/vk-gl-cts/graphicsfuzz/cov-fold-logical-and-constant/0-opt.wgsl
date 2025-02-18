var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  i = 2;
  loop {
    let x_6 : i32 = i;
    i = (x_6 + 1);

    continuing {
      let x_35 : f32 = gl_FragCoord.x;
      break if !((x_35 >= 0.0) & false);
    }
  }
  let x_8 : i32 = i;
  if ((x_8 == 3)) {
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
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
