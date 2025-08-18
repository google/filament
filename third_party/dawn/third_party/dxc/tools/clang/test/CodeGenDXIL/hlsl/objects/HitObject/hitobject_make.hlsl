// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL
// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL
// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s --check-prefix AST

// AST: | |-CXXRecordDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit referenced class HitObject definition
// AST-NEXT: | | |-FinalAttr {{[^ ]+}} <<invalid sloc>> Implicit final
// AST-NEXT: | | |-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST-NEXT: | | |-HLSLHitObjectAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |-FieldDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// AST-NEXT: | | |-CXXConstructorDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used HitObject 'void ()'
// AST-NEXT: | | | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 358
// AST-NEXT: | | | `-HLSLCXXOverloadAttr {{[^ ]+}} <<invalid sloc>> Implicit

// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> MakeMiss
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TRayFlags
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TMissShaderIndex
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TRay
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit MakeMiss 'TResult (TRayFlags, TMissShaderIndex, TRay) const' static
// AST-NEXT: | | | | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> RayFlags 'TRayFlags'
// AST-NEXT: | | | | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> MissShaderIndex 'TMissShaderIndex'
// AST-NEXT: | | | | `-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> Ray 'TRay'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used MakeMiss 'dx::HitObject (unsigned int, unsigned int, RayDesc)' static
// AST-NEXT: | | |   |-TemplateArgument type 'dx::HitObject'
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-TemplateArgument type 'RayDesc'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> RayFlags 'unsigned int'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> MissShaderIndex 'unsigned int'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> Ray 'RayDesc'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 387
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> MakeNop
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit MakeNop 'TResult () const' static
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used MakeNop 'dx::HitObject ()' static
// AST-NEXT: | | |   |-TemplateArgument type 'dx::HitObject'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 358
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// FCGL: %{{[^ ]+}} = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %{{[^ ]+}})
// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %{{[^ ]+}})
// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.RayDesc*)"(i32 387, %dx.types.HitObject* %{{[^ ]+}}, i32 0, i32 1, %struct.RayDesc* %{{[^ ]+}})
// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.RayDesc*)"(i32 387, %dx.types.HitObject* %{{[^ ]+}}, i32 0, i32 2, %struct.RayDesc* %{{[^ ]+}})

// Expect HitObject_Make* calls with identical parameters to be folded.
// DXIL:  {{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// DXIL-NOT:  {{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeNop
// DXIL:  %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32 265, i32 0, i32 1, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0x3FA99999A0000000, float 1.000000e+03)  ; HitObject_MakeMiss(RayFlags,MissShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax)
// DXIL-NOT:  %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32 265, i32 0, i32 1
// DXIL:  %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32 265, i32 0, i32 2, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0x3FA99999A0000000, float 1.000000e+03)  ; HitObject_MakeMiss(RayFlags,MissShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax)

void Use(in dx::HitObject hit) {
  dx::MaybeReorderThread(hit);
}

[shader("raygeneration")]
void main() {
  dx::HitObject nop;
  Use(nop);

  dx::HitObject nop2 = dx::HitObject::MakeNop();
  Use(nop2);

  RayDesc ray = {{0,0,0}, {0,0,1}, 0.05, 1000.0};
  dx::HitObject miss = dx::HitObject::MakeMiss(0, 1, ray);
  Use(miss);

  dx::HitObject miss2 = dx::HitObject::MakeMiss(0, 1, ray);
  Use(miss2);

  dx::HitObject miss3 = dx::HitObject::MakeMiss(0, 2, ray);
  Use(miss3);
}
