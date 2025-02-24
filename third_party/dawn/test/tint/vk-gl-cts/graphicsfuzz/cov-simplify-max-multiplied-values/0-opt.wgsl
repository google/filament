struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var A : array<i32, 4u>;
  var x_77 : bool;
  var x_87 : bool;
  var x_97 : bool;
  var x_78_phi : bool;
  var x_88_phi : bool;
  var x_98_phi : bool;
  let x_33 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  i = x_33;
  loop {
    let x_38 : i32 = i;
    let x_40 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    let x_43 : i32 = i;
    let x_45 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    A[x_43] = x_45;
    let x_47 : i32 = i;
    let x_50 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_54 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((max((2 * x_47), (2 * x_50)) == x_54)) {
      let x_58 : i32 = i;
      A[x_58] = 1;
    }

    continuing {
      let x_60 : i32 = i;
      i = (x_60 + 1);
    }
  }
  let x_63 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_65 : i32 = A[x_63];
  let x_67 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_68 : bool = (x_65 == x_67);
  x_78_phi = x_68;
  if (x_68) {
    let x_72 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_74 : i32 = A[x_72];
    let x_76 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_77 = (x_74 == x_76);
    x_78_phi = x_77;
  }
  let x_78 : bool = x_78_phi;
  x_88_phi = x_78;
  if (x_78) {
    let x_82 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_84 : i32 = A[x_82];
    let x_86 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_87 = (x_84 == x_86);
    x_88_phi = x_87;
  }
  let x_88 : bool = x_88_phi;
  x_98_phi = x_88;
  if (x_88) {
    let x_92 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_94 : i32 = A[x_92];
    let x_96 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_97 = (x_94 == x_96);
    x_98_phi = x_97;
  }
  let x_98 : bool = x_98_phi;
  if (x_98) {
    let x_103 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_106 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_109 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_112 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_103), f32(x_106), f32(x_109), f32(x_112));
  } else {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
