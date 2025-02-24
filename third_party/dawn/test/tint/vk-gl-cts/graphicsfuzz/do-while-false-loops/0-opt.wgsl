var<private> x_GLF_color : vec4<f32>;

fn f_() -> vec3<f32> {
  var iteration : i32;
  var k : i32;
  iteration = 0;
  k = 0;
  loop {
    let x_7 : i32 = k;
    if ((x_7 < 100)) {
    } else {
      break;
    }
    let x_8 : i32 = iteration;
    iteration = (x_8 + 1);

    continuing {
      let x_10 : i32 = k;
      k = (x_10 + 1);
    }
  }
  let x_12 : i32 = iteration;
  if ((x_12 < 100)) {
    let x_13 : i32 = iteration;
    let x_15 : i32 = iteration;
    return vec3<f32>(1.0, f32((x_13 - 1)), f32((x_15 - 1)));
  } else {
    loop {
      loop {
        return vec3<f32>(1.0, 0.0, 0.0);
      }
    }
  }
}

fn main_1() {
  let x_35 : vec3<f32> = f_();
  x_GLF_color = vec4<f32>(x_35.x, x_35.y, x_35.z, 1.0);
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
