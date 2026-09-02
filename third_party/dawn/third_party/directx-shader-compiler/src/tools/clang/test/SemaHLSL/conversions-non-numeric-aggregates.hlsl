// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tvs_6_0 -Wno-unused-value -verify %s

// Tests that conversions between numeric and non-numeric types/aggregates are disallowed.

struct NumStruct { int a; };
struct ObjStruct { Buffer a; };
struct SpecializedObjStruct {
  int a;
  RWStructuredBuffer<float> b;
};
struct NestedSpecializedObjStruct {
  int a;
  SpecializedObjStruct b;
};
enum ResourceEnum { ResourceEnumValue };

[shader("vertex")]
void main()
{
  (Buffer[1])0; /* expected-error {{cannot convert from 'literal int' to 'Buffer [1]'}} fxc-error {{X3017: cannot convert from 'int' to 'Buffer<float4>[1]'}} */
  (ObjStruct)0; /* expected-error {{cannot convert from 'literal int' to 'ObjStruct'}} fxc-error {{X3017: cannot convert from 'int' to 'struct ObjStruct'}} */
  (SpecializedObjStruct)0; /* expected-error {{cannot convert from 'literal int' to 'SpecializedObjStruct'}} */
  (NestedSpecializedObjStruct)0; /* expected-error {{cannot convert from 'literal int' to 'NestedSpecializedObjStruct'}} */
  (SpecializedObjStruct)ResourceEnumValue; /* expected-error {{cannot convert from 'ResourceEnum' to 'SpecializedObjStruct'}} */
  (Buffer[1])(int[1])0; /* expected-error {{cannot convert from 'int [1]' to 'Buffer [1]'}} fxc-error {{X3017: cannot convert from 'const int[1]' to 'Buffer<float4>[1]'}} */
  (ObjStruct)(NumStruct)0; /* expected-error {{cannot convert from 'NumStruct' to 'ObjStruct'}} fxc-error {{X3017: cannot convert from 'const struct NumStruct' to 'struct ObjStruct'}} */

  Buffer oa1[1];
  ObjStruct os1;
  (int)oa1; /* expected-error {{cannot convert from 'Buffer [1]' to 'int'}} fxc-error {{X3017: cannot convert from 'Buffer<float4>[1]' to 'int'}} */
  (int)os1; /* expected-error {{cannot convert from 'ObjStruct' to 'int'}} fxc-error {{X3017: cannot convert from 'struct ObjStruct' to 'int'}} */
}
