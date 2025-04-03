// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=float     -DNUM=4 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=int       -DNUM=7 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=double    -DNUM=16 -DDBL %s | FileCheck %s --check-prefixes=CHECK
// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=uint64_t  -DNUM=17 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=float16_t -DNUM=34 -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -fcgl -HV 2018 -T lib_6_9 -DTYPE=int16_t   -DNUM=129 -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL

// Mainly a source for the longvec scalarizer IR test.
// Serves to verify some codegen as well.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// CHECK: %"class.RWStructuredBuffer<{{.*}}>" = type { [[TYPE:[a-z0-9]*]] }
// CHECK: external global {{\[}}[[NUM:[0-9]*]] x %"class.RWStructuredBuffer
RWStructuredBuffer<TYPE> buf[NUM];


// Test assignment operators.
// CHECK-LABEL: define void @"\01?assignments
export void assignments(inout vector<TYPE, NUM> things[10]) {

  // CHECK: [[res0:%.*]] =  call [[TYPE]] @"dx.hl.op.ro.[[TYPE]] (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle {{%.*}}, i32 1)
  // CHECK: [[vec0:%.*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[res0]], i32 0
  // CHECK: [[res0:%.*]] = shufflevector <[[NUM]] x [[TYPE]]> [[vec0]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res0]], <[[NUM]] x [[TYPE]]>* [[adr0]]
  things[0] = buf[0].Load(1);

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // CHECK: [[res1:%.*]] = [[ADD:f?add( fast)?]] <[[NUM]] x [[TYPE]]> [[vec1]], [[vec5]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[adr1]]
  things[1] += things[5];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr6]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[res2:%.*]] = [[SUB:f?sub( fast)?]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec6]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[adr2]]
  things[2] -= things[6];

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr7]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[res3:%.*]] = [[MUL:f?mul( fast)?]] <[[NUM]] x [[TYPE]]> [[vec3]], [[vec7]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[adr3]]
  things[3] *= things[7];

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr8]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[res4:%.*]] = [[DIV:[ufs]?div( fast)?]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec8]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[adr4]]
  things[4] /= things[8];

#ifndef DBL
  // NODBL: [[adr9:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // NODBL: [[vec9:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr9]]
  // NODBL: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // NODBL: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // NODBL: [[res5:%.*]] = [[REM:[ufs]?rem( fast)?]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec9]]
  // NODBL: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[adr5]]
  things[5] %= things[9];
#endif
}

// Test arithmetic operators.
// CHECK-LABEL: define void @"\01?arithmetic
export vector<TYPE, NUM> arithmetic(inout vector<TYPE, NUM> things[11])[11] {
  vector<TYPE, NUM> res[11];
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: [[res1:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr0]]
  // CHECK: [[res0:%.*]] = [[SUB]] <[[NUM]] x [[TYPE]]>
  res[0] = -things[0];
  res[1] = +things[0];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[res2:%.*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec1]], [[vec2]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[adr2]]
  res[2] = things[1] + things[2];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[res3:%.*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[adr3]]
  res[3] = things[2] - things[3];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[res4:%.*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[vec3]], [[vec4]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[adr4]]
  res[4] = things[3] * things[4];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // CHECK: [[res5:%.*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[adr5]]
  res[5] = things[4] / things[5];

#ifndef DBL
  // NODBL: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // NODBL: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // NODBL: [[adr6:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // NODBL: [[vec6:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr6]]
  // NODBL: [[res6:%.*]] = [[REM]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec6]]
  // NODBL: [[adr6:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 6
  // NODBL: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[adr6]]
  res[6] = things[5] % things[6];
#endif

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr7]]
  // CHECK: [[res7:%.*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec7]], <[[TYPE]] [[POS1:(1|1\.0*e\+0*|0xH3C00)]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res7]], <[[NUM]] x [[TYPE]]>* [[adr7]]
  // This is a post op, so the original value goes into res[].
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 7
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec7]], <[[NUM]] x [[TYPE]]>* [[adr7]]
  res[7] = things[7]++;

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr8]]
  // CHECK: [[res8:%.*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec8]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res8]], <[[NUM]] x [[TYPE]]>* [[adr8]]
  // This is a post op, so the original value goes into res[].
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 8
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec8]], <[[NUM]] x [[TYPE]]>* [[adr8]]
  res[8] = things[8]--;

  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // CHECK: [[vec9:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr9]]
  // CHECK: [[res9:%.*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec9]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[adr9]]
  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 9
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[adr9]]
  res[9] = ++things[9];

  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 10
  // CHECK: [[vec10:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr10]]
  // CHECK: [[res10:%.*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec10]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res10]], <[[NUM]] x [[TYPE]]>* [[adr10]]
  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %res, i32 0, i32 10
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res10]], <[[NUM]] x [[TYPE]]>* [[adr10]]
  res[10] = --things[10];

  // Memcpy res into return value.
  // CHECK: [[retptr:%.*]] = bitcast [11 x <[[NUM]] x [[TYPE]]>]* %agg.result to i8*
  // CHECK: [[resptr:%.*]] = bitcast [11 x <[[NUM]] x [[TYPE]]>]* %res to i8*
  // CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[retptr]], i8* [[resptr]]
  // CHECK: ret void
  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
