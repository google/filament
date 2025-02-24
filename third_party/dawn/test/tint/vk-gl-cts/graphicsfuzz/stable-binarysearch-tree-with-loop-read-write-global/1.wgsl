struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> tree : array<BST, 10u>;

@group(0) @binding(0) var<uniform> x_50 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_i1_(node : ptr<function, BST>, data : ptr<function, i32>) {
  let x_208 : i32 = *(data);
  (*(node)).data = x_208;
  (*(node)).leftIndex = -1;
  (*(node)).rightIndex = -1;
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
    let x_217 : i32 = baseIndex;
    let x_218 : i32 = *(treeIndex);
    if ((x_217 <= x_218)) {
    } else {
      break;
    }
    let x_221 : i32 = *(data_1);
    let x_222 : i32 = baseIndex;
    let x_224 : i32 = tree[x_222].data;
    if ((x_221 <= x_224)) {
      let x_229 : i32 = baseIndex;
      let x_231 : i32 = tree[x_229].leftIndex;
      if ((x_231 == -1)) {
        let x_236 : i32 = baseIndex;
        let x_237 : i32 = *(treeIndex);
        tree[x_236].leftIndex = x_237;
        let x_239 : i32 = *(treeIndex);
        let x_241 : BST = tree[x_239];
        param = x_241;
        let x_242 : i32 = *(data_1);
        param_1 = x_242;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param), &(param_1));
        let x_244 : BST = param;
        tree[x_239] = x_244;
        return;
      } else {
        let x_246 : i32 = baseIndex;
        let x_248 : i32 = tree[x_246].leftIndex;
        baseIndex = x_248;
        continue;
      }
    } else {
      let x_249 : i32 = baseIndex;
      let x_251 : i32 = tree[x_249].rightIndex;
      if ((x_251 == -1)) {
        let x_256 : i32 = baseIndex;
        let x_257 : i32 = *(treeIndex);
        tree[x_256].rightIndex = x_257;
        let x_259 : i32 = *(treeIndex);
        let x_261 : BST = tree[x_259];
        param_2 = x_261;
        let x_262 : i32 = *(data_1);
        param_3 = x_262;
        makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_2), &(param_3));
        let x_264 : BST = param_2;
        tree[x_259] = x_264;
        return;
      } else {
        let x_266 : i32 = baseIndex;
        let x_268 : i32 = tree[x_266].rightIndex;
        baseIndex = x_268;
        continue;
      }
    }
  }
  return;
}

fn identity_i1_(a : ptr<function, i32>) -> i32 {
  let x_202 : i32 = *(a);
  let x_203 : i32 = *(a);
  obj.numbers[x_202] = x_203;
  let x_206 : i32 = obj.numbers[2];
  return x_206;
}

