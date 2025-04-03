// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=float     -DNUM=5 %s | FileCheck %s -check-prefixes=CHECK,F5
// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=bool      -DNUM=7 %s | FileCheck %s -check-prefixes=CHECK,B7
// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=uint64_t  -DNUM=9 %s | FileCheck %s -check-prefixes=CHECK,L9
// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=double    -DNUM=17 %s | FileCheck %s -check-prefixes=CHECK,D17
// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=float16_t -DNUM=256 -enable-16bit-types %s | FileCheck %s -check-prefixes=CHECK,H256
// RUN: %dxc -fcgl -T lib_6_9 -DTYPE=int16_t   -DNUM=1024 -enable-16bit-types %s | FileCheck %s -check-prefixes=CHECK,S1024

// A test to verify that declarations of longvecs are permitted in all the accepted places.
// Only tests for acceptance, most codegen is ignored for now.

// CHECK: %struct.LongVec = type { <4 x float>, <[[NUM:[0-9]*]] x [[STY:[a-z0-9]*]]> }
struct LongVec {
  float4 f;
  vector<TYPE,NUM> vec;
};

struct LongVecSub : LongVec {
  int3 is;
};

template<int N>
struct LongVecTpl {
  float4 f;
  vector<float,N> vec;
};

// Just some dummies to capture the types and mangles.
// CHECK: @"\01?dummy@@3[[MNG:F|M|N|_N|_K|\$f16@]]A" = external addrspace(3) global [[STY]]
groupshared TYPE dummy;

// Use the first groupshared to establish mangles and sizes
// F5-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:M]]$[[VS:04]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
// B7-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:_N]]$[[VS:06]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
// L9-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:_K]]$[[VS:08]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
// D17-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:N]]$[[VS:0BB@]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
// H256-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:\$f16@]]$[[VS:0BAA@]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
// S1024-DAG: @"\01?gs_vec@@3V?$vector@[[MNG:F]]$[[VS:0EAA@]]@@A" = external addrspace(3) global <[[NUM]] x [[STY]]>
groupshared vector<TYPE, NUM> gs_vec;

// CHECK-DAG: @"\01?gs_vec_arr@@3PAV?$vector@[[MNG]]$[[VS]]@@A" = external addrspace(3) global [10 x <[[NUM]] x [[STY]]>]
groupshared vector<TYPE, NUM> gs_vec_arr[10];
// CHECK-DAG: @"\01?gs_vec_rec@@3ULongVec@@A" = external addrspace(3) global %struct.LongVec
groupshared LongVec gs_vec_rec;
// CHECK-DAG: @"\01?gs_vec_sub@@3ULongVecSub@@A" = external addrspace(3) global %struct.LongVecSub
groupshared LongVecSub gs_vec_sub;
// CHECK-DAG: @"\01?gs_vec_tpl@@3U?$LongVecTpl@$[[VS]]@@A" = external addrspace(3) global %"struct.LongVecTpl<[[NUM]]>"
groupshared LongVecTpl<NUM> gs_vec_tpl;

// CHECK-DAG: @static_vec = internal global <[[NUM]] x [[STY]]>
static vector<TYPE, NUM> static_vec;
// CHECK-DAG: @static_vec_arr = internal global [10 x <[[NUM]] x [[STY]]>] zeroinitializer
static vector<TYPE, NUM> static_vec_arr[10];
// CHECK-DAG: @static_vec_rec = internal global %struct.LongVec
static LongVec static_vec_rec;
// CHECK-DAG: @static_vec_sub = internal global %struct.LongVecSub
static LongVecSub static_vec_sub;
// CHECK-DAG: @static_vec_tpl = internal global %"struct.LongVecTpl<[[NUM]]>"
static LongVecTpl<NUM> static_vec_tpl;

// CHECK: define [[RTY:[a-z0-9]*]] @"\01?getVal@@YA[[MNG]][[MNG]]@Z"([[RTY]] {{.*}}%t)
export TYPE getVal(TYPE t) {TYPE ret = dummy; dummy = t; return ret;}

// CHECK: define <[[NUM]] x [[RTY]]>
// CHECK-LABEL: @"\01?lv_param_passthru
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$[[VS]]@@V1@@Z"(<[[NUM]] x [[RTY]]> %vec1)
// CHECK:   ret <[[NUM]] x [[RTY]]>
export vector<TYPE, NUM> lv_param_passthru(vector<TYPE, NUM> vec1) {
  return vec1;
}

