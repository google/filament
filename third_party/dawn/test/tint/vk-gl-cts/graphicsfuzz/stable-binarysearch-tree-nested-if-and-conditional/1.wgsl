struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> tree_1 : array<BST, 10u>;

@group(0) @binding(0) var<uniform> x_16 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_i1_(tree : ptr<function, BST>, data : ptr<function, i32>) {
  let x_165 : i32 = *(data);
  (*(tree)).data = x_165;
  (*(tree)).leftIndex = -1;
  (*(tree)).rightIndex = -1;
  return;
}

fn insert_i1_i1_(treeIndex : ptr<function, i32>, data_1 : ptr<function, i32>) {
  var baseIndex : i32;
  var param : BST;
  var param_1 : i32;
  var x_170 : i32;
  var param_2 : BST;
  var param_3 : i32;
  baseIndex = 0;
  loop {
    let x_175 : i32 = baseIndex;
    let x_176 : i32 = *(treeIndex);
    if ((x_175 <= x_176)) {
    } else {
      break;
    }
    let x_179 : i32 = *(data_1);
    let x_180 : i32 = baseIndex;
    let x_182 : i32 = tree_1[x_180].data;
    if ((x_179 <= x_182)) {
      let x_187 : i32 = baseIndex;
      let x_189 : i32 = tree_1[x_187].leftIndex;
      if ((x_189 == -1)) {
        let x_194 : i32 = baseIndex;
        let x_195 : i32 = *(treeIndex);
        tree_1[x_194].leftIndex = x_195;
        let x_198 : f32 = x_16.injectionSwitch.x;
        let x_200 : f32 = x_16.injectionSwitch.y;
        if ((x_198 < x_200)) {
          let x_204 : i32 = *(treeIndex);
          let x_206 : BST = tree_1[x_204];
          param = x_206;
          let x_207 : i32 = *(data_1);
          param_1 = x_207;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param), &(param_1));
          let x_209 : BST = param;
          tree_1[x_204] = x_209;
        }
        let x_212 : f32 = x_16.injectionSwitch.x;
        let x_214 : f32 = x_16.injectionSwitch.y;
        if ((x_212 < x_214)) {
          return;
        }
      } else {
        let x_218 : i32 = baseIndex;
        let x_220 : i32 = tree_1[x_218].leftIndex;
        baseIndex = x_220;
        continue;
      }
    } else {
      let x_222 : f32 = x_16.injectionSwitch.x;
      let x_224 : f32 = x_16.injectionSwitch.y;
      if ((x_222 < x_224)) {
        let x_229 : i32 = baseIndex;
        let x_231 : i32 = tree_1[x_229].rightIndex;
        x_170 = x_231;
      } else {
        let x_232 : i32 = baseIndex;
        let x_234 : i32 = tree_1[x_232].rightIndex;
        x_170 = x_234;
      }
      let x_235 : i32 = x_170;
      if ((x_235 == -1)) {
        let x_240 : i32 = baseIndex;
        let x_241 : i32 = *(treeIndex);
        tree_1[x_240].rightIndex = x_241;
        let x_243 : i32 = *(treeIndex);
        let x_245 : BST = tree_1[x_243];
        param_2 = x_245;
        let x_246 : i32 = *(data_1);
        param_3 = x_246;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_2), &(param_3));
        let x_248 : BST = param_2;
        tree_1[x_243] = x_248;
        return;
      } else {
        let x_250 : i32 = baseIndex;
        let x_252 : i32 = tree_1[x_250].rightIndex;
        baseIndex = x_252;
        continue;
      }
    }
    let x_254 : f32 = x_16.injectionSwitch.x;
    let x_256 : f32 = x_16.injectionSwitch.y;
    if ((x_254 > x_256)) {
      return;
    }
  }
  return;
}

