struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var A : array<i32, 2u>;
  var i : i32;
  var a : i32;
  var v1 : vec2<f32>;
  var v2 : vec2<f32>;
  var b : i32;
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  A[x_46] = x_48;
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  A[x_51] = x_53;
  let x_56 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  i = x_56;
  loop {
    let x_61 : i32 = i;
    let x_63 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_61 > x_63)) {
    } else {
      break;
    }
    let x_66 : i32 = i;
    i = (x_66 - 1);
  }
  let x_69 : f32 = x_10.x_GLF_uniform_float_values[1].el;
  let x_71 : f32 = x_10.x_GLF_uniform_float_values[1].el;
  let x_73 : i32 = i;
  let x_76 : i32 = A[select(x_73, 1, (x_69 >= x_71))];
  a = x_76;
  let x_78 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_80 : i32 = a;
  let x_84 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_87 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_91 : f32 = x_10.x_GLF_uniform_float_values[1].el;
  let x_93 : f32 = x_10.x_GLF_uniform_float_values[0].el;
  v1 = select(vec2<f32>(f32(x_78), f32(x_80)), vec2<f32>(f32(x_84), f32(x_87)), vec2<bool>((x_91 < x_93), true));
  let x_98 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_100 : f32 = v1[x_98];
  let x_103 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_105 : f32 = v1[x_103];
  v2 = select(vec2<f32>(x_100, x_100), vec2<f32>(x_105, x_105), vec2<bool>(false, false));
  let x_109 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_110 : f32 = f32(x_109);
  let x_113 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_114 : f32 = f32(x_113);
  let x_116 : vec2<f32> = v2;
  let x_121 : i32 = A[i32(clamp(vec2<f32>(x_110, x_110), vec2<f32>(x_114, x_114), x_116).x)];
  b = x_121;
  let x_122 : i32 = b;
  let x_124 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_122 == x_124)) {
    let x_130 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_133 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_136 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_139 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_130), f32(x_133), f32(x_136), f32(x_139));
  } else {
    let x_143 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_144 : f32 = f32(x_143);
    x_GLF_color = vec4<f32>(x_144, x_144, x_144, x_144);
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