// CHECK-LABEL: define void @"\01?lv_param_arr_passthru
// CHECK-SAME: @@YA$$BY09V?$vector@[[MNG]]$[[VS]]@@Y09V1@@Z"([10 x <[[NUM]] x [[STY]]>]* noalias sret %agg.result, [10 x <[[NUM]] x [[STY]]>]* %vec)
// CHECK: ret void
export vector<TYPE, NUM> lv_param_arr_passthru(vector<TYPE, NUM> vec[10])[10] {
  return vec;
}

// CHECK-LABEL: define void @"\01?lv_param_rec_passthru@@YA?AULongVec@@U1@@Z"(%struct.LongVec* noalias sret %agg.result, %struct.LongVec* %vec)
// CHECK: memcpy
// CHECK:   ret void
export LongVec lv_param_rec_passthru(LongVec vec) {
  return vec;
}

// CHECK-LABEL: define void @"\01?lv_param_sub_passthru@@YA?AULongVec@@U1@@Z"(%struct.LongVec* noalias sret %agg.result, %struct.LongVec* %vec)
// CHECK: memcpy
// CHECK:   ret void
export LongVec lv_param_sub_passthru(LongVec vec) {
  return vec;
}

// CHECK-LABEL: define void @"\01?lv_param_tpl_passthru@@YA?AULongVec@@U1@@Z"(%struct.LongVec* noalias sret %agg.result, %struct.LongVec* %vec)
// CHECK: memcpy
// CHECK:   ret void
export LongVec lv_param_tpl_passthru(LongVec vec) {
  return vec;
}

// CHECK-LABEL: define void @"\01?lv_param_in_out
// CHECK-SAME: @@YAXV?$vector@[[MNG]]$[[VS]]@@AIAV1@@Z"(<[[NUM]] x [[RTY]]> %vec1, <[[NUM]] x [[STY]]>* noalias dereferenceable({{[0-9]*}}) %vec2)
// CHECK:   store <[[NUM]] x [[STY]]> {{%.*}}, <[[NUM]] x [[STY]]>* %vec2, align 4
// CHECK:   ret void
export void lv_param_in_out(in vector<TYPE, NUM> vec1, out vector<TYPE, NUM> vec2) {
  vec2 = vec1;
}

// CHECK-LABEL: define void @"\01?lv_param_in_out_rec@@YAXULongVec@@U1@@Z"(%struct.LongVec* %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_in_out_rec(in LongVec vec1, out LongVec vec2) {
  vec2 = vec1;
}

// CHECK-LABEL: define void @"\01?lv_param_in_out_sub@@YAXULongVec@@U1@@Z"(%struct.LongVec* %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_in_out_sub(in LongVec vec1, out LongVec vec2) {
  vec2 = vec1;
}

// CHECK-LABEL: define void @"\01?lv_param_in_out_tpl@@YAXULongVec@@U1@@Z"(%struct.LongVec* %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_in_out_tpl(in LongVec vec1, out LongVec vec2) {
  vec2 = vec1;
}


// CHECK-LABEL: define void @"\01?lv_param_inout
// CHECK-SAME: @@YAXAIAV?$vector@[[MNG]]$[[VS]]@@0@Z"(<[[NUM]] x [[STY]]>* noalias dereferenceable({{[0-9]*}}) %vec1, <[[NUM]] x [[STY]]>* noalias dereferenceable({{[0-9]*}}) %vec2)
// CHECK:   load <[[NUM]] x [[STY]]>, <[[NUM]] x [[STY]]>* %vec1, align 4
// CHECK:   load <[[NUM]] x [[STY]]>, <[[NUM]] x [[STY]]>* %vec2, align 4
// CHECK:   store <[[NUM]] x [[STY]]> {{%.*}}, <[[NUM]] x [[STY]]>* %vec1, align 4
// CHECK:   store <[[NUM]] x [[STY]]> {{%.*}}, <[[NUM]] x [[STY]]>* %vec2, align 4
// CHECK:   ret void
export void lv_param_inout(inout vector<TYPE, NUM> vec1, inout vector<TYPE, NUM> vec2) {
  vector<TYPE, NUM> tmp = vec1;
  vec1 = vec2;
  vec2 = tmp;
}

// CHECK-LABEL: define void @"\01?lv_param_inout_rec@@YAXULongVec@@0@Z"(%struct.LongVec* noalias %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_inout_rec(inout LongVec vec1, inout LongVec vec2) {
  LongVec tmp = vec1;
  vec1 = vec2;
  vec2 = tmp;
}

