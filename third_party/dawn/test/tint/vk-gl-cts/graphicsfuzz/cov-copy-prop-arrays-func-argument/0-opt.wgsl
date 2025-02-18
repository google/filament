struct Array {
  values : array<i32, 2u>,
}

struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_50 : bool = false;
  var x_15 : i32;
  var x_16 : i32;
  var param : Array;
  var x_19 : i32;
  var x_20_phi : i32;
  param = Array(array<i32, 2u>());
  x_50 = false;
  loop {
    var x_19_phi : i32;
    var x_63_phi : bool;
    loop {
      let x_17 : i32 = x_8.zero;
      let x_18 : i32 = param.values[x_17];
      if ((x_18 == 1)) {
        x_50 = true;
        x_15 = 1;
        x_19_phi = 1;
        x_63_phi = true;
        break;
      }
      x_19_phi = 0;
      x_63_phi = false;
      break;
    }
    x_19 = x_19_phi;
    let x_63 : bool = x_63_phi;
    x_20_phi = x_19;
    if (x_63) {
      break;
    }
    x_50 = true;
    x_15 = 1;
    x_20_phi = 1;
    break;
  }
  let x_20 : i32 = x_20_phi;
  x_16 = x_20;
  if ((x_20 == 1)) {
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
  var x_70 : bool = false;
  var x_12 : i32;
  var x_13 : i32;
  var x_72_phi : bool;
  var x_14_phi : i32;
  x_72_phi = false;
  loop {
    var x_77 : bool;
    var x_77_phi : bool;
    var x_13_phi : i32;
    var x_87_phi : bool;
    let x_72 : bool = x_72_phi;
    x_77_phi = x_72;
    loop {
      x_77 = x_77_phi;
      let x_10 : i32 = x_8.zero;
      let x_11 : i32 = (*(a)).values[x_10];
      if ((x_11 == 1)) {
        x_70 = true;
        x_12 = 1;
        x_13_phi = 1;
        x_87_phi = true;
        break;
      }
      x_13_phi = 0;
      x_87_phi = x_77;
      break;

      continuing {
        x_77_phi = false;
      }
    }
    x_13 = x_13_phi;
    let x_87 : bool = x_87_phi;
    x_14_phi = x_13;
    if (x_87) {
      break;
    }
    x_70 = true;
    x_12 = 1;
    x_14_phi = 1;
    break;

    continuing {
      x_72_phi = false;
    }
  }
  let x_14 : i32 = x_14_phi;
  return x_14;
}
