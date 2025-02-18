struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_23 : i32;
  var x_27 : i32;
  var x_37 : i32;
  var x_23_phi : i32;
  var x_45_phi : i32;
  x_23_phi = 0;
  loop {
    var x_24 : i32;
    x_23 = x_23_phi;
    x_27 = x_5.x_GLF_uniform_int_values[1].el;
    if ((x_23 < (100 - bitcast<i32>(x_27)))) {
    } else {
      break;
    }

    continuing {
      x_24 = bitcast<i32>((x_23 + bitcast<i32>(1)));
      x_23_phi = x_24;
    }
  }
  var x_37_phi : i32;
  var x_40_phi : i32;
  let x_32 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  x_45_phi = 1;
  if ((x_32 == 0)) {
    x_37_phi = 1;
    x_40_phi = x_23;
    loop {
      var x_41 : i32;
      var x_38 : i32;
      x_37 = x_37_phi;
      let x_40 : i32 = x_40_phi;
      if ((x_40 < 100)) {
      } else {
        break;
      }

      continuing {
        x_41 = (x_40 + 1);
        x_38 = bitcast<i32>((x_37 * bitcast<i32>((1 - bitcast<i32>(x_37)))));
        x_37_phi = x_38;
        x_40_phi = x_41;
      }
    }
    x_45_phi = x_37;
  }
  let x_45 : i32 = x_45_phi;
  if ((x_45 == x_32)) {
    let x_50 : f32 = f32(x_27);
    let x_51 : f32 = f32(x_32);
    x_GLF_color = vec4<f32>(x_50, x_51, x_51, x_50);
  } else {
    let x_53 : f32 = f32(x_32);
    x_GLF_color = vec4<f32>(x_53, x_53, x_53, x_53);
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
