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
  loop {
    let x_174 : f32 = x_8.v1.x;
    let x_176 : f32 = x_8.v1.y;
    if ((x_174 > x_176)) {
    } else {
      break;
    }
    return;
  }
  let x_180 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  (*(s)).data = x_180;
  return;
}

fn main_1() {
  var i : i32;
  var arr : array<S, 3u>;
  var i_1 : i32;
  var param : S;
  var j : i32;
  var x_136 : bool;
  var x_146 : bool;
  var x_137_phi : bool;
  var x_147_phi : bool;
  let x_46 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  i = x_46;
  loop {
    let x_51 : i32 = i;
    let x_53 : i32 = x_10.x_GLF_uniform_int_values[1].el;
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
    let x_69 : i32 = x_10.x_GLF_uniform_int_values[1].el;
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
    let x_83 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_81 == x_83)) {
      let x_88 : i32 = i_1;
      let x_91 : i32 = x_10.x_GLF_uniform_int_values[3].el;
      arr[clamp(x_88, 0, 3)].data = x_91;
      let x_94 : S = arr[2];
      param = x_94;
      func_struct_S_i11_(&(param));
      let x_96 : S = param;
      arr[2] = x_96;
    } else {
      let x_99 : i32 = x_10.x_GLF_uniform_int_values[2].el;
      j = x_99;
      loop {
        let x_104 : i32 = j;
        let x_106 : i32 = x_10.x_GLF_uniform_int_values[1].el;
        if ((x_104 < x_106)) {
        } else {
          break;
        }
        let x_109 : i32 = j;
        let x_111 : i32 = arr[x_109].data;
        let x_113 : i32 = x_10.x_GLF_uniform_int_values[4].el;
        if ((x_111 > x_113)) {
          discard;
        }

        continuing {
          let x_117 : i32 = j;
          j = (x_117 + 1);
        }
      }
    }

    continuing {
      let x_119 : i32 = i_1;
      i_1 = (x_119 + 1);
    }
  }
  let x_122 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  let x_124 : i32 = arr[x_122].data;
  let x_126 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  let x_127 : bool = (x_124 == x_126);
  x_137_phi = x_127;
  if (x_127) {
    let x_131 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_133 : i32 = arr[x_131].data;
    let x_135 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    x_136 = (x_133 == x_135);
    x_137_phi = x_136;
  }
  let x_137 : bool = x_137_phi;
  x_147_phi = x_137;
  if (x_137) {
    let x_141 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_143 : i32 = arr[x_141].data;
    let x_145 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_146 = (x_143 == x_145);
    x_147_phi = x_146;
  }
  let x_147 : bool = x_147_phi;
  if (x_147) {
    let x_152 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_155 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_158 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_161 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_152), f32(x_155), f32(x_158), f32(x_161));
  } else {
    let x_165 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_166 : f32 = f32(x_165);
    x_GLF_color = vec4<f32>(x_166, x_166, x_166, x_166);
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
