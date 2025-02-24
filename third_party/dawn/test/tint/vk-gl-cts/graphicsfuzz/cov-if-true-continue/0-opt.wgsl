struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var b : i32;
  var c : i32;
  var x_65 : bool;
  var x_66_phi : bool;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_29;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  b = x_31;
  let x_33 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_33;
  loop {
    let x_38 : i32 = a;
    let x_39 : i32 = b;
    if ((x_38 < x_39)) {
    } else {
      break;
    }
    let x_42 : i32 = a;
    a = (x_42 + 1);
    let x_44 : i32 = c;
    let x_46 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_44 == x_46)) {
      let x_52 : i32 = x_6.x_GLF_uniform_int_values[3].el;
      let x_53 : i32 = c;
      c = (x_53 * x_52);
    } else {
      if (true) {
        continue;
      }
    }
  }
  let x_57 : i32 = a;
  let x_58 : i32 = b;
  let x_59 : bool = (x_57 == x_58);
  x_66_phi = x_59;
  if (x_59) {
    let x_62 : i32 = c;
    let x_64 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_65 = (x_62 == x_64);
    x_66_phi = x_65;
  }
  let x_66 : bool = x_66_phi;
  if (x_66) {
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_74 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_77 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_80 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_71), f32(x_74), f32(x_77), f32(x_80));
  } else {
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_85 : f32 = f32(x_84);
    x_GLF_color = vec4<f32>(x_85, x_85, x_85, x_85);
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
