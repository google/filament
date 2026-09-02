// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -enable-16bit-types %s | FileCheck %s

///////////////////////////////////////
// CHECK: !{void (%struct.Payload_20*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?anyhit1{{[@$?.A-Za-z0-9_]+}}",
// CHECK: !{i32 8, i32 9, i32 6, i32 20, i32 7, i32 8,

struct Payload_20 {
  float3 color;
  uint2 pos;
  // align 4
};

[shader("anyhit")]
void anyhit1( inout Payload_20 payload,
                  in BuiltInTriangleIntersectionAttributes attr )
{
}

///////////////////////////////////////
// CHECK: !{void (%struct.Params_16*)* @"\01?callable4{{[@$?.A-Za-z0-9_]+}}",
// CHECK: !{i32 8, i32 12, i32 6, i32 16

struct Params_16 {
  int64_t i;
  int16_t i16;
  // align 8
};

[shader("callable")]
void callable4( inout Params_16 params )
{
}

///////////////////////////////////////
// CHECK: !{void (%struct.Payload_16*, %struct.Attributes_12*)* @"\01?closesthit2{{[@$?.A-Za-z0-9_]+}}",
// CHECK: !{i32 8, i32 10, i32 6, i32 16, i32 7, i32 12

struct Payload_16 {
  half a;
  half2 h2;
  half b;
  half c;
  // pad 2 bytes
  uint u;
  // align 4
};

struct Attributes_12 {
  half a;
  // pad 2 bytes
  bool b;   // 4 bytes for bool
  uint16_t c;
  // align 4
};

[shader("closesthit")]
void closesthit2( inout Payload_16 payload,
                  in Attributes_12 attr )
{
}

///////////////////////////////////////
// CHECK: !{void (%struct.Payload_10*, %struct.Attributes_40*)* @"\01?closesthit3{{[@$?.A-Za-z0-9_]+}}",
// CHECK: !{i32 8, i32 10, i32 6, i32 10, i32 7, i32 40

struct Payload_10 {
  half4 color;
  int16_t i16;
  // align 2
};

struct Attributes_40 {
  half a;
  // pad 6 bytes
  double d;
  int16_t2 w2;
  // pad 4 bytes
  int64_t i;
  half h;
  // align 8
};

[shader("closesthit")]
void closesthit3( inout Payload_10 payload,
                  in Attributes_40 attr )
{
}

///////////////////////////////////////
// CHECK: !{void (%struct.Payload_8*)* @"\01?miss4{{[@$?.A-Za-z0-9_]+}}",
// CHECK: !{i32 8, i32 11, i32 6, i32 8

struct Payload_8 {
  half color;
  int16_t3 i16;
  // align 2
};

[shader("miss")]
void miss4( inout Payload_8 payload )
{
}
