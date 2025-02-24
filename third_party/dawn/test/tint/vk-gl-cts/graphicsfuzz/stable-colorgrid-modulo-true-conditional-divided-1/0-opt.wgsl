struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn compute_value_f1_f1_(limit : ptr<function, f32>, thirty_two : ptr<function, f32>) -> f32 {
  var x_91 : f32;
  var x_91_phi : f32;
  var x_94_phi : i32;
  x_91_phi = -0.5;
  x_94_phi = 1;
  loop {
    var x_104 : f32;
    var x_113 : f32;
    var x_95 : i32;
    var x_92_phi : f32;
    x_91 = x_91_phi;
    let x_94 : i32 = x_94_phi;
    if ((x_94 < 800)) {
    } else {
      break;
    }
    var x_112 : f32;
    var x_113_phi : f32;
    if (((x_94 % 32) == 0)) {
      x_104 = (x_91 + 0.400000006);
      x_92_phi = x_104;
    } else {
      let x_106 : f32 = *(thirty_two);
      x_113_phi = x_91;
      if (((f32(x_94) - (round(x_106) * floor((f32(x_94) / round(x_106))))) <= 0.01)) {
        x_112 = (x_91 + 100.0);
        x_113_phi = x_112;
      }
      x_113 = x_113_phi;
      x_92_phi = x_113;
    }
    var x_92 : f32;
    x_92 = x_92_phi;
    let x_115 : f32 = *(limit);
    if ((f32(x_94) >= x_115)) {
      return x_92;
    }

    continuing {
      x_95 = (x_94 + 1);
      x_91_phi = x_92;
      x_94_phi = x_95;
    }
  }
  return x_91;
}

fn main_1() {
  var c : vec3<f32>;
  var param : f32;
  var param_1 : f32;
  var param_2 : f32;
  var param_3 : f32;
  var x_68_phi : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_52 : f32 = x_10.resolution.x;
  let x_54 : f32 = round((x_52 * 0.125));
  let x_56 : f32 = gl_FragCoord.x;
  param = x_56;
  param_1 = x_54;
  let x_57 : f32 = compute_value_f1_f1_(&(param), &(param_1));
  c.x = x_57;
  let x_60 : f32 = gl_FragCoord.y;
  param_2 = x_60;
  param_3 = x_54;
  let x_61 : f32 = compute_value_f1_f1_(&(param_2), &(param_3));
  c.y = x_61;
  let x_63 : f32 = c.x;
  let x_64 : f32 = c.y;
  c.z = (x_63 + x_64);
  x_68_phi = 0;
  loop {
    var x_69 : i32;
    let x_68 : i32 = x_68_phi;
    if ((x_68 < 3)) {
    } else {
      break;
    }
    let x_75 : f32 = c[x_68];
    if ((x_75 >= 1.0)) {
      let x_79 : f32 = c[x_68];
      let x_80 : f32 = c[x_68];
      c[x_68] = (x_79 * x_80);
    }

    continuing {
      x_69 = (x_68 + 1);
      x_68_phi = x_69;
    }
  }
  let x_82 : vec3<f32> = c;
  let x_84 : vec3<f32> = normalize(abs(x_82));
  x_GLF_color = vec4<f32>(x_84.x, x_84.y, x_84.z, 1.0);
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
