// RUN: %dxc -DObjType=RasterizerOrderedTexture1D -DEltType=uint -DPosType=uint1 -T ps_6_0 %s | FileCheck %s -check-prefixes=TXTCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedTexture2D -DEltType=uint -DPosType=uint2 -T ps_6_0 %s | FileCheck %s -check-prefixes=TXTCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedTexture3D -DEltType=uint -DPosType=uint3 -T ps_6_0 %s | FileCheck %s -check-prefixes=TXTCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedTexture1DArray -DEltType=uint -DPosType=uint2 -T ps_6_0 %s | FileCheck %s -check-prefixes=TXTCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedTexture2DArray -DEltType=uint -DPosType=uint3 -T ps_6_0 %s | FileCheck %s -check-prefixes=TXTCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedBuffer -DEltType=uint -DPosType=uint1 -T ps_6_0 %s | FileCheck %s -check-prefixes=BUFCHK,CHECK
// RUN: %dxc -DObjType=RasterizerOrderedStructuredBuffer -DEltType=uint -DPosType=uint1 -T ps_6_0 %s | FileCheck %s -check-prefixes=BUFCHK,CHECK

// BABs can't be indexed so they require special casing
// RUN: %dxc -DBAB -DResType=RasterizerOrderedByteAddressBuffer -DPosType=uint -T ps_6_0 %s | FileCheck %s -check-prefixes=BUFCHK,CHECK

// Verify that the ROV texture type is correctly identified
// such that a struct member of that type can be assigned

#ifndef BAB
typedef ObjType<EltType> ResType;
#endif

ResType GlobalResource[6] : register(u0);

// Use a few different storage and declaration ways

struct ROVThings {
  ResType Resource;
};

const static struct {
  ResType Resource;
} thing1 = {GlobalResource[0]};

const static ROVThings thing2 = {GlobalResource[1]};

void main(in PosType pos : P, out float4 Out : SV_Target0)
{
        // CHECK: %[[t3:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 2
        const struct {
          ResType Resource;
        } thing3 = {GlobalResource[2]};
        // CHECK: %[[t4:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 3
        struct {
          ResType Resource;
        } thing4 = {GlobalResource[3]};

        // CHECK: %[[t5:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 4
        ROVThings thing5 = {GlobalResource[4]};
        // CHECK: %[[t6:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 5
        const ROVThings thing6 = {GlobalResource[5]};


	Out = 0.0;

#ifndef BAB
        // CHECK: %[[t1:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t1]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t1]]
        // CHECK: add i32
        // TXTCHK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle %[[t1]]
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t1]]
	thing1.Resource[pos] += 1;
        // CHECK: %[[t2:[0-9a-zA-Z_]*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 1
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t2]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t2]]
        // CHECK: add i32
        // TXTCHK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle %[[t2]]
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t2]]
	thing2.Resource[pos] += 1;
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t3]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t3]]
        // CHECK: add i32
        // TXTCHK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle %[[t3]]
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t3]]
	thing3.Resource[pos] += 1;
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t4]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t4]]
        // CHECK: add i32
        // TXTCHK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle %[[t4]]
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t4]]
	thing4.Resource[pos] += 1;
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t5]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t5]]
        // CHECK: add i32
        // TXTCHK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle %[[t5]]
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t5]]
	thing5.Resource[pos] += 1;
        // TXTCHK: call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %[[t6]]
        // BUFCHK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %[[t6]]
        // CHECK: add i32
        // BUFCHK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[t6]]
	thing6.Resource[pos] += 1;
#else
        // ByteAddressBuffers are too weird to play with the others
        // Note that all the CHECKS this needs are included
        // in the corresponding locations of the BUFCHK variant above
	thing1.Resource.Store(pos, thing1.Resource.Load(pos) + 1);
	thing2.Resource.Store(pos, thing2.Resource.Load(pos) + 1);
	thing3.Resource.Store(pos, thing3.Resource.Load(pos) + 1);
	thing4.Resource.Store(pos, thing4.Resource.Load(pos) + 1);
	thing5.Resource.Store(pos, thing5.Resource.Load(pos) + 1);
	thing6.Resource.Store(pos, thing6.Resource.Load(pos) + 1);
#endif
}
