struct S {
  f0 : i32,
  f1 : mat4x3<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_51 : i32;
  var x_12_phi : i32;
  loop {
    var x_45 : S;
    var x_45_phi : S;
    var x_11_phi : i32;
    x_45_phi = S(0, mat4x3<f32>(vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(0.0, 0.0, 0.0)));
    x_11_phi = 0;
    loop {
      var x_46 : S;
      var x_9 : i32;
      x_45 = x_45_phi;
      let x_11 : i32 = x_11_phi;
      let x_49 : f32 = gl_FragCoord.x;
      x_51 = select(2, 1, (x_49 == 0.0));
      if ((x_11 < x_51)) {
      } else {
        break;
      }

      continuing {
        x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
        x_46 = x_45;
        x_46.f0 = (x_45.f0 + 1);
        x_9 = (x_11 + 1);
        x_45_phi = x_46;
        x_11_phi = x_9;
      }
    }
    if ((x_45.f0 < 1000)) {
      break;
    }
    break;
  }
  x_12_phi = 0;
  loop {
    var x_6 : i32;
    let x_12 : i32 = x_12_phi;
    if ((x_12 < x_51)) {
    } else {
      break;
    }

    continuing {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
      x_6 = (x_12 + 1);
      x_12_phi = x_6;
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
