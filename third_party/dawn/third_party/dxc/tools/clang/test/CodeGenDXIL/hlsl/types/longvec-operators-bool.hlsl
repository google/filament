// RUN: %dxc -HV 2018 -T lib_6_9 -DNUM=2 %s | FileCheck %s
// RUN: %dxc -HV 2018 -T lib_6_9 -DNUM=5 %s | FileCheck %s
// RUN: %dxc -HV 2018 -T lib_6_9 -DNUM=3 %s | FileCheck %s
// RUN: %dxc -HV 2018 -T lib_6_9 -DNUM=9 %s | FileCheck %s

// Test relevant operators on an assortment bool vector sizes with 6.9 native vectors.
// Bools have a different representation in memory and a smaller set of interesting ops.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// Uses non vector buffer to avoid interacting with that implementation.
// CHECK: %dx.types.ResRet.[[TY:[a-z0-9]*]] = type { [[TYPE:[a-z_0-9]*]]
RWStructuredBuffer< bool > buf;

groupshared vector<bool, NUM> gs_vec1, gs_vec2;
groupshared vector<bool, NUM+1> gs_vec3;


// A mixed-type overload to test overload resolution and mingle different vector element types in ops
// Test assignment operators.
// CHECK-LABEL: define void @"\01?assignments
export void assignments(inout vector<bool, NUM> things[10], bool scales[10]) {

  // Another trick to capture the size.
  // CHECK: [[res:%[0-9]*]] = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{[^,]*}}, i32 [[NUM:[0-9]*]]
  // CHECK: [[scl:%[0-9]*]] = extractvalue %dx.types.ResRet.i32 [[res]], 0
  // CHECK: [[bscl:%[0-9]*]] = icmp ne i32 [[scl]], 0
  bool scalar = buf.Load(NUM);

  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 9
  // CHECK: [[vec9:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add9]]
  // CHECK: [[bvec9:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec9]], zeroinitializer
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 0
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec9]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[add0]]
  things[0] = things[9];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i1> undef, i1 [[bscl]], i32 0
  // CHECK: [[res:%[0-9]*]] = shufflevector <[[NUM]] x i1> [[spt]], <[[NUM]] x i1> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[res]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[add5]]
  things[5] = scalar;

}

// Test arithmetic operators.
// CHECK-LABEL: define void @"\01?arithmetic
export vector<bool, NUM> arithmetic(inout vector<bool, NUM> things[10])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add6]]

  // CHECK: [[bvec0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[svec0:%[0-9]*]] = sext <[[NUM]] x i1> [[bvec0]] to <[[NUM]] x i32>
  // CHECK: [[bsvec0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[svec0]], zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[bsvec0]] to <[[NUM]] x i32>
  res[0] = -things[0];

  // CHECK: [[vec0:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec0]] to <[[NUM]] x i32>
  // CHECK: [[bvec0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec0]] to <[[NUM]] x i32>
  res[1] = +things[0];

  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[vec1:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec1]] to <[[NUM]] x i32>
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[vec2:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec2]] to <[[NUM]] x i32>
  // CHECK: [[res2:%[0-9]*]] = add nuw nsw <[[NUM]] x i32> [[vec2]], [[vec1]]
  // CHECK: [[bres2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res2]], zeroinitializer
  // CHECK: [[res2:%[0-9][0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  res[2] = things[1] + things[2];

  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[vec3:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec3]] to <[[NUM]] x i32>
  // CHECK: [[res3:%[0-9]*]] = sub nsw <[[NUM]] x i32> [[vec2]], [[vec3]]
  // CHECK: [[bres3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res3]], zeroinitializer
  // CHECK: [[res3:%[0-9][0-9]*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  res[3] = things[2] - things[3];

  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[vec4:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec4]] to <[[NUM]] x i32>
  // CHECK: [[res4:%[0-9]*]] = mul nuw nsw <[[NUM]] x i32> [[vec4]], [[vec3]]
  // CHECK: [[bres4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res4]], zeroinitializer
  // CHECK: [[res4:%[0-9][0-9]*]] = zext <[[NUM]] x i1> [[bres4]] to <[[NUM]] x i32>
  res[4] = things[3] * things[4];

  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[vec5:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec5]] to <[[NUM]] x i32>
  // CHECK: [[res5:%[0-9]*]] = sdiv <[[NUM]] x i32> [[vec4]], [[vec5]]
  // CHECK: [[bres5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res5]], zeroinitializer
  // CHECK: [[res5:%[0-9][0-9]*]] = zext <[[NUM]] x i1> [[bres5]] to <[[NUM]] x i32>
  res[5] = things[4] / things[5];

  // CHECK: [[bvec6:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec6]], zeroinitializer
  // CHECK: [[vec6:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec6]] to <[[NUM]] x i32>
  // CHECK: [[res6:%[0-9]*]] = {{[ufs]?rem( fast)?}} <[[NUM]] x i32> [[vec5]], [[vec6]]
  res[6] = things[5] % things[6];

  // Stores into res[]. Previous were for things[] inout.
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 1
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 2
  // CHECK: store <[[NUM]] x i32> [[res2]], <[[NUM]] x i32>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 3
  // CHECK: store <[[NUM]] x i32> [[res3]], <[[NUM]] x i32>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[add5]]
  // CHECK: ret void


  return res;
}

