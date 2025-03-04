// RUN: %dxc -EMSMain -enable-16bit-types -Tms_6_6 %s | %opt -S -hlsl-dxil-pix-meshshader-output-instrumentation,expand-payload=0,dispatchArgY=3,dispatchArgZ=7,UAVSize=8192 | %FileCheck %s

// Check that the instrumentation properly coerces different output types into int32

// CHECK: [[PAYLOAD:%.*]]  = call %struct.MyPayload* @dx.op.getMeshPayload.struct.MyPayload(i32 170)

// FP16 value in 0th member
// CHECK: [[F16:%.*]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* [[PAYLOAD]], i32 0, i32 0
// CHECK: [[F16LOADED:%.*]] = load half, half* [[F16]]
// CHECK: [[F16CONV:%.*]] = fpext half [[F16LOADED]] to float

// uint16 value in 1st member
// CHECK: [[U16:%.*]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* [[PAYLOAD]], i32 0, i32 1
// CHECK: [[U16LOADED:%.*]] = load i16, i16* [[U16]]
// CHECK: [[U16CONV:%.*]] = uitofp i16 [[U16LOADED]] to float

// int16 value in 2nd member
// CHECK: [[I16:%.*]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* [[PAYLOAD]], i32 0, i32 2
// CHECK: [[I16LOADED:%.*]] = load i16, i16* [[I16]]
// CHECK: [[I16CONV:%.*]] = sitofp i16 [[I16LOADED]] to float

// float value in 3rd member
// CHECK: [[FP:%.*]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* [[PAYLOAD]], i32 0, i32 3
// CHECK: [[FPLOADED:%.*]] = load float, float* [[FP]]


// Check that these converted values are written out:
// CHECK: [[F16COERCED:%.*]] = bitcast float [[F16CONV]] to i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[ANYRESOURCE:%.*]], i32 [[ANYOFFSET:%.*]], i32 undef, i32 [[F16COERCED]]
// CHECK: [[U16COERCED:%.*]] = bitcast float [[U16CONV]] to i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[ANYRESOURCE:%.*]], i32 [[ANYOFFSET:%.*]], i32 undef, i32 [[U16COERCED]]
// CHECK: [[I16COERCED:%.*]] = bitcast float [[I16CONV]] to i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[ANYRESOURCE:%.*]], i32 [[ANYOFFSET:%.*]], i32 undef, i32 [[I16COERCED]]
// CHECK: [[FPCOERCED:%.*]] = bitcast float [[FPLOADED]] to i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[ANYRESOURCE:%.*]], i32 [[ANYOFFSET:%.*]], i32 undef, i32 [[FPCOERCED]]

struct PSInput
{
    float4 position : SV_POSITION;
};

struct MyPayload
{
  half f16;
  uint16_t u16;
  int16_t i16;
  float f;
};

[outputtopology("triangle")]
[numthreads(4, 1, 1)]
void MSMain(
    in payload MyPayload small,
    in uint tid : SV_GroupThreadID,
    out vertices PSInput verts[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);
    verts[tid].position = float4(small.f16, small.u16, small.i16, small.f);
    triangles[tid % 2] = uint3(0, tid + 1, tid + 2);
}
