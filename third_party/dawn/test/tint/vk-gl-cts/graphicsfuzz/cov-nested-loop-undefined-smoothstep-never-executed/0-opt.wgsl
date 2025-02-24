struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v0 : vec4<f32>;
  var v1 : vec4<f32>;
  var a : i32;
  var c : i32;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  v0 = vec4<f32>(x_41, x_41, x_41, x_41);
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec4<f32>(x_44, x_44, x_44, x_44);
  let x_47 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  a = x_47;
  loop {
    let x_52 : i32 = a;
    let x_54 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_52 < x_54)) {
    } else {
      break;
    }
    let x_58 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    c = x_58;
    loop {
      let x_63 : i32 = c;
      let x_65 : i32 = x_10.x_GLF_uniform_int_values[2].el;
      if ((x_63 < x_65)) {
      } else {
        break;
      }
      let x_68 : i32 = c;
      let x_69 : i32 = clamp(x_68, 0, 3);
      let x_71 : f32 = x_6.x_GLF_uniform_float_values[1].el;
      let x_73 : f32 = v0[x_69];
      v0[x_69] = (x_73 - x_71);
      let x_77 : i32 = x_10.x_GLF_uniform_int_values[1].el;
      let x_79 : i32 = x_10.x_GLF_uniform_int_values[3].el;
      if ((x_77 == x_79)) {
        let x_83 : i32 = a;
        let x_85 : f32 = x_6.x_GLF_uniform_float_values[1].el;
        let x_87 : f32 = x_6.x_GLF_uniform_float_values[1].el;
        let x_89 : f32 = x_6.x_GLF_uniform_float_values[1].el;
        let x_91 : vec4<f32> = v0;
        let x_93 : i32 = a;
        v1[x_83] = smoothstep(vec4<f32>(x_85, x_87, x_89, 3.0), vec4<f32>(1.0, 1.0, 1.0, 1.0), x_91)[x_93];
      }

      continuing {
        let x_96 : i32 = c;
        c = (x_96 + 1);
      }
    }

    continuing {
      let x_98 : i32 = a;
      a = (x_98 + 1);
    }
  }
  let x_101 : f32 = v1.x;
  let x_103 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_101 == x_103)) {
    let x_109 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_112 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_115 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_118 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_109), f32(x_112), f32(x_115), f32(x_118));
  } else {
    let x_122 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_123 : f32 = f32(x_122);
    x_GLF_color = vec4<f32>(x_123, x_123, x_123, x_123);
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
