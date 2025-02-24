// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Verify that modificaion of in-only struct parameter does not modify
// the value passed in by the caller.

// CHECK-DAG: [[f:%[^ ]*]] = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0,
// CHECK-DAG: [[p:%[^ ]*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0,
// CHECK-DAG: [[o1:%[^ ]*]] = fmul fast float [[p]], [[f]]
// CHECK-DAG: [[ret:%[^ ]*]] = fadd fast float [[o1]], [[p]]
// CHECK-DAG: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float [[ret]])

struct PayloadStruct {
  float Color;
};

PayloadStruct MulPayload(in PayloadStruct Payload, in float x)
{
  Payload.Color *= x;
  return Payload;
}

void main(PayloadStruct p : Payload,
          float f : INPUT,
          out PayloadStruct o : SV_Target) {

  o = MulPayload(p, f);
  o.Color += p.Color;
}
