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
  var m : mat2x2<f32>;
  var f : f32;
  var i : i32;
  var j : i32;
  let x_36 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  if ((x_36 == 1)) {
    let x_40 : f32 = f;
    m = mat2x2<f32>(vec2<f32>(x_40, 0.0), vec2<f32>(0.0, x_40));
  }
  let x_45 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  i = x_45;
  loop {
    let x_50 : i32 = i;
    let x_52 : i32 = x_5.x_GLF_uniform_int_values[0].el;
    if ((x_50 < x_52)) {
    } else {
      break;
    }
    let x_56 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    j = x_56;
    loop {
      let x_61 : i32 = j;
      let x_63 : i32 = x_5.x_GLF_uniform_int_values[0].el;
      if ((x_61 < x_63)) {
      } else {
        break;
      }
      let x_66 : i32 = i;
      let x_67 : i32 = j;
      let x_68 : i32 = i;
      let x_70 : i32 = x_5.x_GLF_uniform_int_values[0].el;
      let x_72 : i32 = j;
      m[x_66][x_67] = f32(((x_68 * x_70) + x_72));

      continuing {
        let x_76 : i32 = j;
        j = (x_76 + 1);
      }
    }

    continuing {
      let x_78 : i32 = i;
      i = (x_78 + 1);
    }
  }
  let x_80 : mat2x2<f32> = m;
  let x_82 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_85 : i32 = x_5.x_GLF_uniform_int_values[2].el;
  let x_88 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_91 : i32 = x_5.x_GLF_uniform_int_values[3].el;
  let x_95 : mat2x2<f32> = mat2x2<f32>(vec2<f32>(f32(x_82), f32(x_85)), vec2<f32>(f32(x_88), f32(x_91)));
  if ((all((x_80[0u] == x_95[0u])) & all((x_80[1u] == x_95[1u])))) {
    let x_109 : i32 = x_5.x_GLF_uniform_int_values[2].el;
    let x_112 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    let x_115 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    let x_118 : i32 = x_5.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_109), f32(x_112), f32(x_115), f32(x_118));
  } else {
    let x_122 : i32 = x_5.x_GLF_uniform_int_values[1].el;
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
