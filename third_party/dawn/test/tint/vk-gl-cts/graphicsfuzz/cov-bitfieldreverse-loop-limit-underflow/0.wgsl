struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_28 : i32;
  var x_29 : i32;
  var x_28_phi : i32;
  var x_31_phi : i32;
  var x_42_phi : i32;
  let x_24 : i32 = min(1, reverseBits(1));
  let x_26 : i32 = x_5.x_GLF_uniform_int_values[3].el;
  x_28_phi = x_26;
  x_31_phi = 1;
  loop {
    var x_32 : i32;
    x_28 = x_28_phi;
    let x_31 : i32 = x_31_phi;
    x_42_phi = x_28;
    if ((x_31 <= (x_24 - 1))) {
    } else {
      break;
    }
    x_29 = bitcast<i32>((x_28 + bitcast<i32>(x_31)));
    let x_38 : i32 = x_5.x_GLF_uniform_int_values[0].el;
    if ((x_38 == 1)) {
      x_42_phi = x_29;
      break;
    }

    continuing {
      x_32 = (x_31 + 1);
      x_28_phi = x_29;
      x_31_phi = x_32;
    }
  }
  let x_42 : i32 = x_42_phi;
  let x_44 : i32 = x_5.x_GLF_uniform_int_values[2].el;
  if ((x_42 == x_44)) {
    let x_50 : i32 = x_5.x_GLF_uniform_int_values[0].el;
    let x_51 : f32 = f32(x_50);
    let x_53 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    let x_54 : f32 = f32(x_53);
    x_GLF_color = vec4<f32>(x_51, x_54, x_54, x_51);
  } else {
    let x_57 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    let x_58 : f32 = f32(x_57);
    x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
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
