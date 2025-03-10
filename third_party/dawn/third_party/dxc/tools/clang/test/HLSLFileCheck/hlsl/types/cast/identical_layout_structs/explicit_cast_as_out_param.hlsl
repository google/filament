// RUN: %dxc /Tvs_6_0 /Emain -HV 2018 %s | FileCheck %s
// Test explicit cast between structs of identical layout where
// the destination struct is marked as out param.

// o1.f1 = input.f1
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 2.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 4.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.f3[4] = input.f3[4]
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 0, i8 0, float 1.300000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 0, i8 1, float 1.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 1, i8 0, float 1.500000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 1, i8 1, float 1.600000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 2, i8 0, float 1.700000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 2, i8 1, float 1.800000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 3, i8 0, float 1.900000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 2, i32 3, i8 1, float 2.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.f4 = input.f4
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 3, i32 0, i8 0, i32 21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 3, i32 0, i8 1, i32 22)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 3, i32 0, i8 2, i32 23)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.s.f5 = input.s.f5
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 4, i32 0, i8 0, float 2.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 4, i32 0, i8 1, float 2.500000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 4, i32 0, i8 2, float 2.600000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 4, i32 0, i8 3, float 2.700000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.s.f6 = input.s.f6
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 5, i32 0, i8 0, float 2.800000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 5, i32 0, i8 1, float 2.900000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 5, i32 0, i8 2, float 3.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.f7 = input.f7
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 6, i32 0, i8 0, i32 1)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f1 = input.f1
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 7, i32 0, i8 0, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 7, i32 0, i8 1, float 2.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 7, i32 0, i8 2, float 3.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 7, i32 0, i8 3, float 4.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f3[4] = input.f3[4]
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 0, i8 0, float 1.300000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 0, i8 1, float 1.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 1, i8 0, float 1.500000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 1, i8 1, float 1.600000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 2, i8 0, float 1.700000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 2, i8 1, float 1.800000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 3, i8 0, float 1.900000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 9, i32 3, i8 1, float 2.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f4 = input.f4
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 10, i32 0, i8 0, i32 21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 10, i32 0, i8 1, i32 22)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 10, i32 0, i8 2, i32 23)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f5 = input.f5
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 11, i32 0, i8 0, float 2.400000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 11, i32 0, i8 1, float 2.500000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 11, i32 0, i8 2, float 2.600000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 11, i32 0, i8 3, float 2.700000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f6 = input.f6
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 12, i32 0, i8 0, float 2.800000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 12, i32 0, i8 1, float 2.900000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 12, i32 0, i8 2, float 3.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f7 = input.f7
// CHECK: call void  @dx.op.storeOutput.i32(i32 5, i32 13, i32 0, i8 0, i32 1)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o1.f2 (column_major) = input.f2
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 5.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 7.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float 9.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float 1.100000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 0, float 6.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 1, float 8.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 2, float 1.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 3, float 1.200000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

// o2.f2 (column_major) = input.f2
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 0, i8 0, float 5.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 0, i8 1, float 7.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 0, i8 2, float 9.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 0, i8 3, float 1.100000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 1, i8 0, float 6.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 1, i8 1, float 8.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 1, i8 2, float 1.000000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void  @dx.op.storeOutput.f32(i32 5, i32 8, i32 1, i8 3, float 1.200000e+01)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)

struct VSIn_1
{
    float4 f5 : VIN4;
    float3 f6 : VIN5;
};
    
struct VSIn
{
    float4 f1 : VIN0;
    float4x2 f2 : VIN1;
    float2 f3[4] : VIN2;
    uint3 f4 : VIN3;
    VSIn_1 s;
    bool f7 : VIN6;
};

struct VSOut_1
{
    float4 f5 : VOUT4;
    float3 f6 : VOUT5;
};
   
   
struct VSOut
{
    float4 f1 : VOUT0;
    float4x2 f2 : VOUT1;
    float2 f3[4] : VOUT2;
    uint3 f4 : VOUT3;
    VSOut_1 s;
    bool f7 : VOUT6;
};

void main(out VSOut o1 : A, out VSOut o2 : B)
{
   VSIn input;
   input.f1 = float4(1, 2, 3, 4);
   input.f2[0][0] = 5;
   input.f2[0][1] = 6;
   input.f2[1][0] = 7;
   input.f2[1][1] = 8;
   input.f2[2][0] = 9;
   input.f2[2][1] = 10;
   input.f2[3][0] = 11;
   input.f2[3][1] = 12;
   input.f3[0] = float2(13, 14);
   input.f3[1] = float2(15, 16);
   input.f3[2] = float2(17, 18);
   input.f3[3] = float2(19, 20);   
   input.f4 = uint3(21, 22, 23);
   input.s.f5 = float4(24, 25, 26, 27);
   input.s.f6 = float3(28, 29, 30);
   input.f7 = true;
   o1 = input;
   o2 = (VSOut)input;
}
