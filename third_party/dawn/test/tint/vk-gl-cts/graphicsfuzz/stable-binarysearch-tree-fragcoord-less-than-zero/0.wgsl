struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

var<private> tree_1 : array<BST, 10u>;

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_i1_(tree : ptr<function, BST>, data : ptr<function, i32>) {
  let x_158 : i32 = *(data);
  (*(tree)).data = x_158;
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
    let x_167 : i32 = baseIndex;
    let x_168 : i32 = *(treeIndex);
    if ((x_167 <= x_168)) {
    } else {
      break;
    }
    let x_171 : i32 = *(data_1);
    let x_172 : i32 = baseIndex;
    let x_174 : i32 = tree_1[x_172].data;
    if ((x_171 <= x_174)) {
      let x_179 : i32 = baseIndex;
      let x_181 : i32 = tree_1[x_179].leftIndex;
      if ((x_181 == -1)) {
        let x_186 : i32 = baseIndex;
        let x_187 : i32 = *(treeIndex);
        tree_1[x_186].leftIndex = x_187;
        let x_189 : i32 = *(treeIndex);
        let x_191 : BST = tree_1[x_189];
        param = x_191;
        let x_192 : i32 = *(data_1);
        param_1 = x_192;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param), &(param_1));
        let x_194 : BST = param;
        tree_1[x_189] = x_194;
        return;
      } else {
        let x_196 : i32 = baseIndex;
        let x_198 : i32 = tree_1[x_196].leftIndex;
        baseIndex = x_198;
        continue;
      }
    } else {
      let x_199 : i32 = baseIndex;
      let x_201 : i32 = tree_1[x_199].rightIndex;
      if ((x_201 == -1)) {
        let x_206 : i32 = baseIndex;
        let x_207 : i32 = *(treeIndex);
        tree_1[x_206].rightIndex = x_207;
        let x_209 : i32 = *(treeIndex);
        let x_211 : BST = tree_1[x_209];
        param_2 = x_211;
        let x_212 : i32 = *(data_1);
        param_3 = x_212;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_2), &(param_3));
        let x_214 : BST = param_2;
        tree_1[x_209] = x_214;
        return;
      } else {
        let x_216 : i32 = baseIndex;
        let x_218 : i32 = tree_1[x_216].rightIndex;
        baseIndex = x_218;
        continue;
      }
    }
  }
  return;
}

fn search_i1_(t: ptr<function, i32>) -> i32 {
  var index : i32;
  var currentNode : BST;
  var x_220 : i32;
  index = 0;
  loop {
    let x_225 : i32 = index;
    if ((x_225 != -1)) {
    } else {
      break;
    }
    let x_228 : i32 = index;
    let x_230 : BST = tree_1[x_228];
    currentNode = x_230;
    let x_232 : i32 = currentNode.data;
    let x_233 : i32 = *(t);
    if ((x_232 == x_233)) {
      let x_237 : i32 = *(t);
      return x_237;
    }
    let x_238 : i32 = *(t);
    let x_240 : i32 = currentNode.data;
    if ((x_238 > x_240)) {
      let x_246 : i32 = currentNode.rightIndex;
      x_220 = x_246;
    } else {
      let x_248 : i32 = currentNode.leftIndex;
      x_220 = x_248;
    }
    let x_249 : i32 = x_220;
    index = x_249;
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
  let x_84 : BST = tree_1[0];
  param_4 = x_84;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_4), &(param_5));
  let x_86 : BST = param_4;
  tree_1[0] = x_86;
  let x_88 : i32 = treeIndex_1;
  treeIndex_1 = (x_88 + 1);
  let x_90 : i32 = treeIndex_1;
  param_6 = x_90;
  param_7 = 5;
  insert_i1_i1_(&(param_6), &(param_7));
  let x_92 : i32 = treeIndex_1;
  treeIndex_1 = (x_92 + 1);
  let x_94 : i32 = treeIndex_1;
  param_8 = x_94;
  param_9 = 12;
  insert_i1_i1_(&(param_8), &(param_9));
  let x_96 : i32 = treeIndex_1;
  treeIndex_1 = (x_96 + 1);
  let x_98 : i32 = treeIndex_1;
  param_10 = x_98;
  param_11 = 15;
  insert_i1_i1_(&(param_10), &(param_11));
  let x_100 : i32 = treeIndex_1;
  treeIndex_1 = (x_100 + 1);
  let x_102 : i32 = treeIndex_1;
  param_12 = x_102;
  param_13 = 7;
  insert_i1_i1_(&(param_12), &(param_13));
  let x_104 : i32 = treeIndex_1;
  treeIndex_1 = (x_104 + 1);
  let x_106 : i32 = treeIndex_1;
  param_14 = x_106;
  param_15 = 8;
  insert_i1_i1_(&(param_14), &(param_15));
  let x_108 : i32 = treeIndex_1;
  treeIndex_1 = (x_108 + 1);
  let x_110 : i32 = treeIndex_1;
  param_16 = x_110;
  param_17 = 2;
  insert_i1_i1_(&(param_16), &(param_17));
  let x_112 : i32 = treeIndex_1;
  treeIndex_1 = (x_112 + 1);
  let x_114 : i32 = treeIndex_1;
  param_18 = x_114;
  param_19 = 6;
  insert_i1_i1_(&(param_18), &(param_19));
  let x_116 : i32 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  let x_118 : i32 = treeIndex_1;
  param_20 = x_118;
  param_21 = 17;
  insert_i1_i1_(&(param_20), &(param_21));
  let x_120 : i32 = treeIndex_1;
  treeIndex_1 = (x_120 + 1);
  let x_122 : i32 = treeIndex_1;
  param_22 = x_122;
  param_23 = 13;
  insert_i1_i1_(&(param_22), &(param_23));
  count = 0;
  i = 0;
  loop {
    let x_128 : i32 = i;
    if ((x_128 < 20)) {
    } else {
      break;
    }
    let x_131 : i32 = i;
    param_24 = x_131;
    let x_132 : i32 = search_i1_(&(param_24));
    result = x_132;
    let x_133 : i32 = i;
    switch(x_133) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        let x_143 : i32 = result;
        let x_144 : i32 = i;
        if ((x_143 == x_144)) {
          let x_148 : i32 = count;
          count = (x_148 + 1);
        }
      }
      default: {
        let x_137 : i32 = result;
        if ((x_137 == -1)) {
          let x_141 : i32 = count;
          count = (x_141 + 1);
        }
      }
    }

    continuing {
      let x_150 : i32 = i;
      i = (x_150 + 1);
    }
  }
  let x_152 : i32 = count;
  if ((x_152 == 20)) {
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
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
