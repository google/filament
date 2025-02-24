struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

struct Obj {
  odd_numbers : array<f32, 10u>,
  even_numbers : array<f32, 10u>,
}

var<private> tree_1 : array<BST, 10u>;

@group(0) @binding(0) var<uniform> x_27 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_i1_(tree : ptr<function, BST>, data : ptr<function, i32>) {
  let x_74 : i32 = *(data);
  (*(tree)).data = x_74;
  (*(tree)).leftIndex = -1;
  (*(tree)).rightIndex = -1;
  return;
}

fn insert_i1_i1_(treeIndex : ptr<function, i32>, data_1 : ptr<function, i32>) {
  var baseIndex : i32;
  var param : BST;
  var param_1 : i32;
  var param_2 : BST;
  var param_3 : i32;
  var GLF_live8i : i32;
  var GLF_live8A : array<f32, 50u>;
  baseIndex = 0;
  loop {
    let x_75 : i32 = baseIndex;
    let x_76 : i32 = *(treeIndex);
    if ((x_75 <= x_76)) {
    } else {
      break;
    }
    let x_77 : i32 = *(data_1);
    let x_78 : i32 = baseIndex;
    let x_79 : i32 = tree_1[x_78].data;
    if ((x_77 <= x_79)) {
      let x_80 : i32 = baseIndex;
      let x_81 : i32 = tree_1[x_80].leftIndex;
      if ((x_81 == -1)) {
        let x_82 : i32 = baseIndex;
        let x_83 : i32 = *(treeIndex);
        tree_1[x_82].leftIndex = x_83;
        let x_84 : i32 = *(treeIndex);
        let x_350 : BST = tree_1[x_84];
        param = x_350;
        let x_85 : i32 = *(data_1);
        param_1 = x_85;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param), &(param_1));
        let x_352 : BST = param;
        tree_1[x_84] = x_352;
        return;
      } else {
        let x_86 : i32 = baseIndex;
        let x_87 : i32 = tree_1[x_86].leftIndex;
        baseIndex = x_87;
        continue;
      }
    } else {
      let x_88 : i32 = baseIndex;
      let x_89 : i32 = tree_1[x_88].rightIndex;
      if ((x_89 == -1)) {
        let x_90 : i32 = baseIndex;
        let x_91 : i32 = *(treeIndex);
        tree_1[x_90].rightIndex = x_91;
        let x_92 : i32 = *(treeIndex);
        let x_362 : BST = tree_1[x_92];
        param_2 = x_362;
        let x_93 : i32 = *(data_1);
        param_3 = x_93;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_2), &(param_3));
        let x_364 : BST = param_2;
        tree_1[x_92] = x_364;
        return;
      } else {
        GLF_live8i = 1;
        let x_94 : i32 = GLF_live8i;
        let x_95 : i32 = GLF_live8i;
        let x_96 : i32 = GLF_live8i;
        let x_369 : i32 = select(0, x_96, ((x_94 >= 0) & (x_95 < 50)));
        let x_371 : f32 = GLF_live8A[0];
        let x_373 : f32 = GLF_live8A[x_369];
        GLF_live8A[x_369] = (x_373 + x_371);
        loop {
          let x_97 : i32 = baseIndex;
          let x_98 : i32 = tree_1[x_97].rightIndex;
          baseIndex = x_98;

          continuing {
            let x_382 : f32 = x_27.injectionSwitch.x;
            let x_384 : f32 = x_27.injectionSwitch.y;
            break if !(x_382 > x_384);
          }
        }
        continue;
      }
    }
  }
  return;
}

fn search_i1_(t : ptr<function, i32>) -> i32 {
  var index : i32;
  var currentNode : BST;
  var x_387 : i32;
  index = 0;
  loop {
    let x_99 : i32 = index;
    if ((x_99 != -1)) {
    } else {
      break;
    }
    let x_100 : i32 = index;
    let x_395 : BST = tree_1[x_100];
    currentNode = x_395;
    let x_101 : i32 = currentNode.data;
    let x_102 : i32 = *(t);
    if ((x_101 == x_102)) {
      let x_103 : i32 = *(t);
      return x_103;
    }
    let x_104 : i32 = *(t);
    let x_105 : i32 = currentNode.data;
    if ((x_104 > x_105)) {
      let x_106 : i32 = currentNode.rightIndex;
      x_387 = x_106;
    } else {
      let x_107 : i32 = currentNode.leftIndex;
      x_387 = x_107;
    }
    let x_108 : i32 = x_387;
    index = x_108;
  }
  return -1;
}

