// RUN: %dxc -Zi -Qembed_debug -T lib_6_9 %s -DNUM=8 | FileCheck %s  --check-prefix=CHECK-LONG
// RUN: %dxc -Zi -Qembed_debug -T lib_6_9 %s -DNUM=4 | FileCheck %s  --check-prefix=CHECK-SHORT

// Test debug info for short and long vector types

RWByteAddressBuffer buf;

export vector<float, NUM> lv_global_arr_ret() {
  vector<float, NUM> d = buf.Load<vector<float, NUM> >(0);
  return d;
}

// CHECK-LONG:  ![[TYDI:[^ ]+]] = !DICompositeType(tag: DW_TAG_class_type, name: "vector<float, 8>", file: !{{[^ ]+}}, size: 256, align: 32, elements: ![[ELEMDI:[^ ]+]],
// CHECK-LONG:  ![[ELEMDI]] = !{![[C0:[^ ]+]], ![[C1:[^ ]+]], ![[C2:[^ ]+]], ![[C3:[^ ]+]], ![[C4:[^ ]+]], ![[C5:[^ ]+]], ![[C6:[^ ]+]], ![[C7:[^ ]+]]}
// CHECK-LONG:  ![[C0]] = !DIDerivedType(tag: DW_TAG_member, name: "c0", scope: !{{[^ ]+}} file: !{{[^ ]+}}, baseType: ![[BASETY:[^ ]+]], size: 32, align: 32, flags: DIFlagPublic)
// CHECK-LONG:  ![[BASETY]] = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
// CHECK-LONG:  ![[C1]] = !DIDerivedType(tag: DW_TAG_member, name: "c1", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 32, flags: DIFlagPublic)
// CHECK-LONG:  ![[C2]] = !DIDerivedType(tag: DW_TAG_member, name: "c2", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 64, flags: DIFlagPublic)
// CHECK-LONG:  ![[C3]] = !DIDerivedType(tag: DW_TAG_member, name: "c3", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 96, flags: DIFlagPublic)
// CHECK-LONG:  ![[C4]] = !DIDerivedType(tag: DW_TAG_member, name: "c4", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 128, flags: DIFlagPublic)
// CHECK-LONG:  ![[C5]] = !DIDerivedType(tag: DW_TAG_member, name: "c5", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 160, flags: DIFlagPublic)
// CHECK-LONG:  ![[C6]] = !DIDerivedType(tag: DW_TAG_member, name: "c6", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 192, flags: DIFlagPublic)
// CHECK-LONG:  ![[C7]] = !DIDerivedType(tag: DW_TAG_member, name: "c7", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 224, flags: DIFlagPublic)
// CHECK-LONG:  !{{[^ ]+}} = !DILocalVariable(tag: DW_TAG_auto_variable, name: "d", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, line: 9, type: ![[TYDI]])

// CHECK-SHORT:  ![[TYDI:[^ ]+]] = !DICompositeType(tag: DW_TAG_class_type, name: "vector<float, 4>", file: !{{[^ ]+}}, size: 128, align: 32, elements: ![[ELEMDI:[^ ]+]],
// CHECK-SHORT:  ![[ELEMDI]] = !{![[X:[^ ]+]], ![[Y:[^ ]+]], ![[Z:[^ ]+]], ![[W:[^ ]+]]}
// CHECK-SHORT:  ![[X]] = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY:[^ ]+]], size: 32, align: 32, flags: DIFlagPublic)
// CHECK-SHORT:  ![[BASETY]] = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
// CHECK-SHORT:  ![[Y]] = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 32, flags: DIFlagPublic)
// CHECK-SHORT:  ![[Z]] = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 64, flags: DIFlagPublic)
// CHECK-SHORT:  ![[W]] = !DIDerivedType(tag: DW_TAG_member, name: "w", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, baseType: ![[BASETY]], size: 32, align: 32, offset: 96, flags: DIFlagPublic)
// CHECK-SHORT:  !{{[^ ]+}} = !DILocalVariable(tag: DW_TAG_auto_variable, name: "d", scope: !{{[^ ]+}}, file: !{{[^ ]+}}, line: 9, type: ![[TYDI]])