fn search_i1_(t : ptr<function, i32>) -> i32 {
  var index : i32;
  var currentNode : BST;
  var x_261 : i32;
  index = 0;
  loop {
    let x_266 : i32 = index;
    if ((x_266 != -1)) {
    } else {
      break;
    }
    let x_269 : i32 = index;
    let x_271 : BST = tree_1[x_269];
    currentNode = x_271;
    let x_273 : i32 = currentNode.data;
    let x_274 : i32 = *(t);
    if ((x_273 == x_274)) {
      let x_278 : i32 = *(t);
      return x_278;
    }
    let x_279 : i32 = *(t);
    let x_281 : i32 = currentNode.data;
    if ((x_279 > x_281)) {
      let x_287 : i32 = currentNode.rightIndex;
      x_261 = x_287;
    } else {
      let x_289 : i32 = currentNode.leftIndex;
      x_261 = x_289;
    }
    let x_290 : i32 = x_261;
    index = x_290;
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
  let x_91 : BST = tree_1[0];
  param_4 = x_91;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_4), &(param_5));
  let x_93 : BST = param_4;
  tree_1[0] = x_93;
  let x_95 : i32 = treeIndex_1;
  treeIndex_1 = (x_95 + 1);
  let x_97 : i32 = treeIndex_1;
  param_6 = x_97;
  param_7 = 5;
  insert_i1_i1_(&(param_6), &(param_7));
  let x_99 : i32 = treeIndex_1;
  treeIndex_1 = (x_99 + 1);
  let x_101 : i32 = treeIndex_1;
  param_8 = x_101;
  param_9 = 12;
  insert_i1_i1_(&(param_8), &(param_9));
  let x_103 : i32 = treeIndex_1;
  treeIndex_1 = (x_103 + 1);
  let x_105 : i32 = treeIndex_1;
  param_10 = x_105;
  param_11 = 15;
  insert_i1_i1_(&(param_10), &(param_11));
  let x_107 : i32 = treeIndex_1;
  treeIndex_1 = (x_107 + 1);
  let x_109 : i32 = treeIndex_1;
  param_12 = x_109;
  param_13 = 7;
  insert_i1_i1_(&(param_12), &(param_13));
  let x_111 : i32 = treeIndex_1;
  treeIndex_1 = (x_111 + 1);
  let x_113 : i32 = treeIndex_1;
  param_14 = x_113;
  param_15 = 8;
  insert_i1_i1_(&(param_14), &(param_15));
  let x_115 : i32 = treeIndex_1;
  treeIndex_1 = (x_115 + 1);
  let x_117 : i32 = treeIndex_1;
  param_16 = x_117;
  param_17 = 2;
  insert_i1_i1_(&(param_16), &(param_17));
  let x_119 : i32 = treeIndex_1;
  treeIndex_1 = (x_119 + 1);
  let x_121 : i32 = treeIndex_1;
  param_18 = x_121;
  param_19 = 6;
  insert_i1_i1_(&(param_18), &(param_19));
  let x_123 : i32 = treeIndex_1;
  treeIndex_1 = (x_123 + 1);
  let x_125 : i32 = treeIndex_1;
  param_20 = x_125;
  param_21 = 17;
  insert_i1_i1_(&(param_20), &(param_21));
  let x_127 : i32 = treeIndex_1;
  treeIndex_1 = (x_127 + 1);
  let x_129 : i32 = treeIndex_1;
  param_22 = x_129;
  param_23 = 13;
  insert_i1_i1_(&(param_22), &(param_23));
  count = 0;
  i = 0;
  loop {
    let x_135 : i32 = i;
    if ((x_135 < 20)) {
    } else {
      break;
    }
    let x_138 : i32 = i;
    param_24 = x_138;
    let x_139 : i32 = search_i1_(&(param_24));
    result = x_139;
    let x_140 : i32 = i;
    switch(x_140) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        let x_150 : i32 = result;
        let x_151 : i32 = i;
        if ((x_150 == x_151)) {
          let x_155 : i32 = count;
          count = (x_155 + 1);
        }
      }
      default: {
        let x_144 : i32 = result;
        if ((x_144 == -1)) {
          let x_148 : i32 = count;
          count = (x_148 + 1);
        }
      }
    }

    continuing {
      let x_157 : i32 = i;
      i = (x_157 + 1);
    }
  }
  let x_159 : i32 = count;
  if ((x_159 == 20)) {
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