fn search_i1_(t : ptr<function, i32>) -> i32 {
  var index : i32;
  var currentNode : BST;
  var x_270 : i32;
  index = 0;
  loop {
    let x_275 : i32 = index;
    if ((x_275 != -1)) {
    } else {
      break;
    }
    let x_278 : i32 = index;
    let x_280 : BST = tree[x_278];
    currentNode = x_280;
    let x_282 : i32 = currentNode.data;
    let x_283 : i32 = *(t);
    if ((x_282 == x_283)) {
      let x_287 : i32 = *(t);
      return x_287;
    }
    let x_288 : i32 = *(t);
    let x_290 : i32 = currentNode.data;
    if ((x_288 > x_290)) {
      let x_296 : i32 = currentNode.rightIndex;
      x_270 = x_296;
    } else {
      let x_298 : i32 = currentNode.leftIndex;
      x_270 = x_298;
    }
    let x_299 : i32 = x_270;
    index = x_299;
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
  var pp : i32;
  var looplimiter0 : i32;
  var i : i32;
  var param_24 : i32;
  var count : i32;
  var i_1 : i32;
  var result : i32;
  var param_25 : i32;
  treeIndex_1 = 0;
  let x_101 : BST = tree[0];
  param_4 = x_101;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(&(param_4), &(param_5));
  let x_103 : BST = param_4;
  tree[0] = x_103;
  let x_105 : i32 = treeIndex_1;
  treeIndex_1 = (x_105 + 1);
  let x_107 : i32 = treeIndex_1;
  param_6 = x_107;
  param_7 = 5;
  insert_i1_i1_(&(param_6), &(param_7));
  let x_109 : i32 = treeIndex_1;
  treeIndex_1 = (x_109 + 1);
  let x_111 : i32 = treeIndex_1;
  param_8 = x_111;
  param_9 = 12;
  insert_i1_i1_(&(param_8), &(param_9));
  let x_113 : i32 = treeIndex_1;
  treeIndex_1 = (x_113 + 1);
  let x_115 : i32 = treeIndex_1;
  param_10 = x_115;
  param_11 = 15;
  insert_i1_i1_(&(param_10), &(param_11));
  let x_117 : i32 = treeIndex_1;
  treeIndex_1 = (x_117 + 1);
  let x_119 : i32 = treeIndex_1;
  param_12 = x_119;
  param_13 = 7;
  insert_i1_i1_(&(param_12), &(param_13));
  let x_121 : i32 = treeIndex_1;
  treeIndex_1 = (x_121 + 1);
  let x_123 : i32 = treeIndex_1;
  param_14 = x_123;
  param_15 = 8;
  insert_i1_i1_(&(param_14), &(param_15));
  let x_125 : i32 = treeIndex_1;
  treeIndex_1 = (x_125 + 1);
  let x_127 : i32 = treeIndex_1;
  param_16 = x_127;
  param_17 = 2;
  insert_i1_i1_(&(param_16), &(param_17));
  let x_129 : i32 = treeIndex_1;
  treeIndex_1 = (x_129 + 1);
  let x_131 : i32 = treeIndex_1;
  param_18 = x_131;
  param_19 = 6;
  insert_i1_i1_(&(param_18), &(param_19));
  let x_133 : i32 = treeIndex_1;
  treeIndex_1 = (x_133 + 1);
  let x_135 : i32 = treeIndex_1;
  param_20 = x_135;
  param_21 = 17;
  insert_i1_i1_(&(param_20), &(param_21));
  let x_137 : i32 = treeIndex_1;
  treeIndex_1 = (x_137 + 1);
  let x_139 : i32 = treeIndex_1;
  param_22 = x_139;
  param_23 = 13;
  insert_i1_i1_(&(param_22), &(param_23));
  pp = 0;
  looplimiter0 = 0;
  i = 0;
  loop {
    let x_145 : i32 = i;
    if ((x_145 < 10000)) {
    } else {
      break;
    }
    let x_148 : i32 = looplimiter0;
    let x_150 : f32 = x_50.injectionSwitch.y;
    if ((x_148 >= i32(x_150))) {
      let x_156 : f32 = x_50.injectionSwitch.y;
      param_24 = (1 + i32(x_156));
      let x_159 : i32 = identity_i1_(&(param_24));
      pp = x_159;
      break;
    }
    let x_160 : i32 = looplimiter0;
    looplimiter0 = (x_160 + 1);

    continuing {
      let x_162 : i32 = i;
      i = (x_162 + 1);
    }
  }
  let x_164 : i32 = pp;
  if ((x_164 != 2)) {
    return;
  }
  count = 0;
  i_1 = 0;
  loop {
    let x_172 : i32 = i_1;
    if ((x_172 < 20)) {
    } else {
      break;
    }
    let x_175 : i32 = i_1;
    param_25 = x_175;
    let x_176 : i32 = search_i1_(&(param_25));
    result = x_176;
    let x_177 : i32 = i_1;
    switch(x_177) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        let x_187 : i32 = result;
        let x_188 : i32 = i_1;
        if ((x_187 == x_188)) {
          let x_192 : i32 = count;
          count = (x_192 + 1);
        }
      }
      default: {
        let x_181 : i32 = result;
        if ((x_181 == -1)) {
          let x_185 : i32 = count;
          count = (x_185 + 1);
        }
      }
    }

    continuing {
      let x_194 : i32 = i_1;
      i_1 = (x_194 + 1);
    }
  }
  let x_196 : i32 = count;
  if ((x_196 == 20)) {
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