// CHECK-LABEL: define void @"\01?logic
export vector<bool,NUM> logic(vector<bool,NUM> truth[10], vector<TYPE, NUM> consequences[10])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 0
  // CHECK: [[vec0:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr0]]
  // CHECK: [[bvec0:%.*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[bres0:%.*]] = icmp eq <[[NUM]] x i1> [[bvec0]], zeroinitializer
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 0
  // CHECK: [[res0:%.*]] = zext <[[NUM]] x i1> [[bres0]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[adr0]]
  res[0] = !truth[0];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr1]]
  // CHECK: [[bvec1:%.*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr2]]
  // CHECK: [[bvec2:%.*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[val1:%.*]] = icmp ne <[[NUM]] x i1> [[bvec1]], zeroinitializer
  // CHECK: [[val2:%.*]] = icmp ne <[[NUM]] x i1> [[bvec2]], zeroinitializer
  // CHECK: [[bres1:%.*]] = or <[[NUM]] x i1> [[val1]], [[val2]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 1
  // CHECK: [[res1:%.*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[adr1]]
  res[1] = truth[1] || truth[2];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr2]]
  // CHECK: [[bvec2:%.*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr3]]
  // CHECK: [[bvec3:%.*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[val2:%.*]] = icmp ne <[[NUM]] x i1> [[bvec2]], zeroinitializer
  // CHECK: [[val3:%.*]] = icmp ne <[[NUM]] x i1> [[bvec3]], zeroinitializer
  // CHECK: [[bres2:%.*]] = and <[[NUM]] x i1> [[val2]], [[val3]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 2
  // CHECK: [[res2:%.*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res2]], <[[NUM]] x i32>* [[adr2]]
  res[2] = truth[2] && truth[3];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr3]]
  // CHECK: [[bvec3:%.*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr4]]
  // CHECK: [[bvec4:%.*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 5
  // CHECK: [[vec5:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr5]]
  // CHECK: [[bvec5:%.*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[bres3:%.*]] = select <[[NUM]] x i1> [[bvec3]], <[[NUM]] x i1> [[bvec4]], <[[NUM]] x i1> [[bvec5]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 3
  // CHECK: [[res3:%.*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res3]], <[[NUM]] x i32>* [[adr3]]
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 0
  // CHECK: [[vec0:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr0]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // CHECK: [[bres4:%.*]] = [[CMP:[fi]?cmp( fast)?]] {{o?}}eq <[[NUM]] x [[TYPE]]> [[vec0]], [[vec1]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 4
  // CHECK: [[res4:%.*]] = zext <[[NUM]] x i1> [[bres4]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[adr4]]
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[bres5:%.*]] = [[CMP]] {{u?}}ne <[[NUM]] x [[TYPE]]> [[vec1]], [[vec2]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 5
  // CHECK: [[res5:%.*]] = zext <[[NUM]] x i1> [[bres5]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[adr5]]
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[bres6:%.*]] = [[CMP]] {{[osu]?}}lt <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 6
  // CHECK: [[res6:%.*]] = zext <[[NUM]] x i1> [[bres6]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res6]], <[[NUM]] x i32>* [[adr6]]
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[bres7:%.*]] = [[CMP]] {{[osu]]?}}gt <[[NUM]] x [[TYPE]]> [[vec3]], [[vec4]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 7
  // CHECK: [[res7:%.*]] = zext <[[NUM]] x i1> [[bres7]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res7]], <[[NUM]] x i32>* [[adr7]]
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 4
  // CHECK: [[vec4:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 5
  // CHECK: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // CHECK: [[bres8:%.*]] = [[CMP]] {{[osu]]?}}le <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 8
  // CHECK: [[res8:%.*]] = zext <[[NUM]] x i1> [[bres8]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res8]], <[[NUM]] x i32>* [[adr8]]
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 5
  // CHECK: [[vec5:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 6
  // CHECK: [[vec6:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr6]]
  // CHECK: [[bres9:%.*]] = [[CMP]] {{[osu]?}}ge <[[NUM]] x [[TYPE]]> [[vec5]], [[vec6]]
  // CHECK: [[adr9:%.*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %res, i32 0, i32 9
  // CHECK: [[res9:%.*]] = zext <[[NUM]] x i1> [[bres9]] to <[[NUM]] x i32>
  // CHECK: store <[[NUM]] x i32> [[res9]], <[[NUM]] x i32>* [[adr9]]
  res[9] = consequences[5] >= consequences[6];

  // Memcpy res into return value.
  // CHECK: [[retptr:%.*]] = bitcast [10 x <[[NUM]] x i32>]* %agg.result to i8*
  // CHECK: [[resptr:%.*]] = bitcast [10 x <[[NUM]] x i32>]* %res to i8*
  // CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[retptr]], i8* [[resptr]]
  // CHECK: ret void
  return res;
}

static const int Ix = 2;

