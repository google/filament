var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var data : array<f32, 10u>;
  var i : i32;
  data = array<f32, 10u>(0.100000001, 0.200000003, 0.300000012, 0.400000006, 0.5, 0.600000024, 0.699999988, 0.800000012, 0.899999976, 1.0);
  i = 0;
  loop {
    let x_7 : i32 = i;
    if ((x_7 < 10)) {
    } else {
      break;
    }
    let x_50 : f32 = gl_FragCoord.x;
    if ((x_50 < 0.0)) {
      discard;
    }
    let x_8 : i32 = i;
    let x_55 : f32 = data[x_8];
    data[0] = x_55;

    continuing {
      let x_9 : i32 = i;
      i = (x_9 + 1);
    }
  }
  let x_58 : f32 = data[0];
  x_GLF_color = vec4<f32>(x_58, 0.0, 0.0, 1.0);
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
