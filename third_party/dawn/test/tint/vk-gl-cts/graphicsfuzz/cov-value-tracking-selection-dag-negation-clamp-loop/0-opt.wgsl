struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 16u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var r : array<i32, 15u>;
  var i : i32;
  var data : array<i32, 15u>;
  var i_1 : i32;
  var i_2 : i32;
  var i_3 : i32;
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  r[x_46] = x_48;
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_51] = x_53;
  let x_56 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_58 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_56] = x_58;
  let x_61 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_63 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  r[x_61] = x_63;
  let x_66 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_68 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  r[x_66] = x_68;
  let x_71 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_73 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_71] = -(x_73);
  let x_77 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  let x_79 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_77] = -(x_79);
  let x_83 : i32 = x_6.x_GLF_uniform_int_values[9].el;
  let x_85 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_83] = -(x_85);
  let x_89 : i32 = x_6.x_GLF_uniform_int_values[10].el;
  let x_91 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_89] = -(x_91);
  let x_95 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_97 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_95] = -(x_97);
  let x_101 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_103 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_101] = -(x_103);
  let x_107 : i32 = x_6.x_GLF_uniform_int_values[12].el;
  let x_109 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_107] = -(x_109);
  let x_113 : i32 = x_6.x_GLF_uniform_int_values[13].el;
  let x_115 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_113] = -(x_115);
  let x_119 : i32 = x_6.x_GLF_uniform_int_values[14].el;
  let x_121 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_119] = -(x_121);
  let x_125 : i32 = x_6.x_GLF_uniform_int_values[15].el;
  let x_127 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_125] = -(x_127);
  i = 0;
  loop {
    let x_134 : i32 = i;
    let x_136 : i32 = x_6.x_GLF_uniform_int_values[5].el;
    if ((x_134 < x_136)) {
    } else {
      break;
    }
    let x_139 : i32 = i;
    let x_140 : i32 = i;
    let x_142 : i32 = i;
    let x_145 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    data[x_139] = ~(clamp(~(x_140), ~(x_142), x_145));

    continuing {
      let x_149 : i32 = i;
      i = (x_149 + 1);
    }
  }
  let x_152 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  i_1 = x_152;
  loop {
    let x_157 : i32 = i_1;
    let x_159 : i32 = x_6.x_GLF_uniform_int_values[6].el;
    if ((x_157 < x_159)) {
    } else {
      break;
    }
    let x_162 : i32 = i_1;
    let x_163 : i32 = i_1;
    data[x_162] = ~(clamp(~(x_163), 0, 1));

    continuing {
      let x_168 : i32 = i_1;
      i_1 = (x_168 + 1);
    }
  }
  let x_171 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  i_2 = x_171;
  loop {
    let x_176 : i32 = i_2;
    let x_178 : i32 = x_6.x_GLF_uniform_int_values[7].el;
    if ((x_176 < x_178)) {
    } else {
      break;
    }
    let x_181 : i32 = i_2;
    let x_182 : i32 = i_2;
    data[x_181] = ~(clamp(x_182, 0, 1));

    continuing {
      let x_186 : i32 = i_2;
      i_2 = (x_186 + 1);
    }
  }
  let x_189 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  i_3 = x_189;
  loop {
    let x_194 : i32 = i_3;
    let x_196 : i32 = x_6.x_GLF_uniform_int_values[7].el;
    if ((x_194 < x_196)) {
    } else {
      break;
    }
    let x_199 : i32 = i_3;
    let x_201 : i32 = data[x_199];
    let x_202 : i32 = i_3;
    let x_204 : i32 = r[x_202];
    if ((x_201 != x_204)) {
      let x_209 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      let x_210 : f32 = f32(x_209);
      x_GLF_color = vec4<f32>(x_210, x_210, x_210, x_210);
      return;
    }

    continuing {
      let x_212 : i32 = i_3;
      i_3 = (x_212 + 1);
    }
  }
  let x_215 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_218 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_221 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_224 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  x_GLF_color = vec4<f32>(f32(x_215), f32(x_218), f32(x_221), f32(x_224));
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