// CHECK-LABEL: define void @"\01?lv_param_inout_sub@@YAXULongVec@@0@Z"(%struct.LongVec* noalias %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_inout_sub(inout LongVec vec1, inout LongVec vec2) {
  LongVec tmp = vec1;
  vec1 = vec2;
  vec2 = tmp;
}

// CHECK-LABEL: define void @"\01?lv_param_inout_tpl@@YAXULongVec@@0@Z"(%struct.LongVec* noalias %vec1, %struct.LongVec* noalias %vec2)
// CHECK: memcpy
// CHECK:   ret void
export void lv_param_inout_tpl(inout LongVec vec1, inout LongVec vec2) {
  LongVec tmp = vec1;
  vec1 = vec2;
  vec2 = tmp;
}

// CHECK-LABEL: define void @"\01?lv_global_assign
// CHECK-SAME: @@YAXV?$vector@[[MNG]]$[[VS]]@@Y09V1@ULongVec@@ULongVecSub@@U?$LongVecTpl@$[[VS]]@@@Z"(<[[NUM]] x [[RTY]]> %vec, [10 x <[[NUM]] x [[STY]]>]* %arr, %struct.LongVec* %rec, %struct.LongVecSub* %sub, %"struct.LongVecTpl<[[NUM]]>"* %tpl)
// CHECK:   store <[[NUM]] x [[STY]]> {{%.*}}, <[[NUM]] x [[STY]]>* @static_vec
// CHECK:   ret void
export void lv_global_assign(vector<TYPE, NUM> vec, vector<TYPE, NUM> arr[10],
                             LongVec rec, LongVecSub sub, LongVecTpl<NUM> tpl) {
  static_vec = vec;
  static_vec_arr = arr;
  static_vec_rec = rec;
  static_vec_sub = sub;
  static_vec_tpl = tpl;
}

// CHECK-LABEL: define void @"\01?lv_gs_assign
// CHECK-SAME: @@YAXV?$vector@[[MNG]]$[[VS]]@@Y09V1@ULongVec@@ULongVecSub@@U?$LongVecTpl@$[[VS]]@@@Z"(<[[NUM]] x [[RTY]]> %vec, [10 x <[[NUM]] x [[STY]]>]* %arr, %struct.LongVec* %rec, %struct.LongVecSub* %sub, %"struct.LongVecTpl<[[NUM]]>"* %tpl)
// CHECK:   store <[[NUM]] x [[STY]]> {{%.*}}, <[[NUM]] x [[STY]]> addrspace(3)* @"\01?gs_vec@@3V?$vector@[[MNG]]$[[VS]]@@A"
// CHECK:   ret void
export void lv_gs_assign(vector<TYPE, NUM> vec, vector<TYPE, NUM> arr[10],
                         LongVec rec, LongVecSub sub, LongVecTpl<NUM> tpl) {
  gs_vec = vec;
  gs_vec_arr = arr;
  gs_vec_rec = sub;
  gs_vec_tpl = tpl;
}

// CHECK: define <[[NUM]] x [[RTY]]>
// CHECK-LABEL: @"\01?lv_global_ret
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$[[VS]]@@XZ"()
// CHECK:   load <[[NUM]] x [[STY]]>, <[[NUM]] x [[STY]]>* @static_vec
// CHECK:   ret <[[NUM]] x [[RTY]]>
export vector<TYPE, NUM> lv_global_ret() {
  return static_vec;
}

// CHECK-LABEL: define void @"\01?lv_global_arr_ret
// CHECK-SAME: @@YA$$BY09V?$vector@[[MNG]]$[[VS]]@@XZ"([10 x <[[NUM]] x [[STY]]>]* noalias sret %agg.result)
// CHECK: ret void
export vector<TYPE, NUM> lv_global_arr_ret()[10] {
  return static_vec_arr;
}

// CHECK-LABEL: define void @"\01?lv_global_rec_ret@@YA?AULongVec@@XZ"(%struct.LongVec* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVec lv_global_rec_ret() {
  return static_vec_rec;
}

// CHECK-LABEL: define void @"\01?lv_global_sub_ret@@YA?AULongVecSub@@XZ"(%struct.LongVecSub* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVecSub lv_global_sub_ret() {
  return static_vec_sub;
}

// CHECK-LABEL: define void @"\01?lv_global_tpl_ret
// CHECK-SAME: @@YA?AU?$LongVecTpl@$[[VS]]@@XZ"(%"struct.LongVecTpl<[[NUM]]>"* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVecTpl<NUM> lv_global_tpl_ret() {
  return static_vec_tpl;
}

