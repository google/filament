var<private> data : array<i32, 9u>;

var<private> gl_FragCoord : vec4<f32>;

var<private> temp : array<i32, 7u>;

var<private> x_GLF_color : vec4<f32>;

fn func_i1_(a : ptr<function, i32>) -> f32 {
  var b : i32;
  var i : i32;
  var x_115 : bool;
  var x_116_phi : bool;
  b = 0;
  data[0] = 5;
  data[2] = 0;
  data[4] = 0;
  data[6] = 0;
  data[8] = 0;
  let x_71 : f32 = gl_FragCoord.x;
  if ((x_71 >= 0.0)) {
    loop {
      let x_79 : i32 = b;
      let x_80 : i32 = *(a);
      if ((x_79 <= x_80)) {
      } else {
        break;
      }
      let x_83 : i32 = b;
      if ((x_83 <= 5)) {
        let x_87 : i32 = b;
        let x_88 : i32 = b;
        let x_90 : i32 = data[x_88];
        temp[x_87] = x_90;
        let x_92 : i32 = b;
        b = (x_92 + 2);
      }
    }
  }
  i = 0;
  loop {
    let x_98 : i32 = i;
    if ((x_98 < 3)) {
    } else {
      break;
    }
    let x_101 : i32 = i;
    let x_103 : i32 = temp[0];
    data[x_101] = (x_103 + 1);

    continuing {
      let x_106 : i32 = i;
      i = (x_106 + 1);
    }
  }
  let x_109 : i32 = temp[0];
  let x_110 : bool = (x_109 == 5);
  x_116_phi = x_110;
  if (x_110) {
    let x_114 : i32 = data[0];
    x_115 = (x_114 == 6);
    x_116_phi = x_115;
  }
  let x_116 : bool = x_116_phi;
  if (x_116) {
    return 1.0;
  } else {
    return 0.0;
  }
}

fn main_1() {
  var i_1 : i32;
  var param : i32;
  var param_1 : i32;
  i_1 = 0;
  loop {
    let x_51 : i32 = i_1;
    if ((x_51 < 6)) {
    } else {
      break;
    }
    let x_54 : i32 = i_1;
    param = x_54;
    let x_55 : f32 = func_i1_(&(param));
    let x_56 : i32 = i_1;
    param_1 = x_56;
    let x_57 : f32 = func_i1_(&(param_1));
    if ((x_57 == 1.0)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    } else {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    }

    continuing {
      let x_62 : i32 = i_1;
      i_1 = (x_62 + 1);
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
