// RUN: %dxc -T lib_6_x  %s | FileCheck %s

// resources in return/params allowed for lib_6_x
// CHECK: alloca %struct.T.hdl
// CHECK: store %dx.types.Handle
// CHECK: call void @"\01?resStruct{{[@$?.A-Za-z0-9_]+}}"(%struct.T2.hdl
// CHECK: %[[ptr:[^, ]+]] = getelementptr inbounds %struct.T2.hdl
// CHECK: %[[val:[^, ]+]] = load %dx.types.Handle, %dx.types.Handle* %[[ptr]]
// CHECK: call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %[[val]])

// Make sure save bitcast for global symbol and HLSL type.
// CHECK:i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?outputBuffer{{[@$?.A-Za-z0-9_]+}}" to %struct.RWByteAddressBuffer*), !"outputBuffer"
// CHECK:i32 1, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?outputBuffer2{{[@$?.A-Za-z0-9_]+}}" to %struct.RWByteAddressBuffer*), !"outputBuffer2"


struct T {
RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;
};

struct D {
  float4 a;
  int4 b;
};

struct T2 {
   RWStructuredBuffer<D> uav;
};

T2 resStruct(T t, uint2 id);

RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;

[shader("compute")]
[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
    T t = {outputBuffer,outputBuffer2};
    T2 t2 = resStruct(t, id);
    uint counter = t2.uav.IncrementCounter();
    t2.uav[counter].b.xy = id;
}
