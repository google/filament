// RUN: %dxc -E main -T cs_6_6 -Od %s | FileCheck -check-prefixes=CHECK,SM66 %s
// RUN: %dxc -E main -T cs_6_7 -Od %s | FileCheck -check-prefixes=CHECK,SM67 %s

// In Shader Model 6.6 and earlier the QuadAny and QuadAll intrinsics will
// expand out to quadOp instructions reading from each of the 3 other lanes and
// bitwise & or | instructions to compute the result.

// SM66:      [[cond:%[a-z0-9]+]] = sext i1 %{{[a-z0-9]+}} to i32
// SM66-NEXT: [[x:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 0)
// SM66-NEXT: [[y:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 1)
// SM66-NEXT: [[z:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 2)
// SM66-NEXT: [[xy:%[a-z0-9]+]] = or i32 [[x]], [[y]]
// SM66-NEXT: [[xyz:%[a-z0-9]+]] = or i32 [[xy]], [[z]]
// SM66-NEXT: [[wide:%[a-z0-9]+]] = or i32 [[xyz]], [[cond]]
// SM66-NEXT: [[any:%[a-z0-9]+]] = trunc i32 [[wide]] to i1 

// SM67:      [[cond:%[a-z0-9]+]] = icmp ne i1 %{{[a-z0-9]+}}, false
// SM67-NEXT: [[any:%[a-zA-Z0-9]+]] = call i1 @dx.op.quadVote.i1(i32 222, i1 [[cond]], i8 0)

// CHECK:     select i1 [[any]], float 1.000000e+00, float 2.000000e+00


// SM66:      [[cond:%[a-z0-9]+]] = sext i1 {{%[a-z0-9]+}} to i32
// SM66-NEXT: [[x:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 0)
// SM66-NEXT: [[y:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 1)
// SM66-NEXT: [[z:%[a-zA-Z0-9]+]] = call i32 @dx.op.quadOp.i32(i32 123, i32 [[cond]], i8 2)
// SM66-NEXT: [[xy:%[a-z0-9]+]] = and i32 [[x]], [[y]]
// SM66-NEXT: [[xyz:%[a-z0-9]+]] = and i32 [[xy]], [[z]]
// SM66-NEXT: [[wide:%[a-z0-9]+]] = and i32 [[xyz]], [[cond]]
// SM66-NEXT: [[all:%[a-z0-9]+]] = trunc i32 [[wide]] to i1

// SM67:      [[cond:%[a-z0-9]+]] = icmp ne i1 %{{[a-z0-9]+}}, false
// SM67-NEXT: [[all:%[a-zA-Z0-9]+]] = call i1 @dx.op.quadVote.i1(i32 222, i1 [[cond]], i8 1)

// CHECK:     select i1 [[all]], float 3.000000e+00, float 4.000000e+00

RWStructuredBuffer<float4> Values;

[numthreads(8, 8, 1)]
void main(uint3 ID: SV_DispatchThreadID) {
  uint OutIdx = (ID.y * 8) + ID.x;
  Values[OutIdx].x = QuadAny(OutIdx % 4 == 0) ? 1.0 : 2.0;
  Values[OutIdx].y = QuadAll(OutIdx % 2 == 0) ? 3.0 : 4.0;
  Values[OutIdx].z = 0.0;
  Values[OutIdx].w = 0.0;
}
