// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// CHECK: @dx.op.bufferLoad.i32
// CHECK: extractvalue %dx.types.ResRet.i32 %BufferLoad, 2
// CHECK-NOT: dx.hl.subscript.[]

struct SMeshVertex
{
    float4 position                 : ATTR0;
};

Buffer< uint4 > Elements;

struct SVertexToPixel
{
    float projectedPosition : POSITION0;
};

SVertexToPixel main( in SMeshVertex inputRaw )
{
    SVertexToPixel output = (SVertexToPixel)0;

    output.projectedPosition.x = Elements[1][2];
    
    return output;
}