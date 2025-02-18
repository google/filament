var<private> x_GLF_color : vec4<f32>;

fn func_() -> vec3<f32> {
  var v : vec2<f32>;
  var i : i32;
  var k : i32;
  v = vec2<f32>(1.0, 1.0);
  i = 0;
  k = 0;
  loop {
    let x_79 : i32 = k;
    if ((x_79 < 2)) {
    } else {
      break;
    }
    let x_83 : f32 = v.y;
    if (((x_83 + 1.0) > 4.0)) {
      break;
    }
    v.y = 1.0;
    let x_89 : i32 = i;
    i = (x_89 + 1);

    continuing {
      let x_91 : i32 = k;
      k = (x_91 + 1);
    }
  }
  let x_93 : i32 = i;
  if ((x_93 < 10)) {
    return vec3<f32>(1.0, 0.0, 0.0);
  } else {
    return vec3<f32>(0.0, 0.0, 1.0);
  }
}

fn main_1() {
  var j : i32;
  var data : array<vec3<f32>, 2u>;
  var j_1 : i32;
  j = 0;
  loop {
    let x_46 : i32 = j;
    if ((x_46 < 1)) {
    } else {
      break;
    }
    let x_49 : i32 = j;
    let x_50 : vec3<f32> = func_();
    data[x_49] = x_50;

    continuing {
      let x_52 : i32 = j;
      j = (x_52 + 1);
    }
  }
  j_1 = 0;
  loop {
    let x_58 : i32 = j_1;
    if ((x_58 < 1)) {
    } else {
      break;
    }
    let x_61 : i32 = j_1;
    let x_64 : vec3<f32> = func_();
    data[((4 * x_61) + 1)] = x_64;

    continuing {
      let x_66 : i32 = j_1;
      j_1 = (x_66 + 1);
    }
  }
  let x_69 : vec3<f32> = data[0];
  x_GLF_color = vec4<f32>(x_69.x, x_69.y, x_69.z, 1.0);
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
