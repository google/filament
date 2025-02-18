var<private> x_GLF_color : vec4<f32>;

fn func_() -> vec3<f32> {
  var v : vec2<f32>;
  var i : i32;
  var k : i32;
  v = vec2<f32>(1.0, 1.0);
  i = 0;
  k = 0;
  loop {
    let x_90 : i32 = k;
    if ((x_90 < 2)) {
    } else {
      break;
    }
    let x_94 : f32 = v.y;
    if (((x_94 + 1.0) > 4.0)) {
      break;
    }
    v.y = 1.0;
    let x_100 : i32 = i;
    i = (x_100 + 1);

    continuing {
      let x_102 : i32 = k;
      k = (x_102 + 1);
    }
  }
  let x_104 : i32 = i;
  if ((x_104 < 10)) {
    return vec3<f32>(1.0, 0.0, 0.0);
  } else {
    return vec3<f32>(0.0, 0.0, 1.0);
  }
}

fn main_1() {
  var j : i32;
  var data : array<vec3<f32>, 2u>;
  var j_1 : i32;
  var x_80 : bool;
  var x_81_phi : bool;
  j = 0;
  loop {
    let x_49 : i32 = j;
    if ((x_49 < 1)) {
    } else {
      break;
    }
    let x_52 : i32 = j;
    let x_53 : vec3<f32> = func_();
    data[x_52] = x_53;

    continuing {
      let x_55 : i32 = j;
      j = (x_55 + 1);
    }
  }
  j_1 = 0;
  loop {
    let x_61 : i32 = j_1;
    if ((x_61 < 1)) {
    } else {
      break;
    }
    let x_64 : i32 = j_1;
    let x_67 : vec3<f32> = func_();
    data[((4 * x_64) + 1)] = x_67;

    continuing {
      let x_69 : i32 = j_1;
      j_1 = (x_69 + 1);
    }
  }
  let x_72 : vec3<f32> = data[0];
  let x_74 : bool = all((x_72 == vec3<f32>(1.0, 0.0, 0.0)));
  x_81_phi = x_74;
  if (x_74) {
    let x_78 : vec3<f32> = data[1];
    x_80 = all((x_78 == vec3<f32>(1.0, 0.0, 0.0)));
    x_81_phi = x_80;
  }
  let x_81 : bool = x_81_phi;
  if (x_81) {
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
