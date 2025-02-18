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
  var x_147 : bool;
  var x_158 : f32;
  var x_159 : f32;
  var x_179 : f32;
  var x_178 : f32;
  var x_185 : f32;
  var x_184 : f32;
  var x_160_phi : f32;
  var x_180_phi : f32;
  var x_186_phi : f32;
  let x_149 : f32 = gl_FragCoord.y;
  let x_151 : f32 = x_9.resolution.y;
  let x_153 : bool = (x_149 < (x_151 / 2.0));
  if (x_153) {
    x_158 = *(a);
    x_160_phi = x_158;
  } else {
    x_159 = 0.0;
    x_160_phi = x_159;
  }
  var x_166 : f32;
  var x_167 : f32;
  var x_168_phi : f32;
  let x_160 : f32 = x_160_phi;
  var guard155 : bool = true;
  if (false) {
  } else {
    if (guard155) {
      if (x_153) {
        x_166 = *(b);
        x_168_phi = x_166;
      } else {
        x_167 = 0.0;
        x_168_phi = x_167;
      }
      let x_168 : f32 = x_168_phi;
      let x_169 : bool = (x_160 > x_168);
      if (x_153) {
        x_147 = x_169;
      }
      if (true) {
      } else {
        guard155 = false;
      }
      if (guard155) {
        guard155 = false;
      }
    }
  }
  if (x_153) {
    x_179 = 0.0;
    x_180_phi = x_179;
  } else {
    x_178 = *(a);
    x_180_phi = x_178;
  }
  let x_180 : f32 = x_180_phi;
  if (x_153) {
    x_185 = 0.0;
    x_186_phi = x_185;
  } else {
    x_184 = *(b);
    x_186_phi = x_184;
  }
  let x_186 : f32 = x_186_phi;
  if (x_153) {
  } else {
    x_147 = (x_180 < x_186);
  }
  let x_191 : bool = x_147;
  return x_191;
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
    let x_59 : i32 = i;
    if ((x_59 < 10)) {
    } else {
      break;
    }
    let x_62 : i32 = i;
    let x_63 : i32 = i;
    let x_67 : f32 = x_13.injectionSwitch.y;
    data[x_62] = (f32((10 - x_63)) * x_67);

    continuing {
      let x_70 : i32 = i;
      i = (x_70 + 1);
    }
  }
  i_1 = 0;
  loop {
    let x_76 : i32 = i_1;
    if ((x_76 < 9)) {
    } else {
      break;
    }
    j = 0;
    loop {
      let x_83 : i32 = j;
      if ((x_83 < 10)) {
      } else {
        break;
      }
      let x_86 : i32 = j;
      let x_87 : i32 = i_1;
      if ((x_86 < (x_87 + 1))) {
        continue;
      }
      let x_92 : i32 = i_1;
      let x_93 : i32 = j;
      let x_95 : f32 = data[x_92];
      param = x_95;
      let x_97 : f32 = data[x_93];
      param_1 = x_97;
      let x_98 : bool = checkSwap_f1_f1_(&(param), &(param_1));
      doSwap = x_98;
      let x_99 : bool = doSwap;
      if (x_99) {
        let x_102 : i32 = i_1;
        let x_104 : f32 = data[x_102];
        temp = x_104;
        let x_105 : i32 = i_1;
        let x_106 : i32 = j;
        let x_108 : f32 = data[x_106];
        data[x_105] = x_108;
        let x_110 : i32 = j;
        let x_111 : f32 = temp;
        data[x_110] = x_111;
      }

      continuing {
        let x_113 : i32 = j;
        j = (x_113 + 1);
      }
    }

    continuing {
      let x_115 : i32 = i_1;
      i_1 = (x_115 + 1);
    }
  }
  let x_118 : f32 = gl_FragCoord.x;
  let x_120 : f32 = x_9.resolution.x;
  if ((x_118 < (x_120 / 2.0))) {
    let x_127 : f32 = data[0];
    let x_130 : f32 = data[5];
    let x_133 : f32 = data[9];
    x_GLF_color = vec4<f32>((x_127 / 10.0), (x_130 / 10.0), (x_133 / 10.0), 1.0);
  } else {
    let x_137 : f32 = data[5];
    let x_140 : f32 = data[9];
    let x_143 : f32 = data[0];
    x_GLF_color = vec4<f32>((x_137 / 10.0), (x_140 / 10.0), (x_143 / 10.0), 1.0);
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