// CHECK: define <[[NUM]] x [[RTY]]>
// CHECK-LABEL: @"\01?lv_gs_ret
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$[[VS]]@@XZ"()
// CHECK:   load <[[NUM]] x [[STY]]>, <[[NUM]] x [[STY]]> addrspace(3)* @"\01?gs_vec@@3V?$vector@[[MNG]]$[[VS]]@@A"
// CHECK:   ret <[[NUM]] x [[RTY]]>
export vector<TYPE, NUM> lv_gs_ret() {
  return gs_vec;
}

// CHECK-LABEL: define void @"\01?lv_gs_arr_ret
// CHECK-SAME: @@YA$$BY09V?$vector@[[MNG]]$[[VS]]@@XZ"([10 x <[[NUM]] x [[STY]]>]* noalias sret %agg.result)
// CHECK: ret void
export vector<TYPE, NUM> lv_gs_arr_ret()[10] {
  return gs_vec_arr;
}

// CHECK-LABEL: define void @"\01?lv_gs_rec_ret@@YA?AULongVec@@XZ"(%struct.LongVec* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVec lv_gs_rec_ret() {
  return gs_vec_rec;
}

// CHECK-LABEL: define void @"\01?lv_gs_sub_ret@@YA?AULongVecSub@@XZ"(%struct.LongVecSub* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVecSub lv_gs_sub_ret() {
  return gs_vec_sub;
}

// CHECK-LABEL: define void @"\01?lv_gs_tpl_ret
// CHECK-SAME: @@YA?AU?$LongVecTpl@$[[VS]]@@XZ"(%"struct.LongVecTpl<[[NUM]]>"* noalias sret %agg.result)
// CHECK: memcpy
// CHECK:   ret void
export LongVecTpl<NUM> lv_gs_tpl_ret() {
  return gs_vec_tpl;
}

// CHECK: define <[[NUM]] x [[RTY]]>
// CHECK-LABEL: @"\01?lv_splat
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$[[VS]]@@[[MNG]]@Z"([[RTY]] {{.*}}%scalar)
// CHECK:   ret <[[NUM]] x [[RTY]]>
export vector<TYPE,NUM> lv_splat(TYPE scalar) {
  vector<TYPE,NUM> ret = scalar;
  return ret;
}

// CHECK: define <6 x [[RTY]]>
// CHECK-LABEL: @"\01?lv_initlist
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$05@@XZ"()
// CHECK:   ret <6 x [[RTY]]>
export vector<TYPE, 6> lv_initlist() {
  vector<TYPE, 6> ret = {1, 2, 3, 4, 5, 6};
  return ret;
}

// CHECK: define <6 x [[RTY]]>
// CHECK-LABEL: @"\01?lv_initlist_vec
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$05@@V?$vector@[[MNG]]$02@@@Z"(<3 x [[RTY]]> %vec)
// CHECK:   ret <6 x [[RTY]]>
export vector<TYPE, 6> lv_initlist_vec(vector<TYPE, 3> vec) {
  vector<TYPE, 6> ret = {vec, 4.0, 5.0, 6.0};
  return ret;
}

// CHECK: define <6 x [[RTY]]>
// CHECK-LABEL: @"\01?lv_vec_vec
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$05@@V?$vector@[[MNG]]$02@@0@Z"(<3 x [[RTY]]> %vec1, <3 x [[RTY]]> %vec2)
// CHECK:   ret <6 x [[RTY]]>
export vector<TYPE, 6> lv_vec_vec(vector<TYPE, 3> vec1, vector<TYPE, 3> vec2) {
  vector<TYPE, 6> ret = {vec1, vec2};
  return ret;
}

// CHECK: define <[[NUM]] x [[RTY]]>
// CHECK-LABEL: @"\01?lv_array_cast
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$[[VS]]@@Y[[VS]][[MNG]]@Z"({{\[}}[[NUM]] x [[STY]]]* %arr)
// CHECK:   ret <[[NUM]] x [[RTY]]>
export vector<TYPE, NUM> lv_array_cast(TYPE arr[NUM]) {
  vector<TYPE, NUM> ret = (vector<TYPE,NUM>)arr;
  return ret;
}

// CHECK: define <6 x [[RTY]]>
// CHECK-LABEL: @"\01?lv_ctor
// CHECK-SAME: @@YA?AV?$vector@[[MNG]]$05@@[[MNG]]@Z"([[RTY]] {{.*}}%s)
// CHECK:  ret <6 x [[RTY]]>
export vector<TYPE, 6> lv_ctor(TYPE s) {
  vector<TYPE, 6> ret = vector<TYPE,6>(1.0, 2.0, 3.0, 4.0, 5.0, s);
  return ret;
}