// Test arithmetic operators with scalars.
// CHECK-LABEL: define void @"\01?scarithmetic
export vector<bool, NUM> scarithmetic(inout vector<bool, NUM> things[10], bool scales[10])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add6]]

  // CHECK: [[bvec0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[vec0:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec0]] to <[[NUM]] x i32>
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 0
  // CHECK: [[scl0:%[0-9]*]] = load i32, i32* [[add0]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl0]], i32 0
  // CHECK: [[spt0:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = add <[[NUM]] x i32> [[spt0]], [[vec0]]
  // CHECK: [[bres0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res0]], zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[bres0]] to <[[NUM]] x i32>
  res[0] = things[0] + scales[0];

  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[vec1:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec1]] to <[[NUM]] x i32>
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 1
  // CHECK: [[scl1:%[0-9]*]] = load i32, i32* [[add1]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl1]], i32 0
  // CHECK: [[spt1:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res1:%[0-9]*]] = sub <[[NUM]] x i32> [[vec1]], [[spt1]]
  // CHECK: [[bres1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res1]], zeroinitializer
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  res[1] = things[1] - scales[1];


  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[vec2:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec2]] to <[[NUM]] x i32>
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 2
  // CHECK: [[scl2:%[0-9]*]] = load i32, i32* [[add2]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl2]], i32 0
  // CHECK: [[spt2:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res2:%[0-9]*]] = mul nuw <[[NUM]] x i32> [[spt2]], [[vec2]]
  // CHECK: [[bres2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res2]], zeroinitializer
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  res[2] = things[2] * scales[2];

  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[vec3:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec3]] to <[[NUM]] x i32>
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 3
  // CHECK: [[scl3:%[0-9]*]] = load i32, i32* [[add3]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl3]], i32 0
  // CHECK: [[spt3:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = sdiv <[[NUM]] x i32> [[vec3]], [[spt3]]
  // CHECK: [[bres3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res3]], zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  res[3] = things[3] / scales[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 4
  // CHECK: [[scl4:%[0-9]*]] = load i32, i32* [[add4]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl4]], i32 0
  // CHECK: [[spt4:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[vec4:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec4]] to <[[NUM]] x i32>
  // CHECK: [[res4:%[0-9]*]] = add <[[NUM]] x i32> [[spt4]], [[vec4]]
  // CHECK: [[bres4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res4]], zeroinitializer
  // CHECK: [[res4:%[0-9]*]] = zext <[[NUM]] x i1> [[bres4]] to <[[NUM]] x i32>
  res[4] = scales[4] + things[4];

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 5
  // CHECK: [[scl5:%[0-9]*]] = load i32, i32* [[add5]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl5]], i32 0
  // CHECK: [[spt5:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[vec5:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec5]] to <[[NUM]] x i32>
  // CHECK: [[res5:%[0-9]*]] = sub <[[NUM]] x i32> [[spt5]], [[vec5]]
  // CHECK: [[bres5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res5]], zeroinitializer
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[bres5]] to <[[NUM]] x i32>
  res[5] = scales[5] - things[5];

  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* %scales, i32 0, i32 6
  // CHECK: [[scl6:%[0-9]*]] = load i32, i32* [[add6]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x i32> undef, i32 [[scl6]], i32 0
  // CHECK: [[spt6:%[0-9]*]] = shufflevector <[[NUM]] x i32> [[spt]], <[[NUM]] x i32> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[bvec6:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec6]], zeroinitializer
  // CHECK: [[vec6:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec6]] to <[[NUM]] x i32>
  // CHECK: [[res6:%[0-9]*]] = mul nuw <[[NUM]] x i32> [[spt6]], [[vec6]]
  // CHECK: [[bres6:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res6]], zeroinitializer
  // CHECK: [[res6:%[0-9]*]] = zext <[[NUM]] x i1> [[bres6]] to <[[NUM]] x i32>
  res[6] = scales[6] * things[6];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 1
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 2
  // CHECK: store <[[NUM]] x i32> [[res2]], <[[NUM]] x i32>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 3
  // CHECK: store <[[NUM]] x i32> [[res3]], <[[NUM]] x i32>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 6
  // CHECK: store <[[NUM]] x i32> [[res6]], <[[NUM]] x i32>* [[add6]]
  // CHECK: ret void


  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
