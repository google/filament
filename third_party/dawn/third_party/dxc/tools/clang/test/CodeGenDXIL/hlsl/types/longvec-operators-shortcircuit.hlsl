// RUN: %dxc -HV 2018 -T lib_6_9 %s | FileCheck %s
// RUN: %dxc -HV 2018 -T lib_6_9 %s | FileCheck %s --check-prefix=NOBR

// Test that no short-circuiting takes place for logic ops with native vectors.
// First run verifies that side effects result in stores.
// Second runline just makes sure there are no branches nor phis at all.

// NOBR-NOT: br i1
// NOBR-NOT: = phi

export int4 logic(inout bool4 truth[5], inout int4 consequences[4]) {
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [5 x <4 x i32>], [5 x <4 x i32>]* %truth, i32 0, i32 0
  // CHECK: [[vec0:%.*]] = load <4 x i32>, <4 x i32>* [[adr0]]
  // CHECK: [[bvec0:%.*]] = icmp ne <4 x i32> [[vec0]], zeroinitializer

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [4 x <4 x i32>], [4 x <4 x i32>]* %consequences, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <4 x i32>, <4 x i32>* [[adr1]]
  // CHECK: [[add:%.*]] = add <4 x i32> [[vec1]], <i32 1, i32 1, i32 1, i32 1>
  // CHECK: store <4 x i32> [[add]], <4 x i32>* [[adr1]]
  // CHECK: [[bvec1:%.*]] = icmp ne <4 x i32> [[vec1]], zeroinitializer
  // CHECK: [[bres3:%.*]] = or <4 x i1> [[bvec1]], [[bvec0]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [5 x <4 x i32>], [5 x <4 x i32>]* %truth, i32 0, i32 3
  // CHECK: [[res3:%.*]] = zext <4 x i1> [[bres3]] to <4 x i32>
  // CHECK: store <4 x i32> [[res3]], <4 x i32>* [[adr3]]
  truth[3] = truth[0] || consequences[1]++;

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [5 x <4 x i32>], [5 x <4 x i32>]* %truth, i32 0, i32 1
  // CHECK: [[vec1:%.*]] = load <4 x i32>, <4 x i32>* [[adr1]]
  // CHECK: [[bvec1:%.*]] = icmp ne <4 x i32> [[vec1]], zeroinitializer
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [4 x <4 x i32>], [4 x <4 x i32>]* %consequences, i32 0, i32 0
  // CHECK: [[vec0:%.*]] = load <4 x i32>, <4 x i32>* [[adr0]]
  // CHECK: [[sub:%.*]] = add <4 x i32> [[vec0]], <i32 -1, i32 -1, i32 -1, i32 -1>
  // CHECK: store <4 x i32> [[sub]], <4 x i32>* [[adr0]]
  // CHECK: [[bvec0:%.*]] = icmp ne <4 x i32> [[vec0]], zeroinitializer
  // CHECK: [[bres4:%.*]] = and <4 x i1> [[bvec0]], [[bvec1]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [5 x <4 x i32>], [5 x <4 x i32>]* %truth, i32 0, i32 4
  // CHECK: [[res4:%.*]] = zext <4 x i1> [[bres4]] to <4 x i32>
  // CHECK: store <4 x i32> [[res4]], <4 x i32>* [[adr4]]
  truth[4] = truth[1] && consequences[0]--;

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [5 x <4 x i32>], [5 x <4 x i32>]* %truth, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <4 x i32>, <4 x i32>* [[adr2]]
  // CHECK: [[bcond:%.*]] = icmp ne <4 x i32> [[vec2]], zeroinitializer
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [4 x <4 x i32>], [4 x <4 x i32>]* %consequences, i32 0, i32 2
  // CHECK: [[vec2:%.*]] = load <4 x i32>, <4 x i32>* [[adr2]]
  // CHECK: [[add:%.*]] = add <4 x i32> %25, <i32 1, i32 1, i32 1, i32 1>
  // CHECK: store <4 x i32> [[add]], <4 x i32>* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [4 x <4 x i32>], [4 x <4 x i32>]* %consequences, i32 0, i32 3
  // CHECK: [[vec3:%.*]] = load <4 x i32>, <4 x i32>* [[adr3]]
  // CHECK: [[sub:%.*]] = add <4 x i32> [[vec3]], <i32 -1, i32 -1, i32 -1, i32 -1>
  // CHECK: store <4 x i32> [[sub]], <4 x i32>* [[adr3]]
  // CHECK: [[res:%.*]] = select <4 x i1> [[bcond]], <4 x i32> [[vec2]], <4 x i32> [[vec3]]
  int4 res = truth[2] ? consequences[2]++ : consequences[3]--;

  // CHECK: ret <4 x i32> %30
  return res;
}
