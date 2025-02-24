var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var ll : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_30 : f32 = gl_FragCoord.x;
  if ((i32(x_30) < 2000)) {
  } else {
    ll = 0;
    loop {
      let x_41 : f32 = gl_FragCoord.x;
      if ((x_41 < 0.0)) {
        discard;
      }
      let x_6 : i32 = ll;
      if ((x_6 >= 5)) {
        break;
      }

      continuing {
        let x_7 : i32 = ll;
        ll = (x_7 + 1);
      }
    }
    let x_49 : f32 = gl_FragCoord.x;
    if ((i32(x_49) >= 2000)) {
      discard;
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
