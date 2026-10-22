// RUN: %dxc -E main -T ps_6_0 -fcgl %s | FileCheck %s

struct ST {
   float4 a[32];
   int4 b[32];
};

ST st;
int ci;

float4 foo(float4 a[32], int i) {
  return a[i];
}

float4 bar(float4 a[32]) {
  return foo(a, ci);
}

float4 main() : SV_Target {
  return bar(st.a);
}

// bar should be called with a copy of st.a.
// CHECK: define <4 x float> @main()
// CHECK: [[a:%[0-9A-Z]+]] = getelementptr inbounds %"$Globals", %"$Globals"* {{%[0-9A-Z]+}}, i32 0, i32 0, i32 0
// CHECK: [[Tmpa:%[0-9A-Z]+]] = alloca [32 x <4 x float>]
// CHECK: [[TmpaPtr:%[0-9A-Z]+]] = bitcast [32 x <4 x float>]* [[Tmpa]] to i8*
// CHECK: [[aPtr:%[0-9A-Z]+]] = bitcast [32 x <4 x float>]* [[a]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[TmpaPtr]], i8* [[aPtr]], i64 512, i32 1, i1 false)
// CHECK: call <4 x float> @"\01?bar{{[@$?.A-Za-z0-9_]+}}"([32 x <4 x float>]* [[Tmpa]])

// Bug: Because a isn't marked noalias, we are generating copies for it.
// CHECK: define internal <4 x float> @"\01?bar{{[@$?.A-Za-z0-9_]+}}"([32 x <4 x float>]* [[a:%[0-9a]+]]) #1 {
// CHECK: [[Tmpa:%[0-9A-Z]+]] = alloca [32 x <4 x float>]
// CHECK: [[TmpaPtr:%[0-9A-Z]+]] = bitcast [32 x <4 x float>]* [[Tmpa]] to i8*
// CHECK: [[aPtr:%[0-9A-Z]+]] = bitcast [32 x <4 x float>]* [[a]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[TmpaPtr]], i8* [[aPtr]], i64 512, i32 1, i1 false)
// CHECK: call <4 x float> @"\01?foo{{[@$?.A-Za-z0-9_]+}}"([32 x <4 x float>]* [[Tmpa]], i32 {{%[0-9A-Z]+}})
