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
  var a : i32;
  var b : i32;
  var c : i32;
  var x_76 : bool;
  var x_83 : bool;
  var x_77_phi : bool;
  var x_84_phi : bool;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_31;
  let x_33 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  b = x_33;
  c = 1;
  loop {
    let x_38 : i32 = b;
    let x_40 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_42 : i32 = a;
    if (((x_38 < x_40) & (x_42 < 10))) {
    } else {
      break;
    }
    let x_46 : i32 = c;
    if ((x_46 > 5)) {
      break;
    }
    let x_50 : i32 = a;
    a = (x_50 + 1);
    let x_52 : i32 = c;
    c = (x_52 + 1);
    let x_54 : i32 = b;
    b = (x_54 + 1);
  }
  loop {
    let x_60 : i32 = a;
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_60 < x_62)) {
    } else {
      break;
    }

    continuing {
      let x_65 : i32 = a;
      a = (x_65 + 1);
    }
  }
  let x_67 : i32 = a;
  let x_69 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_70 : bool = (x_67 == x_69);
  x_77_phi = x_70;
  if (x_70) {
    let x_73 : i32 = b;
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  let x_77 : bool = x_77_phi;
  x_84_phi = x_77;
  if (x_77) {
    let x_80 : i32 = c;
    let x_82 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_83 = (x_80 == x_82);
    x_84_phi = x_83;
  }
  let x_84 : bool = x_84_phi;
  if (x_84) {
    let x_89 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_92 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_95 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_98 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_89), f32(x_92), f32(x_95), f32(x_98));
  } else {
    let x_102 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_103 : f32 = f32(x_102);
    x_GLF_color = vec4<f32>(x_103, x_103, x_103, x_103);
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
