// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Verify that passing struct result of call as arg to another call does not
// generate extra call.

// CHECK-DAG: [[f:%[^ ]*]] = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0,
// CHECK-DAG: [[p:%[^ ]*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0,
// CHECK-DAG: [[factor:%[^ ]*]] = fmul fast float [[f]], 2.000000e+00
// CHECK-DAG: [[factor2:%[^ ]*]] = fadd fast float [[factor]], 1.000000e+00
// CHECK-DAG: [[ret:%[^ ]*]] = fmul fast float [[factor2]], [[p]]
// CHECK-DAG: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float [[ret]])

struct PayloadStruct {
  float Color;
};

static float factor = 1.0;

PayloadStruct MulPayload(in PayloadStruct Payload)
{
  Payload.Color *= factor;
  factor += 1.0;
  return Payload;
}

PayloadStruct AddPayload(in PayloadStruct Payload0, in PayloadStruct Payload1)
{
  Payload0.Color += Payload1.Color;
  return Payload0;
}

void main(PayloadStruct p : Payload,
	  	  float f : INPUT,
          out PayloadStruct OutputPayload : SV_Target) {
  factor = f;
  OutputPayload = AddPayload(MulPayload(p),
                             MulPayload(p));
}