// CHECK-LABEL: define void @"\01?logic
export vector<bool, NUM> logic(vector<bool, NUM> truth[10], vector<bool, NUM> consequences[10])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[cmp:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[cmp0:%[0-9]*]] = icmp eq <[[NUM]] x i1> [[cmp]], zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp0]] to <[[NUM]] x i32>
  res[0] = !truth[0];

  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add1]]
  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[bres1:%[0-9]*]] = or <[[NUM]] x i1> [[bvec2]], [[bvec1]]
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  res[1] = truth[1] || truth[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add3]]
  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[bres2:%[0-9]*]] = and <[[NUM]] x i1> [[bvec3]], [[bvec2]]
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  res[2] = truth[2] && truth[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add4]]
  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add5]]
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // MORE STUFF

  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[bvec0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer

  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add1]]
  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[bres4:%[0-9]*]] = icmp eq <[[NUM]] x i1> [[bvec0]], [[bvec1]]
  // CHECK: [[res4:%[0-9]*]] = zext <[[NUM]] x i1> [[bres4]] to <[[NUM]] x i32>
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[bres5:%[0-9]*]] = icmp {{u?}}ne <[[NUM]] x i1> [[bvec1]], [[bvec2]]
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[bres5]] to <[[NUM]] x i32>
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add3]]
  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[bres6:%[0-9]*]] = icmp {{[osu]?}}lt <[[NUM]] x i1> [[bvec2]], [[bvec3]]
  // CHECK: [[res6:%[0-9]*]] = zext <[[NUM]] x i1> [[bres6]] to <[[NUM]] x i32>
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add4]]
  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[bres7:%[0-9]*]] = icmp {{[osu]]?}}gt <[[NUM]] x i1> [[bvec3]], [[bvec4]]
  // CHECK: [[res7:%[0-9]*]] = zext <[[NUM]] x i1> [[bres7]] to <[[NUM]] x i32>
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add5]]
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[bres8:%[0-9]*]] = icmp {{[osu]]?}}le <[[NUM]] x i1> [[bvec4]], [[bvec5]]
  // CHECK: [[res8:%[0-9]*]] = zext <[[NUM]] x i1> [[bres8]] to <[[NUM]] x i32>
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %consequences, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add6]]
  // CHECK: [[bvec6:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec6]], zeroinitializer
  // CHECK: [[bres9:%[0-9]*]] = icmp {{[osu]?}}ge <[[NUM]] x i1> [[bvec5]], [[bvec6]]
  // CHECK: [[res9:%[0-9]*]] = zext <[[NUM]] x i1> [[bres9]] to <[[NUM]] x i32>
  res[9] = consequences[5] >= consequences[6];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 6
  // CHECK: store <[[NUM]] x i32> [[res6]], <[[NUM]] x i32>* [[add6]]
  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 7
  // CHECK: store <[[NUM]] x i32> [[res7]], <[[NUM]] x i32>* [[add7]]
  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 8
  // CHECK: store <[[NUM]] x i32> [[res8]], <[[NUM]] x i32>* [[add8]]
  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 9
  // CHECK: store <[[NUM]] x i32> [[res9]], <[[NUM]] x i32>* [[add9]]
  // CHECK: ret void

  return res;
}

