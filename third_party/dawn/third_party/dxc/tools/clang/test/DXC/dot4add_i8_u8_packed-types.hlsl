// RUN: %dxc /enable-16bit-types /T cs_6_8 %s | FileCheck %s

// Compiling this HLSL would fail this assertion in TranslateDot4AddPacked:
//
//     DXASSERT(
//         !accTy->isVectorTy() && accTy->isIntegerTy(32),
//         "otherwise, unexpected vector support in high level intrinsic template");
//
// Bug was fixed by changing the declarations of dot4add_i8packed and
// dot4add_u8packed in utils/hct/gen_intrin_main.txt to simply write
// out their argument and return types, rather than using the $typeN
// reference syntax.

// CHECK: call i32 @dx.op.dot4AddPacked.i32{{.*}}Dot4AddI8Packed(acc,a,b)
// CHECK: call i32 @dx.op.dot4AddPacked.i32{{.*}}Dot4AddU8Packed(acc,a,b)
// CHECK: call float @dx.op.dot2AddHalf.f32{{.*}}Dot2AddHalf(acc,ax,ay,bx,by)

RWByteAddressBuffer buf;

[numthreads(1, 1, 1)]
void main()
{
    int a = dot4add_i8packed(0, 0, 0);
    int b = dot4add_i8packed(0, 0, a);
    buf.Store<int>(0, b);

    uint c = dot4add_u8packed(0, 0, 0);
    uint d = dot4add_u8packed(0, 0, c);
    buf.Store<uint>(4, d);

    float e = dot2add(half2(0,0), half2(0,0), 1.0);
    float f = dot2add(half2(0,0), half2(0,0), e);
    buf.Store<float>(8, f);
}
