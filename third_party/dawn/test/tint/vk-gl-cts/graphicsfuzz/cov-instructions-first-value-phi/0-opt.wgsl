struct S {
  data : i32,
}

struct buf1 {
  v1 : vec2<f32>,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(1) var<uniform> x_8 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_struct_S_i11_(s : ptr<function, S>) {
  let x_166 : f32 = x_8.v1.x;
  let x_168 : f32 = x_8.v1.y;
  if ((x_166 > x_168)) {
    return;
  }
  let x_173 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  (*(s)).data = x_173;
  return;
}

fn main_1() {
  var i : i32;
  var arr : array<S, 3u>;
  var i_1 : i32;
  var param : S;
  var j : i32;
  var x_132 : bool;
  var x_142 : bool;
  var x_133_phi : bool;
  var x_143_phi : bool;
  let x_46 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  i = x_46;
  loop {
    let x_51 : i32 = i;
    let x_53 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_51 < x_53)) {
    } else {
      break;
    }
    let x_56 : i32 = i;
    let x_57 : i32 = i;
    arr[x_56].data = x_57;

    continuing {
      let x_59 : i32 = i;
      i = (x_59 + 1);
    }
  }
  let x_62 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  i_1 = x_62;
  loop {
    let x_67 : i32 = i_1;
    let x_69 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_67 < x_69)) {
    } else {
      break;
    }
    let x_73 : f32 = x_8.v1.x;
    let x_75 : f32 = x_8.v1.y;
    if ((x_73 > x_75)) {
      break;
    }
    let x_79 : i32 = i_1;
    let x_81 : i32 = arr[x_79].data;
    let x_83 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    if ((x_81 == x_83)) {
      let x_88 : i32 = i_1;
      let x_90 : S = arr[x_88];
      param = x_90;
      func_struct_S_i11_(&(param));
      let x_92 : S = param;
      arr[x_88] = x_92;
    } else {
      let x_95 : i32 = x_10.x_GLF_uniform_int_values[2].el;
      j = x_95;
      loop {
        let x_100 : i32 = j;
        let x_102 : i32 = x_10.x_GLF_uniform_int_values[0].el;
        if ((x_100 < x_102)) {
        } else {
          break;
        }
        let x_105 : i32 = j;
        let x_107 : i32 = arr[x_105].data;
        let x_109 : i32 = x_10.x_GLF_uniform_int_values[4].el;
        if ((x_107 > x_109)) {
          discard;
        }

        continuing {
          let x_113 : i32 = j;
          j = (x_113 + 1);
        }
      }
    }

    continuing {
      let x_115 : i32 = i_1;
      i_1 = (x_115 + 1);
    }
  }
  let x_118 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  let x_120 : i32 = arr[x_118].data;
  let x_122 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  let x_123 : bool = (x_120 == x_122);
  x_133_phi = x_123;
  if (x_123) {
    let x_127 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_129 : i32 = arr[x_127].data;
    let x_131 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_132 = (x_129 == x_131);
    x_133_phi = x_132;
  }
  let x_133 : bool = x_133_phi;
  x_143_phi = x_133;
  if (x_133) {
    let x_137 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_139 : i32 = arr[x_137].data;
    let x_141 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_142 = (x_139 == x_141);
    x_143_phi = x_142;
  }
  let x_143 : bool = x_143_phi;
  if (x_143) {
    let x_148 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_151 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_154 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_157 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_148), f32(x_151), f32(x_154), f32(x_157));
  } else {
    let x_161 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_162 : f32 = f32(x_161);
    x_GLF_color = vec4<f32>(x_162, x_162, x_162, x_162);
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
