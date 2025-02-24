var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var GLF_live15c : vec4<f32>;
  var GLF_live15i : i32;
  var GLF_live15d : vec4<f32>;
  var GLF_live15i_1 : i32;
  GLF_live15c = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  GLF_live15i = 0;
  loop {
    let x_8 : i32 = GLF_live15i;
    if ((x_8 < 4)) {
    } else {
      break;
    }
    let x_9 : i32 = GLF_live15i;
    if ((x_9 >= 3)) {
      break;
    }
    let x_49 : f32 = GLF_live15c.y;
    if ((x_49 >= 1.0)) {
      let x_10 : i32 = GLF_live15i;
      GLF_live15c[x_10] = 1.0;
    }

    continuing {
      let x_11 : i32 = GLF_live15i;
      GLF_live15i = (x_11 + 1);
    }
  }
  GLF_live15d = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  GLF_live15i_1 = 0;
  loop {
    let x_13 : i32 = GLF_live15i_1;
    if ((x_13 < 4)) {
    } else {
      break;
    }
    let x_14 : i32 = GLF_live15i_1;
    if ((x_14 >= 3)) {
      break;
    }
    let x_64 : f32 = GLF_live15d.y;
    if ((x_64 >= 1.0)) {
      let x_15 : i32 = GLF_live15i_1;
      GLF_live15d[x_15] = 1.0;
    }

    continuing {
      let x_16 : i32 = GLF_live15i_1;
      GLF_live15i_1 = (x_16 + 1);
    }
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
