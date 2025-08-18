// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s --check-prefix AST
// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL

// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetHitKind
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetHitKind 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetHitKind 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 366
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetInstanceID
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetInstanceID 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetInstanceID 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 367
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetInstanceIndex
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetInstanceIndex 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetInstanceIndex 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 368
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetObjectRayDirection
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetObjectRayDirection 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetObjectRayDirection 'vector<float, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'vector<float, 3>':'vector<float, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 369
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetObjectRayOrigin
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetObjectRayOrigin 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetObjectRayOrigin 'vector<float, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'vector<float, 3>':'vector<float, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 370
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetObjectToWorld3x4
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetObjectToWorld3x4 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetObjectToWorld3x4 'matrix<float, 3, 4> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'matrix<float, 3, 4>':'matrix<float, 3, 4>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 371
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetObjectToWorld4x3
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetObjectToWorld4x3 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetObjectToWorld4x3 'matrix<float, 4, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'matrix<float, 4, 3>':'matrix<float, 4, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 372
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetPrimitiveIndex
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetPrimitiveIndex 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetPrimitiveIndex 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 373
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetRayFlags
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetRayFlags 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetRayFlags 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 374
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetRayTCurrent
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetRayTCurrent 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetRayTCurrent 'float ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'float'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 375
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetRayTMin
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetRayTMin 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetRayTMin 'float ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'float'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 376
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetShaderTableIndex
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetShaderTableIndex 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetShaderTableIndex 'unsigned int ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 377
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetWorldRayDirection
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetWorldRayDirection 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetWorldRayDirection 'vector<float, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'vector<float, 3>':'vector<float, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 378
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetWorldRayOrigin
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetWorldRayOrigin 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetWorldRayOrigin 'vector<float, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'vector<float, 3>':'vector<float, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 379
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetWorldToObject3x4
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetWorldToObject3x4 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetWorldToObject3x4 'matrix<float, 3, 4> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'matrix<float, 3, 4>':'matrix<float, 3, 4>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 380
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> GetWorldToObject4x3
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit GetWorldToObject4x3 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used GetWorldToObject4x3 'matrix<float, 4, 3> ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'matrix<float, 4, 3>':'matrix<float, 4, 3>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 381
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> IsHit
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit IsHit 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used IsHit 'bool ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'bool'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 383
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> IsMiss
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit IsMiss 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used IsMiss 'bool ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'bool'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 384
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> IsNop
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit IsNop 'TResult () const'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used IsNop 'bool ()' extern
// AST-NEXT: | | |   |-TemplateArgument type 'bool'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 385
// AST-NEXT: | | |   |-ConstAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> LoadLocalRootTableConstant
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TRootConstantOffsetInBytes
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit LoadLocalRootTableConstant 'TResult (TRootConstantOffsetInBytes) const'
// AST-NEXT: | | | | `-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> RootConstantOffsetInBytes 'TRootConstantOffsetInBytes'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used LoadLocalRootTableConstant 'unsigned int (unsigned int)' extern
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> LoadLocalRootTableConstant 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 386
// AST-NEXT: | | |   |-PureAttr {{[^ ]+}} <<invalid sloc>> Implicit
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""
// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> SetShaderTableIndex
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TRecordIndex
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit SetShaderTableIndex 'TResult (TRecordIndex) const'
// AST-NEXT: | | | | `-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> RecordIndex 'TRecordIndex'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used SetShaderTableIndex 'void (unsigned int)' extern
// AST-NEXT: | | |   |-TemplateArgument type 'void'
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> SetShaderTableIndex 'unsigned int'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 388
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// FCGL: define void @"\01?main@@YAXXZ"() #0 {
// FCGL:   %{{[^ ]+}} = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %[[HIT:[^ ]+]])
// FCGL:   call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32 388, %dx.types.HitObject* %[[HIT]], i32 1)
// FCGL:   %{{[^ ]+}} = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 383, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 384, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 385, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 365, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 366, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 368, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 367, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 373, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 377, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.HitObject*, i32)"(i32 386, %dx.types.HitObject* %[[HIT]], i32 40)
// FCGL:   %{{[^ ]+}} = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 379, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 378, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 370, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 369, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call %class.matrix.float.3.4 @"dx.hl.op.rn.%class.matrix.float.3.4 (i32, %dx.types.HitObject*)"(i32 371, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call %class.matrix.float.4.3 @"dx.hl.op.rn.%class.matrix.float.4.3 (i32, %dx.types.HitObject*)"(i32 372, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call %class.matrix.float.3.4 @"dx.hl.op.rn.%class.matrix.float.3.4 (i32, %dx.types.HitObject*)"(i32 380, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call %class.matrix.float.4.3 @"dx.hl.op.rn.%class.matrix.float.4.3 (i32, %dx.types.HitObject*)"(i32 381, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 374, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call float @"dx.hl.op.rn.float (i32, %dx.types.HitObject*)"(i32 376, %dx.types.HitObject* %[[HIT]])
// FCGL:   %{{[^ ]+}} = call float @"dx.hl.op.rn.float (i32, %dx.types.HitObject*)"(i32 375, %dx.types.HitObject* %[[HIT]])
// FCGL:   ret void

RWByteAddressBuffer outbuf;

template <int M, int N>
float hashM(in matrix<float, M, N> mat) {
  float h = 0.f;
  for (int i = 0; i < M; ++i)
    for (int j = 0; j < N; ++j)
      h += mat[i][j];
  return h;
}

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  int isum = 0;
  float fsum = 0.0f;
  vector<float, 3> vsum = 0;

  ///// Setters
  hit.SetShaderTableIndex(1);

  ///// Getters

  // i1 accessors
  isum += hit.IsHit();
  isum += hit.IsMiss();
  isum += hit.IsNop();

  // i32 accessors
  isum += hit.GetGeometryIndex();
  isum += hit.GetHitKind();
  isum += hit.GetInstanceIndex();
  isum += hit.GetInstanceID();
  isum += hit.GetPrimitiveIndex();
  isum += hit.GetShaderTableIndex();
  isum += hit.LoadLocalRootTableConstant(40);

  // float3 accessors
  vsum += hit.GetWorldRayOrigin();
  vsum += hit.GetWorldRayDirection();
  vsum += hit.GetObjectRayOrigin();
  vsum += hit.GetObjectRayDirection();
  fsum += vsum[0] + vsum[1] + vsum[2];

  // matrix accessors
  fsum += hashM<3, 4>(hit.GetObjectToWorld3x4());
  fsum += hashM<4, 3>(hit.GetObjectToWorld4x3());
  fsum += hashM<3, 4>(hit.GetWorldToObject3x4());
  fsum += hashM<4, 3>(hit.GetWorldToObject4x3());

  // f32 accessors
  isum += hit.GetRayFlags();
  fsum += hit.GetRayTMin();
  fsum += hit.GetRayTCurrent();

  outbuf.Store(0, fsum);
  outbuf.Store(4, isum);
}