fn makeFrame_f1_(v : ptr<function, f32>) -> f32 {
  var param_5 : i32;
  var param_6 : i32;
  var param_7 : i32;
  let x_418 : f32 = *(v);
  *(v) = (x_418 * 6.5);
  let x_420 : f32 = *(v);
  if ((x_420 < 1.5)) {
    param_5 = 100;
    let x_110 : i32 = search_i1_(&(param_5));
    return f32(x_110);
  }
  let x_425 : f32 = *(v);
  if ((x_425 < 4.0)) {
    return 0.0;
  }
  let x_429 : f32 = *(v);
  param_6 = 6;
  let x_111 : i32 = search_i1_(&(param_6));
  if ((x_429 < f32(x_111))) {
    return 1.0;
  }
  param_7 = 30;
  let x_112 : i32 = search_i1_(&(param_7));
  return (10.0 + f32(x_112));
}

fn hueColor_f1_(angle : ptr<function, f32>) -> vec3<f32> {
  var nodeData : f32;
  var param_4 : i32;
  param_4 = 15;
  let x_109 : i32 = search_i1_(&(param_4));
  nodeData = f32(x_109);
  let x_409 : f32 = *(angle);
  let x_410 : f32 = nodeData;
  return ((vec3<f32>(30.0, 30.0, 30.0) + (vec3<f32>(1.0, 5.0, x_410) * x_409)) / vec3<f32>(50.0, 50.0, 50.0));
}

