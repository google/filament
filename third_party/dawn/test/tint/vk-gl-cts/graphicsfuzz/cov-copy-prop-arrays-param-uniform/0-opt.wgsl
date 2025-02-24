struct Array {
  values : array<i32, 2u>,
}

struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_52 : bool = false;
  var x_17 : i32;
  var x_18 : i32;
  var x_16 : array<i32, 2u>;
  var param : Array;
  var x_20 : i32;
  var x_21_phi : i32;
  let x_12 : i32 = x_8.zero;
  let x_22 : array<i32, 2u> = x_16;
  var x_23_1 : array<i32, 2u> = x_22;
  x_23_1[0u] = x_12;
  let x_23 : array<i32, 2u> = x_23_1;
  x_16 = x_23;
  let x_54 : array<i32, 2u> = x_16;
  param = Array(x_54);
  x_52 = false;
  loop {
    var x_20_phi : i32;
    var x_67_phi : bool;
    loop {
      let x_19 : i32 = param.values[x_12];
      if ((x_19 == 0)) {
        x_52 = true;
        x_17 = 42;
        x_20_phi = 42;
        x_67_phi = true;
        break;
      }
      x_20_phi = 0;
      x_67_phi = false;
      break;
    }
    x_20 = x_20_phi;
    let x_67 : bool = x_67_phi;
    x_21_phi = x_20;
    if (x_67) {
      break;
    }
    x_52 = true;
    x_17 = 42;
    x_21_phi = 42;
    break;
  }
  let x_21 : i32 = x_21_phi;
  x_18 = x_21;
  if ((x_21 == 42)) {
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

fn func_struct_Array_i1_2_1_(a : ptr<function, Array>) -> i32 {
  var x_74 : bool = false;
  var x_13 : i32;
  var x_14 : i32;
  var x_76_phi : bool;
  var x_15_phi : i32;
  x_76_phi = false;
  loop {
    var x_81 : bool;
    var x_81_phi : bool;
    var x_14_phi : i32;
    var x_91_phi : bool;
    let x_76 : bool = x_76_phi;
    x_81_phi = x_76;
    loop {
      x_81 = x_81_phi;
      let x_10 : i32 = x_8.zero;
      let x_11 : i32 = (*(a)).values[x_10];
      if ((x_11 == 0)) {
        x_74 = true;
        x_13 = 42;
        x_14_phi = 42;
        x_91_phi = true;
        break;
      }
      x_14_phi = 0;
      x_91_phi = x_81;
      break;

      continuing {
        x_81_phi = false;
      }
    }
    x_14 = x_14_phi;
    let x_91 : bool = x_91_phi;
    x_15_phi = x_14;
    if (x_91) {
      break;
    }
    x_74 = true;
    x_13 = 42;
    x_15_phi = 42;
    break;

    continuing {
      x_76_phi = false;
    }
  }
  let x_15 : i32 = x_15_phi;
  return x_15;
}
