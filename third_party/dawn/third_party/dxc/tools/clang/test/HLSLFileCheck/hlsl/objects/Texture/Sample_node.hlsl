// RUN: %dxc -T lib_6_8  %s | FileCheck %s

// Test SampleCmpBias and SampleCmpGrad for node shader.

// CHECK: define void {{.*}}@BackwardRef
// CHECK:  %[[Sampler:.+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?s@@3USamplerComparisonState@@A", align 4
// CHECK:  %[[T2D:.+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?tex2d@@3V?$Texture2D@V?$vector@M$03@@@@A", align 4

// CHECK: %[[T2DH:.+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %[[T2D]])  ; CreateHandleForLib(Resource)
// CHECK: %[[T2DAnnot:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2DH]], %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>

// CHECK: %[[SamplerH:.+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %[[Sampler]])  ; CreateHandleForLib(Resource)
// CHECK: %[[SamplerAnnot:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[SamplerH]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState

// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %[[T2DAnnot]], %dx.types.Handle %[[SamplerAnnot]], float %{{.*}}, float %{{.*}}, float undef, float undef, i32 -5, i32 7, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}})  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)

// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %[[T2DAnnot]], %dx.types.Handle %[[SamplerAnnot]], float %{{.*}}, float %{{.*}}, float undef, float undef, i32 7, i32 -5, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, float %{{.*}}, float %{{.*}}, float undef, float %{{.*}})  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)

struct rec0
{
    int i0;
    float2 f0;
};

struct rec1
{
    float f1;
    int i1;
};

Texture2D tex2d;
SamplerComparisonState s;

float cmpVal;
float bias;
float2 ddx;
float2 ddy;
bool clamp;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4, 1, 1)]
void BackwardRef(
  RWDispatchNodeInputRecord<rec0> InputyMcInputFace,
  [MaxRecords(5)] NodeOutput<rec1> Output1,
  [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output2)
{
    float2 coord = InputyMcInputFace.Get().f0;
    float r;
    r = tex2d.SampleCmpBias(s, coord, cmpVal, bias, uint2(-5, 7), clamp);
    r += tex2d.SampleCmpGrad(s, coord, cmpVal, ddx,ddy, uint2(7, -5), clamp);

    GroupNodeOutputRecords<rec1> outrec = Output1.GetGroupNodeOutputRecords(1);
    InterlockedCompareStoreFloatBitwise(outrec.Get().f1, 0.0, r);
}
