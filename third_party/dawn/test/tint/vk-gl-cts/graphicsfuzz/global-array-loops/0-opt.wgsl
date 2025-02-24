struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_68 : bool = false;
  var x_29 : i32;
  var x_30 : i32;
  var x_31 : i32;
  var globalNumbers : array<i32, 10u>;
  var x_17 : i32;
  var acc : i32;
  var i_1 : i32;
  var localNumbers : array<i32, 2u>;
  var param : i32;
  var x_24 : i32;
  var x_24_phi : i32;
  var x_23_phi : i32;
  acc = 0;
  i_1 = 0;
  x_24_phi = 0;
  x_23_phi = 0;
  loop {
    var x_33 : i32;
    var x_92 : i32;
    var x_76_phi : bool;
    var x_34_phi : i32;
    var x_25_phi : i32;
    x_24 = x_24_phi;
    let x_23 : i32 = x_23_phi;
    if ((x_23 < 4)) {
    } else {
      break;
    }
    x_68 = false;
    x_76_phi = false;
    loop {
      var x_81 : bool;
      var x_32 : i32;
      var x_81_phi : bool;
      var x_32_phi : i32;
      var x_33_phi : i32;
      var x_90_phi : bool;
      let x_76 : bool = x_76_phi;
      x_30 = 0;
      x_81_phi = x_76;
      x_32_phi = 0;
      loop {
        x_81 = x_81_phi;
        x_32 = x_32_phi;
        let x_86 : f32 = x_8.one;
        x_33_phi = 0;
        x_90_phi = x_81;
        if ((x_32 < i32(x_86))) {
        } else {
          break;
        }
        x_68 = true;
        x_29 = x_32;
        x_33_phi = x_32;
        x_90_phi = true;
        break;

        continuing {
          x_81_phi = false;
          x_32_phi = 0;
        }
      }
      x_33 = x_33_phi;
      let x_90 : bool = x_90_phi;
      x_34_phi = x_33;
      if (x_90) {
        break;
      }
      x_92 = 0;
      x_68 = true;
      x_29 = x_92;
      x_34_phi = x_92;
      break;

      continuing {
        x_76_phi = false;
      }
    }
    let x_34 : i32 = x_34_phi;
    x_31 = x_34;
    let x_93 : i32 = x_31;
    let x_21 : array<i32, 2u> = localNumbers;
    var x_22_1 : array<i32, 2u> = x_21;
    x_22_1[1u] = x_93;
    let x_22 : array<i32, 2u> = x_22_1;
    localNumbers = x_22;
    globalNumbers[0] = 0;
    let x_13 : i32 = x_22[1u];
    param = x_13;
    x_17 = 0;
    x_25_phi = 0;
    loop {
      let x_25 : i32 = x_25_phi;
      if ((x_25 <= x_13)) {
      } else {
        break;
      }
      let x_102_save = x_13;
      let x_18 : i32 = globalNumbers[x_102_save];
      if ((x_18 <= 1)) {
        globalNumbers[x_102_save] = 1;
      }

      continuing {
        let x_19 : i32 = (x_25 + 1);
        x_17 = x_19;
        x_25_phi = x_19;
      }
    }
    let x_107 : f32 = x_8.one;
    let x_14 : i32 = globalNumbers[(i32(x_107) - 1)];
    let x_15 : i32 = bitcast<i32>((x_24 + bitcast<i32>(x_14)));
    acc = x_15;

    continuing {
      let x_16 : i32 = (x_23 + 1);
      i_1 = x_16;
      x_24_phi = x_15;
      x_23_phi = x_16;
    }
  }
  if ((x_24 == bitcast<i32>(4))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
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

fn yieldsZero_() -> i32 {
  var x_116 : bool = false;
  var x_20 : i32;
  var i : i32;
  var x_26 : i32;
  var x_134 : i32;
  var x_118_phi : bool;
  var x_27_phi : i32;
  x_118_phi = false;
  loop {
    var x_123 : bool;
    var x_28 : i32;
    var x_123_phi : bool;
    var x_28_phi : i32;
    var x_26_phi : i32;
    var x_132_phi : bool;
    let x_118 : bool = x_118_phi;
    i = 0;
    x_123_phi = x_118;
    x_28_phi = 0;
    loop {
      x_123 = x_123_phi;
      x_28 = x_28_phi;
      let x_128 : f32 = x_8.one;
      x_26_phi = 0;
      x_132_phi = x_123;
      if ((x_28 < i32(x_128))) {
      } else {
        break;
      }
      x_116 = true;
      x_20 = x_28;
      x_26_phi = x_28;
      x_132_phi = true;
      break;

      continuing {
        x_123_phi = false;
        x_28_phi = 0;
      }
    }
    x_26 = x_26_phi;
    let x_132 : bool = x_132_phi;
    x_27_phi = x_26;
    if (x_132) {
      break;
    }
    x_134 = 0;
    x_116 = true;
    x_20 = x_134;
    x_27_phi = x_134;
    break;

    continuing {
      x_118_phi = false;
    }
  }
  let x_27 : i32 = x_27_phi;
  return x_27;
}
