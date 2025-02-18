struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

var<private> tree_1 : array<BST, 10u>;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_i1_(tree : ptr<function, BST>, data : ptr<function, i32>) {
  let x_169 : i32 = *(data);
  (*(tree)).data = x_169;
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
  baseIndex = 0;
  loop {
    let x_178 : i32 = baseIndex;
    let x_179 : i32 = *(treeIndex);
    if ((x_178 <= x_179)) {
    } else {
      break;
    }
    let x_182 : i32 = *(data_1);
    let x_183 : i32 = baseIndex;
    let x_185 : i32 = tree_1[x_183].data;
    if ((x_182 <= x_185)) {
      let x_190 : i32 = baseIndex;
      let x_192 : i32 = tree_1[x_190].leftIndex;
      if ((x_192 == -1)) {
        let x_197 : i32 = baseIndex;
        let x_198 : i32 = *(treeIndex);
        tree_1[x_197].leftIndex = x_198;
        let x_200 : i32 = *(treeIndex);
        let x_202 : BST = tree_1[x_200];
        param = x_202;
        let x_203 : i32 = *(data_1);
        param_1 = x_203;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param), &(param_1));
        let x_205 : BST = param;
        tree_1[x_200] = x_205;
        return;
      } else {
        let x_207 : i32 = baseIndex;
        let x_209 : i32 = tree_1[x_207].leftIndex;
        baseIndex = x_209;
        continue;
      }
    } else {
      let x_210 : i32 = baseIndex;
      let x_212 : i32 = tree_1[x_210].rightIndex;
      if ((x_212 == -1)) {
        let x_217 : i32 = baseIndex;
        let x_218 : i32 = *(treeIndex);
        tree_1[x_217].rightIndex = x_218;
        let x_220 : i32 = *(treeIndex);
        let x_222 : BST = tree_1[x_220];
        param_2 = x_222;
        let x_223 : i32 = *(data_1);
        param_3 = x_223;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_2), &(param_3));
        let x_225 : BST = param_2;
        tree_1[x_220] = x_225;
        return;
      } else {
        let x_227 : i32 = baseIndex;
        let x_229 : i32 = tree_1[x_227].rightIndex;
        baseIndex = x_229;
        continue;
      }
    }
  }
  return;
}

fn search_i1_(t : ptr<function, i32>) -> i32 {
  var index : i32;
  var currentNode : BST;
  var x_231 : i32;
  index = 0;
  loop {
    let x_236 : i32 = index;
    if ((x_236 != -1)) {
    } else {
      break;
    }
    let x_239 : i32 = index;
    let x_241 : BST = tree_1[x_239];
    currentNode = x_241;
    let x_243 : i32 = currentNode.data;
    let x_244 : i32 = *(t);
    if ((x_243 == x_244)) {
      let x_248 : i32 = *(t);
      return x_248;
    }
    let x_249 : i32 = *(t);
    let x_251 : i32 = currentNode.data;
    if ((x_249 > x_251)) {
      let x_257 : i32 = currentNode.rightIndex;
      x_231 = x_257;
    } else {
      let x_259 : i32 = currentNode.leftIndex;
      x_231 = x_259;
    }
    let x_260 : i32 = x_231;
    index = x_260;
  }
  return -1;
}

fn main_1() {
  var treeIndex_1 : i32;
  var param_4 : BST;
  var param_5 : i32;
  var param_6 : i32;
  var param_7 : i32;
  var param_8 : i32;
  var param_9 : i32;
  var param_10 : i32;
  var param_11 : i32;
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
  var count : i32;
  var i : i32;
  var result : i32;
  var param_24 : i32;
  treeIndex_1 = 0;
  let x_88 : BST = tree_1[0];
  param_4 = x_88;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_4), &(param_5));
  let x_90 : BST = param_4;
  tree_1[0] = x_90;
  let x_92 : i32 = treeIndex_1;
  treeIndex_1 = (x_92 + 1);
  let x_94 : i32 = treeIndex_1;
  param_6 = x_94;
  param_7 = 5;
  insert_i1_i1_(&(param_6), &(param_7));
  let x_96 : i32 = treeIndex_1;
  treeIndex_1 = (x_96 + 1);
  let x_98 : i32 = treeIndex_1;
  param_8 = x_98;
  param_9 = 12;
  insert_i1_i1_(&(param_8), &(param_9));
  let x_100 : i32 = treeIndex_1;
  treeIndex_1 = (x_100 + 1);
  let x_102 : i32 = treeIndex_1;
  param_10 = x_102;
  param_11 = 15;
  insert_i1_i1_(&(param_10), &(param_11));
  let x_104 : i32 = treeIndex_1;
  treeIndex_1 = (x_104 + 1);
  let x_106 : i32 = treeIndex_1;
  param_12 = x_106;
  param_13 = 7;
  insert_i1_i1_(&(param_12), &(param_13));
  let x_108 : i32 = treeIndex_1;
  treeIndex_1 = (x_108 + 1);
  let x_110 : i32 = treeIndex_1;
  param_14 = x_110;
  param_15 = 8;
  insert_i1_i1_(&(param_14), &(param_15));
  let x_112 : i32 = treeIndex_1;
  treeIndex_1 = (x_112 + 1);
  let x_114 : i32 = treeIndex_1;
  param_16 = x_114;
  param_17 = 2;
  insert_i1_i1_(&(param_16), &(param_17));
  let x_116 : i32 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  let x_118 : i32 = treeIndex_1;
  param_18 = x_118;
  param_19 = 6;
  insert_i1_i1_(&(param_18), &(param_19));
  let x_120 : i32 = treeIndex_1;
  treeIndex_1 = (x_120 + 1);
  let x_122 : i32 = treeIndex_1;
  param_20 = x_122;
  param_21 = 17;
  insert_i1_i1_(&(param_20), &(param_21));
  let x_124 : i32 = treeIndex_1;
  treeIndex_1 = (x_124 + 1);
  let x_126 : i32 = treeIndex_1;
  param_22 = x_126;
  param_23 = 13;
  insert_i1_i1_(&(param_22), &(param_23));
  count = 0;
  i = 0;
  loop {
    let x_132 : i32 = i;
    if ((x_132 < 20)) {
    } else {
      break;
    }
    var x_155 : bool;
    var x_156_phi : bool;
    let x_135 : i32 = i;
    param_24 = x_135;
    let x_136 : i32 = search_i1_(&(param_24));
    result = x_136;
    let x_137 : i32 = i;
    switch(x_137) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        let x_147 : i32 = result;
        let x_148 : i32 = i;
        let x_149 : bool = (x_147 == x_148);
        x_156_phi = x_149;
        if (!(x_149)) {
          let x_154 : f32 = gl_FragCoord.x;
          x_155 = (x_154 < 0.0);
          x_156_phi = x_155;
        }
        let x_156 : bool = x_156_phi;
        if (x_156) {
          let x_159 : i32 = count;
          count = (x_159 + 1);
        }
      }
      default: {
        let x_141 : i32 = result;
        if ((x_141 == -1)) {
          let x_145 : i32 = count;
          count = (x_145 + 1);
        }
      }
    }

    continuing {
      let x_161 : i32 = i;
      i = (x_161 + 1);
    }
  }
  let x_163 : i32 = count;
  if ((x_163 == 20)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 1.0, 1.0);
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
