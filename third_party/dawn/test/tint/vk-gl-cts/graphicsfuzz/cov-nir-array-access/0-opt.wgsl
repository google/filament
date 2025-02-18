struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 19u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var A : array<i32, 17u>;
  var r : array<i32, 17u>;
  var a : i32;
  var i : i32;
  var ok : bool;
  var i_1 : i32;
  let x_52 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_54 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_56 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_58 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_60 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_64 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_66 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_68 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_70 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_72 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_74 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_76 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_78 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_80 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_82 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_84 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  A = array<i32, 17u>(x_52, x_54, x_56, x_58, x_60, x_62, x_64, x_66, x_68, x_70, x_72, x_74, x_76, x_78, x_80, x_82, x_84);
  let x_87 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_89 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_91 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_93 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_95 : i32 = x_6.x_GLF_uniform_int_values[7].el;
  let x_97 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  let x_99 : i32 = x_6.x_GLF_uniform_int_values[9].el;
  let x_101 : i32 = x_6.x_GLF_uniform_int_values[10].el;
  let x_103 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_105 : i32 = x_6.x_GLF_uniform_int_values[12].el;
  let x_107 : i32 = x_6.x_GLF_uniform_int_values[13].el;
  let x_109 : i32 = x_6.x_GLF_uniform_int_values[14].el;
  let x_111 : i32 = x_6.x_GLF_uniform_int_values[15].el;
  let x_113 : i32 = x_6.x_GLF_uniform_int_values[16].el;
  let x_115 : i32 = x_6.x_GLF_uniform_int_values[17].el;
  let x_117 : i32 = x_6.x_GLF_uniform_int_values[18].el;
  let x_119 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  r = array<i32, 17u>(x_87, x_89, x_91, x_93, x_95, x_97, x_99, x_101, x_103, x_105, x_107, x_109, x_111, x_113, x_115, x_117, x_119);
  let x_122 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_122;
  let x_124 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_124;
  loop {
    let x_129 : i32 = i;
    let x_131 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_129 < x_131)) {
    } else {
      break;
    }
    let x_134 : i32 = i;
    let x_135 : i32 = a;
    a = (x_135 - 1);
    A[x_134] = x_135;
    let x_138 : i32 = i;
    let x_140 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_142 : i32 = x_6.x_GLF_uniform_int_values[18].el;
    let x_144 : i32 = i;
    let x_146 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    A[clamp(x_138, x_140, x_142)] = (x_144 + x_146);

    continuing {
      let x_149 : i32 = i;
      i = (x_149 + 1);
    }
  }
  ok = true;
  let x_152 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i_1 = x_152;
  loop {
    let x_157 : i32 = i_1;
    let x_159 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_157 < x_159)) {
    } else {
      break;
    }
    let x_162 : i32 = i_1;
    let x_164 : i32 = A[x_162];
    let x_165 : i32 = i_1;
    let x_167 : i32 = r[x_165];
    if ((x_164 != x_167)) {
      ok = false;
    }

    continuing {
      let x_171 : i32 = i_1;
      i_1 = (x_171 + 1);
    }
  }
  let x_174 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_175 : f32 = f32(x_174);
  x_GLF_color = vec4<f32>(x_175, x_175, x_175, x_175);
  let x_177 : bool = ok;
  if (x_177) {
    let x_181 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_184 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_187 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_190 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_181), f32(x_184), f32(x_187), f32(x_190));
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
