// RUN: %dxc -T lib_6_9   -DTYPE=uint     -DNUM=5 %s | FileCheck %s --check-prefixes=CHECK,UNSIG
// RUN: %dxc -T lib_6_9   -DTYPE=int64_t  -DNUM=3 %s | FileCheck %s --check-prefixes=CHECK,SIG
// RUN: %dxc -T lib_6_9   -DTYPE=uint16_t -DNUM=9 -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,UNSIG

// Test bitwise operators on an assortment vector sizes and integer types with 6.9 native vectors.

// Test bit twiddling operators.
// CHECK-LABEL: define void @"\01?bittwiddlers
// CHECK-SAME: ([11 x <[[NUM:[0-9][0-9]*]] x [[TYPE:[a-z0-9]*]]>]*
export void bittwiddlers(inout vector<TYPE, NUM> things[11]) {
  // CHECK: [[adr1:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // CHECK: [[res1:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec1]], <[[TYPE]] -1,
  // CHECK: [[adr0:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[adr0]]
  things[0] = ~things[1];

  // CHECK: [[adr2:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]

  // CHECK: [[adr3:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // CHECK: [[res1:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[vec3]], [[vec2]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[adr1]]
  things[1] = things[2] | things[3];

  // CHECK: [[adr4:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // CHECK: [[res2:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec4]], [[vec3]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[adr2]]
  things[2] = things[3] & things[4];

  // CHECK: [[adr5:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // CHECK: [[res3:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[adr3]]
  things[3] = things[4] ^ things[5];

  // CHECK: [[adr6:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr6]]
  // CHECK: [[shv6:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec6]], <[[TYPE]]
  // CHECK: [[res4:%[0-9]*]] = shl <[[NUM]] x [[TYPE]]> [[vec5]], [[shv6]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[adr4]]
  things[4] = things[5] << things[6];

  // CHECK: [[adr7:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr7]]
  // CHECK: [[shv7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec7]], <[[TYPE]]
  // UNSIG: [[res5:%[0-9]*]] = lshr <[[NUM]] x [[TYPE]]> [[vec6]], [[shv7]]
  // SIG: [[res5:%[0-9]*]] = ashr <[[NUM]] x [[TYPE]]> [[vec6]], [[shv7]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[adr5]]
  things[5] = things[6] >> things[7];

  // CHECK: [[adr8:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr8]]
  // CHECK: [[res6:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[vec8]], [[vec6]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[adr6]]
  things[6] |= things[8];

  // CHECK: [[adr9:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // CHECK: [[vec9:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr9]]
  // CHECK: [[res7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec9]], [[vec7]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res7]], <[[NUM]] x [[TYPE]]>* [[adr7]]
  things[7] &= things[9];

  // CHECK: [[adr10:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 10
  // CHECK: [[vec10:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr10]]
  // CHECK: [[res8:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec8]], [[vec10]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res8]], <[[NUM]] x [[TYPE]]>* [[adr8]]
  things[8] ^= things[10];

  // CHECK: ret void
}