// Test indexing operators
// CHECK-LABEL: define void @"\01?index
export vector<TYPE, NUM> index(vector<TYPE, NUM> things[10], int i)[10] {
  // CHECK: [[res:%.*]] = alloca [10 x <[[NUM]] x [[TYPE]]>]
  // CHECK: store i32 %i, i32* [[iadd:%.[0-9]*]]
  vector<TYPE, NUM> res[10];

  // CHECK: [[res0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> zeroinitializer, <[[NUM]] x [[TYPE]]>* [[res0]]
  res[0] = 0;

  // CHECK: [[i:%.*]] = load i32, i32* [[iadd]]
  // CHECK: [[adri:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 [[i]]
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] {{(1|1\.0*e\+0*|0xH3C00).*}}, <[[NUM]] x [[TYPE]]>* [[adri]]
  res[i] = 1;

  // CHECK: [[res2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] {{(2|2\.0*e\+0*|0xH4000).*}}, <[[NUM]] x [[TYPE]]>* [[res2]]
  res[Ix] = 2;

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: [[thg0:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr0]]
  // CHECK: [[res3:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thg0]], <[[NUM]] x [[TYPE]]>* [[res3]]
  res[3] = things[0];

  // CHECK: [[i:%.*]] = load i32, i32* [[iadd]]
  // CHECK: [[adri:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 [[i]]
  // CHECK: [[thgi:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adri]]
  // CHECK: [[res4:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thgi]], <[[NUM]] x [[TYPE]]>* [[res4]]
  res[4] = things[i];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[thg2:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // CHECK: [[res5:%.*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thg2]], <[[NUM]] x [[TYPE]]>* [[res5]]
  res[5] = things[Ix];
  // CHECK: ret void
  return res;
}

// Test bit twiddling operators.
// INT-LABEL: define void @"\01?bittwiddlers
export void bittwiddlers(inout vector<uint, NUM> things[11]) {
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 1
  // CHECK: [[ld1:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr1]]
  // CHECK: [[res1:%.*]] = xor <[[NUM]] x i32> [[ld1]], <i32 -1
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[adr0]]
  things[0] = ~things[1];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 2
  // CHECK: [[ld2:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 3
  // CHECK: [[ld3:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr3]]
  // CHECK: [[res1:%.*]] = or <[[NUM]] x i32> [[ld2]], [[ld3]]
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[adr1]]
  things[1] = things[2] | things[3];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 3
  // CHECK: [[ld3:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 4
  // CHECK: [[ld4:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr4]]
  // CHECK: [[res2:%.*]] = and <[[NUM]] x i32> [[ld3]], [[ld4]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 2
  // CHECK: store <[[NUM]] x i32> [[res2]], <[[NUM]] x i32>* [[adr2]]
  things[2] = things[3] & things[4];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 4
  // CHECK: [[ld4:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: [[ld5:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr5]]
  // CHECK: [[res3:%.*]] = xor <[[NUM]] x i32> [[ld4]], [[ld5]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 3
  // CHECK: store <[[NUM]] x i32> [[res3]], <[[NUM]] x i32>* [[adr3]]
  things[3] = things[4] ^ things[5];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: [[ld5:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr5]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 6
  // CHECK: [[ld6:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr6]]
  // CHECK: [[shv6:%.*]] = and <[[NUM]] x i32> [[ld6]], <i32 31
  // CHECK: [[res4:%.*]] = shl <[[NUM]] x i32> [[ld5]], [[shv6]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 4
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[adr4]]
  things[4] = things[5] << things[6];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 6
  // CHECK: [[ld6:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr6]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 7
  // CHECK: [[ld7:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr7]]
  // CHECK: [[shv7:%.*]] = and <[[NUM]] x i32> [[ld7]], <i32 31
  // CHECK: [[res5:%.*]] = lshr <[[NUM]] x i32> [[ld6]], [[shv7]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 5
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[adr5]]
  things[5] = things[6] >> things[7];

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 8
  // CHECK: [[ld8:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr8]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 6
  // CHECK: [[ld6:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr6]]
  // CHECK: [[res6:%.*]] = or <[[NUM]] x i32> [[ld6]], [[ld8]]
  // CHECK: store <[[NUM]] x i32> [[res6]], <[[NUM]] x i32>* [[adr6]]
  things[6] |= things[8];

  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 9
  // CHECK: [[ld9:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr9]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 7
  // CHECK: [[ld7:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr7]]
  // CHECK: [[res7:%.*]] = and <[[NUM]] x i32> [[ld7]], [[ld9]]
  // CHECK: store <[[NUM]] x i32> [[res7]], <[[NUM]] x i32>* [[adr7]]
  things[7] &= things[9];

  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 10
  // CHECK: [[ld10:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr10]]
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <[[NUM]] x i32>], [11 x <[[NUM]] x i32>]* %things, i32 0, i32 8
  // CHECK: [[ld8:%.*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[adr8]]
  // CHECK: [[res8:%.*]] = xor <[[NUM]] x i32> [[ld8]], [[ld10]]
  // CHECK: store <[[NUM]] x i32> [[res8]], <[[NUM]] x i32>* [[adr8]]
  things[8] ^= things[10];

  // CHECK: ret void
}
