struct buf1 {
  resolution : vec2<f32>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_9 : buf1;

@group(0) @binding(0) var<uniform> x_13 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn checkSwap_f1_f1_(a : ptr<function, f32>, b : ptr<function, f32>) -> bool {
  var x_144 : bool;
  let x_146 : f32 = gl_FragCoord.y;
  let x_148 : f32 = x_9.resolution.y;
  if ((x_146 < (x_148 / 2.0))) {
    let x_154 : f32 = *(a);
    let x_155 : f32 = *(b);
    x_144 = (x_154 > x_155);
  } else {
    let x_157 : f32 = *(a);
    let x_158 : f32 = *(b);
    x_144 = (x_157 < x_158);
  }
  let x_160 : bool = x_144;
  return x_160;
}

fn main_1() {
  var i : i32;
  var data : array<f32, 10u>;
  var i_1 : i32;
  var j : i32;
  var doSwap : bool;
  var param : f32;
  var param_1 : f32;
  var temp : f32;
  i = 0;
  loop {
    let x_56 : i32 = i;
    if ((x_56 < 10)) {
    } else {
      break;
    }
    let x_59 : i32 = i;
    let x_60 : i32 = i;
    let x_64 : f32 = x_13.injectionSwitch.y;
    data[x_59] = (f32((10 - x_60)) * x_64);

    continuing {
      let x_67 : i32 = i;
      i = (x_67 + 1);
    }
  }
  i_1 = 0;
  loop {
    let x_73 : i32 = i_1;
    if ((x_73 < 9)) {
    } else {
      break;
    }
    j = 0;
    loop {
      let x_80 : i32 = j;
      if ((x_80 < 10)) {
      } else {
        break;
      }
      let x_83 : i32 = j;
      let x_84 : i32 = i_1;
      if ((x_83 < (x_84 + 1))) {
        continue;
      }
      let x_89 : i32 = i_1;
      let x_90 : i32 = j;
      let x_92 : f32 = data[x_89];
      param = x_92;
      let x_94 : f32 = data[x_90];
      param_1 = x_94;
      let x_95 : bool = checkSwap_f1_f1_(&(param), &(param_1));
      doSwap = x_95;
      let x_96 : bool = doSwap;
      if (x_96) {
        let x_99 : i32 = i_1;
        let x_101 : f32 = data[x_99];
        temp = x_101;
        let x_102 : i32 = i_1;
        let x_103 : i32 = j;
        let x_105 : f32 = data[x_103];
        data[x_102] = x_105;
        let x_107 : i32 = j;
        let x_108 : f32 = temp;
        data[x_107] = x_108;
      }

      continuing {
        let x_110 : i32 = j;
        j = (x_110 + 1);
      }
    }

    continuing {
      let x_112 : i32 = i_1;
      i_1 = (x_112 + 1);
    }
  }
  let x_115 : f32 = gl_FragCoord.x;
  let x_117 : f32 = x_9.resolution.x;
  if ((x_115 < (x_117 / 2.0))) {
    let x_124 : f32 = data[0];
    let x_127 : f32 = data[5];
    let x_130 : f32 = data[9];
    x_GLF_color = vec4<f32>((x_124 / 10.0), (x_127 / 10.0), (x_130 / 10.0), 1.0);
  } else {
    let x_134 : f32 = data[5];
    let x_137 : f32 = data[9];
    let x_140 : f32 = data[0];
    x_GLF_color = vec4<f32>((x_134 / 10.0), (x_137 / 10.0), (x_140 / 10.0), 1.0);
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
