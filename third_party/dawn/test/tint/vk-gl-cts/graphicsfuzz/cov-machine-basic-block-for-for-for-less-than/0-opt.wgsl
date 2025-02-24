struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 6u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_12 : buf0;

fn main_1() {
  var data : array<i32, 5u>;
  var a : i32;
  var i : i32;
  var j : i32;
  var i_1 : i32;
  let x_45 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_54 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_45), f32(x_48), f32(x_51), f32(x_54));
  let x_58 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_60 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_64 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_66 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  data = array<i32, 5u>(x_58, x_60, x_62, x_64, x_66);
  let x_69 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  a = x_69;
  loop {
    let x_74 : i32 = a;
    let x_76 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_74 < x_76)) {
    } else {
      break;
    }
    let x_80 : i32 = x_6.x_GLF_uniform_int_values[5].el;
    i = x_80;
    loop {
      let x_85 : i32 = i;
      let x_87 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      if ((x_85 < x_87)) {
      } else {
        break;
      }
      let x_90 : i32 = i;
      j = x_90;
      loop {
        let x_95 : i32 = j;
        let x_97 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        if ((x_95 < x_97)) {
        } else {
          break;
        }
        let x_100 : i32 = i;
        let x_102 : i32 = data[x_100];
        let x_103 : i32 = j;
        let x_105 : i32 = data[x_103];
        if ((x_102 < x_105)) {
          let x_110 : i32 = x_6.x_GLF_uniform_int_values[5].el;
          let x_111 : f32 = f32(x_110);
          x_GLF_color = vec4<f32>(x_111, x_111, x_111, x_111);
        }

        continuing {
          let x_113 : i32 = j;
          j = (x_113 + 1);
        }
      }

      continuing {
        let x_115 : i32 = i;
        i = (x_115 + 1);
      }
    }

    continuing {
      let x_117 : i32 = a;
      a = (x_117 + 1);
    }
  }
  loop {
    let x_124 : f32 = gl_FragCoord.x;
    let x_126 : f32 = x_12.x_GLF_uniform_float_values[0].el;
    if ((x_124 < x_126)) {
    } else {
      break;
    }
    let x_130 : i32 = x_6.x_GLF_uniform_int_values[5].el;
    i_1 = x_130;
    loop {
      let x_135 : i32 = i_1;
      let x_137 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_135 < x_137)) {
      } else {
        break;
      }
      let x_141 : i32 = x_6.x_GLF_uniform_int_values[5].el;
      let x_142 : f32 = f32(x_141);
      x_GLF_color = vec4<f32>(x_142, x_142, x_142, x_142);

      continuing {
        let x_144 : i32 = i_1;
        i_1 = (x_144 + 1);
      }
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
