struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  resolution : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_30 : buf0;

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_92 : i32 = *(i);
  let x_94 : i32 = obj.numbers[x_92];
  temp = x_94;
  let x_95 : i32 = *(i);
  let x_96 : i32 = *(j);
  let x_98 : i32 = obj.numbers[x_96];
  obj.numbers[x_95] = x_98;
  let x_100 : i32 = *(j);
  let x_101 : i32 = temp;
  obj.numbers[x_100] = x_101;
  return;
}

fn performPartition_i1_i1_(l : ptr<function, i32>, h : ptr<function, i32>) -> i32 {
  var pivot : i32;
  var i_1 : i32;
  var j_1 : i32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  let x_104 : i32 = *(h);
  let x_106 : i32 = obj.numbers[x_104];
  pivot = x_106;
  let x_107 : i32 = *(l);
  i_1 = (x_107 - 1);
  let x_109 : i32 = *(l);
  j_1 = x_109;
  loop {
    let x_114 : i32 = j_1;
    let x_115 : i32 = *(h);
    if ((x_114 <= (x_115 - 1))) {
    } else {
      break;
    }
    let x_119 : i32 = j_1;
    let x_121 : i32 = obj.numbers[x_119];
    let x_122 : i32 = pivot;
    if ((x_121 <= x_122)) {
      let x_126 : i32 = i_1;
      i_1 = (x_126 + 1);
      let x_128 : i32 = i_1;
      param = x_128;
      let x_129 : i32 = j_1;
      param_1 = x_129;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_131 : i32 = j_1;
      j_1 = (x_131 + 1);
    }
  }
  let x_133 : i32 = i_1;
  param_2 = (x_133 + 1);
  let x_135 : i32 = *(h);
  param_3 = x_135;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_137 : i32 = i_1;
  return (x_137 + 1);
}

fn quicksort_() {
  var l_1 : i32;
  var h_1 : i32;
  var top : i32;
  var stack : array<i32, 10u>;
  var p : i32;
  var param_4 : i32;
  var param_5 : i32;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  let x_140 : i32 = top;
  let x_141 : i32 = (x_140 + 1);
  top = x_141;
  let x_142 : i32 = l_1;
  stack[x_141] = x_142;
  let x_144 : i32 = top;
  let x_145 : i32 = (x_144 + 1);
  top = x_145;
  let x_146 : i32 = h_1;
  stack[x_145] = x_146;
  loop {
    let x_152 : i32 = top;
    if ((x_152 >= 0)) {
    } else {
      break;
    }
    let x_155 : i32 = top;
    top = (x_155 - 1);
    let x_158 : i32 = stack[x_155];
    h_1 = x_158;
    let x_159 : i32 = top;
    top = (x_159 - 1);
    let x_162 : i32 = stack[x_159];
    l_1 = x_162;
    let x_163 : i32 = l_1;
    param_4 = x_163;
    let x_164 : i32 = h_1;
    param_5 = x_164;
    let x_165 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_165;
    let x_166 : i32 = p;
    let x_168 : i32 = l_1;
    if (((x_166 - 1) > x_168)) {
      let x_172 : i32 = top;
      let x_173 : i32 = (x_172 + 1);
      top = x_173;
      let x_174 : i32 = l_1;
      stack[x_173] = x_174;
      let x_176 : i32 = top;
      let x_177 : i32 = (x_176 + 1);
      top = x_177;
      let x_178 : i32 = p;
      stack[x_177] = (x_178 - 1);
    }
    let x_181 : i32 = p;
    let x_183 : i32 = h_1;
    if (((x_181 + 1) < x_183)) {
      let x_187 : i32 = top;
      let x_188 : i32 = (x_187 + 1);
      top = x_188;
      let x_189 : i32 = p;
      stack[x_188] = (x_189 + 1);
      let x_192 : i32 = top;
      let x_193 : i32 = (x_192 + 1);
      top = x_193;
      let x_194 : i32 = h_1;
      stack[x_193] = x_194;
    }
  }
  return;
}

fn main_1() {
  var i_2 : i32;
  i_2 = 0;
  loop {
    let x_64 : i32 = i_2;
    if ((x_64 < 10)) {
    } else {
      break;
    }
    let x_67 : i32 = i_2;
    let x_68 : i32 = i_2;
    obj.numbers[x_67] = (10 - x_68);
    let x_71 : i32 = i_2;
    let x_72 : i32 = i_2;
    let x_74 : i32 = obj.numbers[x_72];
    let x_75 : i32 = i_2;
    let x_77 : i32 = obj.numbers[x_75];
    obj.numbers[x_71] = (x_74 * x_77);

    continuing {
      let x_80 : i32 = i_2;
      i_2 = (x_80 + 1);
    }
  }
  quicksort_();
  let x_84 : i32 = obj.numbers[0];
  let x_86 : i32 = obj.numbers[4];
  if ((x_84 < x_86)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 1.0, 0.0, 1.0);
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
