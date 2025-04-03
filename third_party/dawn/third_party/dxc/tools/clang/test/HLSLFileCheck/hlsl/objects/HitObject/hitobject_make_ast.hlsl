// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s

// CHECK: | |-CXXRecordDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit referenced class HitObject definition
// CHECK-NEXT: | | |-FinalAttr {{[^ ]+}} <<invalid sloc>> Implicit final
// CHECK-NEXT: | | |-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// CHECK-NEXT: | | |-HLSLHitObjectAttr {{[^ ]+}} <<invalid sloc>> Implicit
// CHECK-NEXT: | | |-FieldDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT: | | |-CXXConstructorDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used HitObject 'void ()'
// CHECK-NEXT: | | | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 358
// CHECK-NEXT: | | | `-HLSLCXXOverloadAttr {{[^ ]+}} <<invalid sloc>> Implicit

// CHECK: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> MakeNop
// CHECK-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit MakeNop 'TResult () const' static
// CHECK-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used MakeNop 'dx::HitObject ()' static
// CHECK-NEXT: | | |   |-TemplateArgument type 'dx::HitObject'
// CHECK-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 358
// CHECK-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  dx::HitObject::MakeNop();
}
