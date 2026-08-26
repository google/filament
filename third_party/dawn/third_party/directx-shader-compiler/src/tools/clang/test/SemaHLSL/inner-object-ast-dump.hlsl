// RUN: %dxc -T ps_6_0 -ast-dump-implicit %s | FileCheck %s

// This test verifies that the inner indexer object types backing .mips and
// .sample (mips_type / mips_slice_type, sample_type / sample_slice_type) carry
// the implicit HLSLNonAutoDeducibleAttr, so that 'auto' cannot deduce them.
// See also subobjects-ast-dump.hlsl, which checks the same attribute on
// subobject types.

Texture2D<float4> srv;
Texture2DMS<float4> srvMS;

float4 main() : SV_Target {
  return 0;
}

// The slice type is created before the indexer type, so it is dumped first;
// each carries the attribute as its first child. Texture2D (.mips) is referenced
// before Texture2DMS (.sample), so the mips records are dumped first.

// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit {{.*}}mips_slice_type definition
// CHECK-NEXT: HLSLNonAutoDeducibleAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit {{.*}}mips_type definition
// CHECK-NEXT: HLSLNonAutoDeducibleAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit {{.*}}sample_slice_type definition
// CHECK-NEXT: HLSLNonAutoDeducibleAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit {{.*}}sample_type definition
// CHECK-NEXT: HLSLNonAutoDeducibleAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