fn main_1() {
  var treeIndex_1 : i32;
  var param_8 : BST;
  var param_9 : i32;
  var param_10 : i32;
  var param_11 : i32;
  var GLF_live1_looplimiter2 : i32;
  var GLF_live1i : i32;
  var param_12 : i32;
  var param_13 : i32;
  var param_14 : i32;
  var param_15 : i32;
  var param_16 : i32;
  var param_17 : i32;
  var param_18 : i32;
  var param_19 : i32;
  var param_20 : i32;
  var param_21 : i32;
  var param_22 : i32;
  var param_23 : i32;
  var GLF_live4_looplimiter3 : i32;
  var GLF_live4i : i32;
  var GLF_live4index : i32;
  var GLF_live4obj : Obj;
  var param_24 : i32;
  var param_25 : i32;
  var param_26 : i32;
  var param_27 : i32;
  var z : vec2<f32>;
  var x : f32;
  var param_28 : f32;
  var y : f32;
  var param_29 : f32;
  var sum : i32;
  var t_1 : i32;
  var result : i32;
  var param_30 : i32;
  var a : f32;
  var x_235 : vec3<f32>;
  var param_31 : f32;
  treeIndex_1 = 0;
  let x_237 : BST = tree_1[0];
  param_8 = x_237;
  param_9 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_8), &(param_9));
  let x_239 : BST = param_8;
  tree_1[0] = x_239;
  let x_113 : i32 = treeIndex_1;
  treeIndex_1 = (x_113 + 1);
  let x_115 : i32 = treeIndex_1;
  param_10 = x_115;
  param_11 = 5;
  insert_i1_i1_(&(param_10), &(param_11));
  let x_116 : i32 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  GLF_live1_looplimiter2 = 0;
  GLF_live1i = 0;
  loop {
    if (true) {
    } else {
      break;
    }
    let x_118 : i32 = GLF_live1_looplimiter2;
    if ((x_118 >= 7)) {
      break;
    }
    let x_119 : i32 = GLF_live1_looplimiter2;
    GLF_live1_looplimiter2 = (x_119 + 1);

    continuing {
      let x_121 : i32 = GLF_live1i;
      GLF_live1i = (x_121 + 1);
    }
  }
  let x_123 : i32 = treeIndex_1;
  param_12 = x_123;
  param_13 = 12;
  insert_i1_i1_(&(param_12), &(param_13));
  let x_124 : i32 = treeIndex_1;
  treeIndex_1 = (x_124 + 1);
  let x_126 : i32 = treeIndex_1;
  param_14 = x_126;
  param_15 = 15;
  insert_i1_i1_(&(param_14), &(param_15));
  let x_127 : i32 = treeIndex_1;
  treeIndex_1 = (x_127 + 1);
  let x_129 : i32 = treeIndex_1;
  param_16 = x_129;
  param_17 = 7;
  insert_i1_i1_(&(param_16), &(param_17));
  let x_130 : i32 = treeIndex_1;
  treeIndex_1 = (x_130 + 1);
  let x_132 : i32 = treeIndex_1;
  param_18 = x_132;
  param_19 = 8;
  insert_i1_i1_(&(param_18), &(param_19));
  let x_133 : i32 = treeIndex_1;
  treeIndex_1 = (x_133 + 1);
  let x_135 : i32 = treeIndex_1;
  param_20 = x_135;
  param_21 = 2;
  insert_i1_i1_(&(param_20), &(param_21));
  let x_136 : i32 = treeIndex_1;
  treeIndex_1 = (x_136 + 1);
  let x_138 : i32 = treeIndex_1;
  param_22 = x_138;
  param_23 = 6;
  insert_i1_i1_(&(param_22), &(param_23));
  let x_139 : i32 = treeIndex_1;
  treeIndex_1 = (x_139 + 1);
  GLF_live4_looplimiter3 = 0;
  GLF_live4i = 0;
  loop {
    if (true) {
    } else {
      break;
    }
    let x_141 : i32 = GLF_live4_looplimiter3;
    if ((x_141 >= 3)) {
      break;
    }
    let x_142 : i32 = GLF_live4_looplimiter3;
    GLF_live4_looplimiter3 = (x_142 + 1);
    GLF_live4index = 1;
    let x_144 : i32 = GLF_live4index;
    let x_145 : i32 = GLF_live4index;
    let x_146 : i32 = GLF_live4index;
    let x_269 : f32 = GLF_live4obj.even_numbers[1];
    GLF_live4obj.even_numbers[select(0, x_146, ((x_144 >= 0) & (x_145 < 10)))] = x_269;
    let x_147 : i32 = GLF_live4i;
    let x_148 : i32 = GLF_live4i;
    let x_149 : i32 = GLF_live4i;
    GLF_live4obj.even_numbers[select(0, x_149, ((x_147 >= 0) & (x_148 < 10)))] = 1.0;

    continuing {
      let x_150 : i32 = GLF_live4i;
      GLF_live4i = (x_150 + 1);
    }
  }
  let x_152 : i32 = treeIndex_1;
  param_24 = x_152;
  param_25 = 17;
  insert_i1_i1_(&(param_24), &(param_25));
  let x_278 : f32 = x_27.injectionSwitch.x;
  let x_280 : f32 = x_27.injectionSwitch.y;
  if ((x_278 > x_280)) {
    return;
  }
  let x_153 : i32 = treeIndex_1;
  treeIndex_1 = (x_153 + 1);
  let x_155 : i32 = treeIndex_1;
  param_26 = x_155;
  param_27 = 13;
  insert_i1_i1_(&(param_26), &(param_27));
  let x_285 : vec4<f32> = gl_FragCoord;
  z = (vec2<f32>(x_285.y, x_285.x) / vec2<f32>(256.0, 256.0));
  let x_289 : f32 = z.x;
  param_28 = x_289;
  let x_290 : f32 = makeFrame_f1_(&(param_28));
  x = x_290;
  let x_292 : f32 = z.y;
  param_29 = x_292;
  let x_293 : f32 = makeFrame_f1_(&(param_29));
  y = x_293;
  sum = -100;
  t_1 = 0;
  loop {
    let x_156 : i32 = t_1;
    if ((x_156 < 20)) {
    } else {
      break;
    }
    let x_157 : i32 = t_1;
    param_30 = x_157;
    let x_158 : i32 = search_i1_(&(param_30));
    result = x_158;
    let x_159 : i32 = result;
    if ((x_159 > 0)) {
    } else {
      let x_160 : i32 = result;
      switch(x_160) {
        case 0: {
          return;
        }
        case -1: {
          let x_161 : i32 = sum;
          sum = (x_161 + 1);
        }
        default: {
        }
      }
    }

    continuing {
      let x_163 : i32 = t_1;
      t_1 = (x_163 + 1);
    }
  }
  let x_307 : f32 = x;
  let x_308 : f32 = y;
  let x_165 : i32 = sum;
  a = (x_307 + (x_308 * f32(x_165)));
  let x_313 : f32 = x_27.injectionSwitch.x;
  let x_315 : f32 = x_27.injectionSwitch.y;
  if ((x_313 < x_315)) {
    x_235 = vec3<f32>(1.0, 0.0, 0.0);
  } else {
    let x_320 : f32 = a;
    param_31 = x_320;
    let x_321 : vec3<f32> = hueColor_f1_(&(param_31));
    x_235 = x_321;
  }
  let x_322 : vec3<f32> = x_235;
  x_GLF_color = vec4<f32>(x_322.x, x_322.y, x_322.z, 1.0);
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
