// RUN: %dxc /Tgs_6_0 /Emain -HV 2018 %s | FileCheck %s
// github issue #1560

// CHECK: main
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 2.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 4.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 5.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 6.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float 7.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float 8.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.emitStream(i32 97, i8 0)  ; EmitStream(streamId)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 9.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.100000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.200000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 1.300000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 1.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float 1.500000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float 1.600000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.emitStream(i32 97, i8 0)  ; EmitStream(streamId)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.700000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.800000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.900000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 2.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 2.100000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 2.200000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float 2.300000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float 2.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.emitStream(i32 97, i8 0)  ; EmitStream(streamId)
// CHECK: ret void

struct VSOutGSIn
{
    float4  clr : COLOR0;
    float4  pos : SV_Position;
};

struct GSOutPSIn
{
    float4  clr : COLOR0;
    float4  pos : SV_Position;
};

[maxvertexcount(3)]
void main(inout TriangleStream<GSOutPSIn> stream)
{
    VSOutGSIn tri[3];
    tri[0].clr = float4(1, 2, 3, 4);
    tri[0].pos = float4(5, 6, 7, 8);

    tri[1].clr = float4(9, 10, 11, 12);
    tri[1].pos = float4(13, 14, 15, 16);

    tri[2].clr = float4(17, 18, 19, 20);
    tri[2].pos = float4(21, 22, 23, 24);
    
    stream.Append(tri[0]);
    stream.Append(tri[1]);
    stream.Append(tri[2]);
}
