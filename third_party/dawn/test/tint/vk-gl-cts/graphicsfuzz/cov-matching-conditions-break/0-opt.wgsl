struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var i : i32;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_34 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_37 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_31), f32(x_34), f32(x_37), f32(x_40));
  let x_44 : f32 = gl_FragCoord.y;
  if ((x_44 < 0.0)) {
    let x_49 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_50 : f32 = f32(x_49);
    x_GLF_color = vec4<f32>(x_50, x_50, x_50, x_50);
  }
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_53;
  loop {
    let x_58 : i32 = i;
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_58 < x_60)) {
    } else {
      break;
    }
    let x_64 : f32 = gl_FragCoord.x;
    if ((x_64 > 0.0)) {
      let x_69 : f32 = gl_FragCoord.y;
      if ((x_69 < 0.0)) {
        let x_74 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        let x_75 : f32 = f32(x_74);
        x_GLF_color = vec4<f32>(x_75, x_75, x_75, x_75);
        break;
      }
    }
    let x_78 : f32 = gl_FragCoord.x;
    if ((x_78 > 0.0)) {
      let x_83 : f32 = gl_FragCoord.y;
      if ((x_83 < 0.0)) {
        let x_88 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        let x_89 : f32 = f32(x_88);
        x_GLF_color = vec4<f32>(x_89, x_89, x_89, x_89);
      }
    }

    continuing {
      let x_91 : i32 = i;
      i = (x_91 + 1);
    }
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
