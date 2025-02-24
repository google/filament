var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  if (true) {
    let x_23 : f32 = gl_FragCoord.x;
    if ((x_23 < 0.0)) {
      loop {
        x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);

        continuing {
          let x_32 : f32 = gl_FragCoord.x;
          break if !(x_32 < 0.0);
        }
      }
    }
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
