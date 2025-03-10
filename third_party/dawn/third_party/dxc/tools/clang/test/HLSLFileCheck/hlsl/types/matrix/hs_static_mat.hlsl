// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// Make sure not crash.
// CHECK:define void @"\01?patchConstantF{{[@$?.A-Za-z0-9_]+}}"()
// CHECK:define void @main()
struct ST
{
 float4x4 m;

};

cbuffer S : register (b1)
{
 float4x4 m;
 };

static const struct {

 float4x4 m;
} M = {m};

static ST staticST;

ST GetM()
{
 ST Result;
 Result.m= M.m;
 return Result; 
}

ST GetST()
{
 return GetM();
}

 struct HsInput
 {
  float4x4 mat : MAT;
  float4 Position : SV_Position;
 };

struct HsOutput
{
 float4x4 mat : MAT;
  float4 Position : SV_Position;
};

struct PatchConstantOutput
{

 float TessFactor[3] : SV_TessFactor;
 float InsideTessFactor : SV_InsideTessFactor;
 float4 m : M;
};

PatchConstantOutput patchConstantF( const OutputPatch<HsOutput, 3 > I )
 {
  PatchConstantOutput O = (PatchConstantOutput)0;

   staticST = GetST();
   O.m = mul (staticST.m, float4(1,3,2,3));
   return O;
}

 [domain("tri")]
 [patchconstantfunc("patchConstantF")]
 [outputcontrolpoints( 3 )]
 [maxtessfactor(12)]
 [partitioning("fractional_even")][outputtopology("triangle_cw")]
 HsOutput main( InputPatch< HsInput , 12 > I, uint ControlPointID : SV_OutputControlPointID )
 {

staticST = GetST();

HsOutput O = (HsOutput) 0;
  O.mat = staticST.m;
return O;
}
