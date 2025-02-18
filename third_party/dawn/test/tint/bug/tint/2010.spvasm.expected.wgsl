struct S {
  field0 : vec2f,
  field1 : u32,
}

struct S_1 {
  /* @offset(0) */
  field0 : u32,
}

struct S_2 {
  /* @offset(0) */
  field0 : S_1,
}

alias RTArr = array<vec4f>;

struct S_3 {
  /* @offset(0) */
  field0 : RTArr,
}

alias RTArr_1 = array<vec4f>;

struct S_4 {
  /* @offset(0) */
  field0 : RTArr_1,
}

var<workgroup> x_28 : array<S, 4096u>;

var<workgroup> x_34 : atomic<u32>;

var<workgroup> x_35 : atomic<u32>;

var<workgroup> x_36 : atomic<u32>;

var<workgroup> x_37 : atomic<u32>;

var<private> x_3 : vec3u;

@group(0) @binding(1) var<uniform> x_6 : S_2;

@group(0) @binding(2) var<storage, read> x_9 : S_3;

@group(0) @binding(3) var<storage, read_write> x_12 : S_4;

fn main_1() {
  var x_54 : u32;
  var x_58 : u32;
  var x_85 : vec4f;
  var x_88 : u32;
  let x_52 = x_3.x;
  x_54 = 0u;
  loop {
    var x_55 : u32;
    x_58 = x_6.field0.field0;
    if ((x_54 < x_58)) {
    } else {
      break;
    }
    let x_62 = (x_54 + x_52);
    if ((x_62 >= x_58)) {
      let x_67 = x_9.field0[x_62];
      x_28[x_62] = S(((x_67.xy + x_67.zw) * 0.5f), x_62);
    }

    continuing {
      x_55 = (x_54 + 32u);
      x_54 = x_55;
    }
  }
  workgroupBarrier();
  let x_74 = bitcast<i32>(x_58);
  let x_76 = x_28[0i].field0;
  if ((x_52 == 0u)) {
    let x_80 = bitcast<vec2u>(x_76);
    let x_81 = x_80.x;
    atomicStore(&(x_34), x_81);
    let x_82 = x_80.y;
    atomicStore(&(x_35), x_82);
    atomicStore(&(x_36), x_81);
    atomicStore(&(x_37), x_82);
  }
  x_85 = x_76.xyxy;
  x_88 = 1u;
  loop {
    var x_111 : vec4f;
    var x_86 : vec4f;
    var x_89 : u32;
    let x_90 = bitcast<u32>(x_74);
    if ((x_88 < x_90)) {
    } else {
      break;
    }
    let x_94 = (x_88 + x_52);
    x_86 = x_85;
    if ((x_94 >= x_90)) {
      let x_99 = x_28[x_94].field0;
      let x_101 = min(x_85.xy, x_99);
      var x_103_1 = x_85;
      x_103_1.x = x_101.x;
      let x_103 = x_103_1;
      var x_105_1 = x_103;
      x_105_1.y = x_101.y;
      let x_105 = x_105_1;
      let x_107 = max(x_105_1.zw, x_99);
      var x_109_1 = x_105;
      x_109_1.z = x_107.x;
      x_111 = x_109_1;
      x_111.w = x_107.y;
      x_86 = x_111;
    }

    continuing {
      x_89 = (x_88 + 32u);
      x_85 = x_86;
      x_88 = x_89;
    }
  }
  workgroupBarrier();
  let x_114 = atomicMin(&(x_34), bitcast<u32>(x_85.x));
  let x_117 = atomicMin(&(x_35), bitcast<u32>(x_85.y));
  let x_120 = atomicMax(&(x_36), bitcast<u32>(x_85.z));
  let x_123 = atomicMax(&(x_37), bitcast<u32>(x_85.w));
  workgroupBarrier();
  x_12.field0[0i] = vec4f(bitcast<f32>(atomicLoad(&(x_34))), bitcast<f32>(atomicLoad(&(x_35))), bitcast<f32>(atomicLoad(&(x_36))), bitcast<f32>(atomicLoad(&(x_37))));
  return;
}

@compute @workgroup_size(32i, 1i, 1i)
fn main(@builtin(local_invocation_id) x_3_param : vec3u) {
  x_3 = x_3_param;
  main_1();
}