static const int Ix = 2;

// Test indexing operators
// CHECK-LABEL: define void @"\01?index
export vector<bool, NUM> index(vector<bool, NUM> things[10], int i, bool val)[10] {
  vector<bool, NUM> res[10];

  // CHECK: [[res:%[0-9]*]] = alloca [10 x <[[NUM]] x i32>]
  // CHECK: [[res0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> zeroinitializer, <[[NUM]] x i32>* [[res0]]
  res[0] = 0;

  // CHECK: [[resi:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 %i
  // CHECK: store <[[NUM]] x i32> <i32 1{{.*}}>, <[[NUM]] x i32>* [[resi]]
  res[i] = 1;

  // CHECK: [[res2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 2
  // CHECK: store <[[NUM]] x i32> <i32 1{{.*}}>, <[[NUM]] x i32>* [[res2]]
  res[Ix] = true;

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 0
  // CHECK: [[thg0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[bthg0:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[thg0]], zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 3
  // CHECK: [[thg0:%[0-9]*]] = zext <[[NUM]] x i1> [[bthg0]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[thg0]], <[[NUM]] x i32>* [[res3]]
  res[3] = things[0];

  // CHECK: [[addi:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 %i
  // CHECK: [[thgi:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[addi]]
  // CHECK: [[bthgi:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[thgi]], zeroinitializer
  // CHECK: [[res4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 4
  // CHECK: [[thgi:%[0-9]*]] = zext <[[NUM]] x i1> [[bthgi]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[thgi]], <[[NUM]] x i32>* [[res4]]
  res[4] = things[i];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 2
  // CHECK: [[thg2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[bthg2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[thg2]], zeroinitializer
  // CHECK: [[res5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* [[res]], i32 0, i32 5
  // CHECK: [[thg2:%[0-9]*]] = zext <[[NUM]] x i1> [[bthg2]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[thg2]], <[[NUM]] x i32>* [[res5]]
  res[5] = things[Ix];
  // CHECK: ret void
  return res;

}

// Test bit twiddling operators.
// CHECK-LABEL: define void @"\01?bittwiddlers
export void bittwiddlers(inout vector<bool, NUM> things[10]) {
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[vec2:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec2]] to <[[NUM]] x i32>

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[vec3:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec3]] to <[[NUM]] x i32>
  // CHECK: [[res1:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[vec3]], [[vec2]]
  // CHECK: [[bres1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res1]], zeroinitializer
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %things, i32 0, i32 1
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[add1]]
  things[1] = things[2] | things[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[bres2:%[0-9]*]] = and <[[NUM]] x i1> [[bvec4]], [[bvec3]]
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  // CHECK: [[bres2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res2]], zeroinitializer
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[add2]]
  things[2] = things[3] & things[4];

  // CHECK: [[vec4:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec4]] to <[[NUM]] x i32>
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[vec5:%[0-9]*]] = zext <[[NUM]] x i1> [[bvec5]] to <[[NUM]] x i32>
  // CHECK: [[res3:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: [[bres3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[res3]], zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[add3]]
  things[3] = things[4] ^ things[5];

  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add6]]
  // CHECK: [[bvec6:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec6]], zeroinitializer
  // CHECK: [[bres4:%[0-9]*]] = or <[[NUM]] x i1> [[bvec6]], [[bvec4]]
  // CHECK: [[res4:%[0-9]*]] = zext <[[NUM]] x i1> [[bres4]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[add4]]
  things[4] |= things[6];

  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add7]]
  // CHECK: [[bvec7:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec7]], zeroinitializer
  // CHECK: [[bres5:%[0-9]*]] = and <[[NUM]] x i1> [[bvec7]], [[bvec5]]
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[bres5]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[add5]]
  things[5] &= things[7];

  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add8]]
  // CHECK: [[bvec8:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec8]], zeroinitializer
  // CHECK: [[bres6:%[0-9]*]] = xor <[[NUM]] x i1> [[bvec6]], [[bvec8]]
  // CHECK: [[res6:%[0-9]*]] = zext <[[NUM]] x i1> [[bres6]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[add6]]
  things[6] ^= things[8];

  // CHECK: ret void
}
