// RUN: %dxc -T lib_6_8 -ast-dump-implicit %s | FileCheck %s
// This test verifies the AST of the NodeOutputArray type. The
// source doesn't matter except that it forces a use to ensure the AST is fully
// loaded by the external sema source.

struct RECORD1
{
  uint value;
  uint value2;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_1(
    [UnboundedSparseNodes] [MaxRecords(37)]
    NodeOutputArray<RECORD1> OutputArray_1_1) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_1[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

// CHECK:|-ClassTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit ThreadNodeOutputRecords
// CHECK-NEXT:| |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class recordType
// CHECK-NEXT:| |-CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit struct ThreadNodeOutputRecords definition
// CHECK-NEXT:| | |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:| | |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit ThreadNodeOutputRecords
// CHECK-NEXT:| | |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:| | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'recordType &(unsigned int)'
// CHECK-NEXT:| | | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:| | | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:| | | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'const recordType &(unsigned int) const'
// CHECK-NEXT:| | | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:| | | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:| | | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Get 'recordType &(unsigned int)'
// CHECK-NEXT:| | | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int' cinit
// CHECK-NEXT:| | | | `-IntegerLiteral 0x{{.+}} <<invalid sloc>> 'unsigned int' 0
// CHECK-NEXT:| | | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:| | | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Get 'const recordType &(unsigned int) const'
// CHECK-NEXT:| | | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int' cinit
// CHECK-NEXT:| | | | `-IntegerLiteral 0x{{.+}} <<invalid sloc>> 'unsigned int' 0
// CHECK-NEXT:| | | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:| | | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| | |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[]
// CHECK-NEXT:| | | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class recordType
// CHECK-NEXT:| | | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'recordType &(unsigned int) const'
// CHECK-NEXT:| | |   |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT:| | |   `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| | `-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> OutputComplete
// CHECK-NEXT:| |   |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:| |   `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit OutputComplete 'TResult () const'
// CHECK-NEXT:| `-ClassTemplateSpecializationDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> struct ThreadNodeOutputRecords definition
// CHECK-NEXT:|   |-TemplateArgument type 'RECORD1'
// CHECK-NEXT:|   |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:|   |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit ThreadNodeOutputRecords
// CHECK-NEXT:|   |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:|   |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'RECORD1 &(unsigned int)'
// CHECK-NEXT:|   | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:|   | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:|   | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:|   |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'const RECORD1 &(unsigned int) const'
// CHECK-NEXT:|   | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:|   | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:|   | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:|   |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Get 'RECORD1 &(unsigned int)'
// CHECK-NEXT:|   | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:|   | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:|   | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:|   |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Get 'const RECORD1 &(unsigned int) const'
// CHECK-NEXT:|   | |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
// CHECK-NEXT:|   | |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "subscript" "" 0
// CHECK-NEXT:|   | `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:|   |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[]
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class recordType
// CHECK-NEXT:|   | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'recordType &(unsigned int) const'
// CHECK-NEXT:|   |   |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT:|   |   `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:|   |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> OutputComplete
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:|   | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> OutputComplete 'TResult () const'
// CHECK-NEXT:|   | `-CXXMethodDecl 0x[[OutComplete:[0-9a-f]+]] <<invalid sloc>> <invalid sloc> used OutputComplete 'void ()' extern
// CHECK-NEXT:|   |   |-TemplateArgument type 'void'
// CHECK-NEXT:|   |   `-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "op" "" {{[0-9]+}}
// CHECK-NEXT:|   `-CXXDestructorDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced ~ThreadNodeOutputRecords 'void () noexcept' inline

// CHECK:|-ClassTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit NodeOutput
// CHECK-NEXT:| |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class recordtype
// CHECK-NEXT:| |-CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit struct NodeOutput definition
// CHECK-NEXT:| | |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:| | |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit NodeOutput
// CHECK-NEXT:| | |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:| | |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetGroupNodeOutputRecords
// CHECK-NEXT:| | | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:| | | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TnumRecords
// CHECK-NEXT:| | | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit GetGroupNodeOutputRecords 'TResult (TnumRecords) const'
// CHECK-NEXT:| | |   `-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> numRecords 'TnumRecords'
// CHECK-NEXT:| | |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetThreadNodeOutputRecords
// CHECK-NEXT:| | | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:| | | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TnumRecords
// CHECK-NEXT:| | | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit GetThreadNodeOutputRecords 'TResult (TnumRecords) const'
// CHECK-NEXT:| | |   `-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> numRecords 'TnumRecords'
// CHECK-NEXT:| | `-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> IsValid
// CHECK-NEXT:| |   |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:| |   `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit IsValid 'TResult () const'
// CHECK-NEXT:| `-ClassTemplateSpecializationDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> struct NodeOutput definition
// CHECK-NEXT:|   |-TemplateArgument type 'RECORD1'
// CHECK-NEXT:|   |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:|   |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit NodeOutput
// CHECK-NEXT:|   |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:|   |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetGroupNodeOutputRecords
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TnumRecords
// CHECK-NEXT:|   | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetGroupNodeOutputRecords 'TResult (TnumRecords) const'
// CHECK-NEXT:|   |   `-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> numRecords 'TnumRecords'
// CHECK-NEXT:|   |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetThreadNodeOutputRecords
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TnumRecords
// CHECK-NEXT:|   | |-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetThreadNodeOutputRecords 'TResult (TnumRecords) const'
// CHECK-NEXT:|   | | `-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> numRecords 'TnumRecords'
// CHECK-NEXT:|   | `-CXXMethodDecl 0x[[GetThreadNodeOutputRecords:[0-9a-f]+]] <<invalid sloc>> <invalid sloc> used GetThreadNodeOutputRecords 'ThreadNodeOutputRecords<RECORD1> (unsigned int)' extern
// CHECK-NEXT:|   |   |-TemplateArgument type 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>'
// CHECK-NEXT:|   |   |-TemplateArgument type 'unsigned int'
// CHECK-NEXT:|   |   |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> GetThreadNodeOutputRecords 'unsigned int'
// CHECK-NEXT:|   |   `-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "op" "" {{[0-9]+}}
// CHECK-NEXT:|   |-FunctionTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> IsValid
// CHECK-NEXT:|   | |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class TResult
// CHECK-NEXT:|   | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> IsValid 'TResult () const'
// CHECK-NEXT:|   `-CXXDestructorDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced ~NodeOutput 'void () noexcept' inline

// CHECK:|-ClassTemplateDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit NodeOutputArray
// CHECK-NEXT:| |-TemplateTypeParmDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> class recordtype
// CHECK-NEXT:| |-CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit struct NodeOutputArray definition
// CHECK-NEXT:| | |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:| | |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit NodeOutputArray
// CHECK-NEXT:| | |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:| | `-CXXMethodDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> operator[] 'NodeOutput<recordtype> (unsigned int)'
// CHECK-NEXT:| |   |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT:| |   |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "indexnodehandle" "" {{[0-9]+}}
// CHECK-NEXT:| |   `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit
// CHECK-NEXT:| `-ClassTemplateSpecializationDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> struct NodeOutputArray definition
// CHECK-NEXT:|   |-TemplateArgument type 'RECORD1'
// CHECK-NEXT:|   |-FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// CHECK-NEXT:|   |-HLSLNodeObjectAttr 0x{{.+}} <<invalid sloc>> Implicit NodeOutputArray
// CHECK-NEXT:|   |-FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
// CHECK-NEXT:|   `-CXXMethodDecl 0x[[SUB:[0-9a-f]+]] <<invalid sloc>> <invalid sloc> used operator[] 'NodeOutput<RECORD1> (unsigned int)'
// CHECK-NEXT:|     |-ParmVarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT:|     |-HLSLIntrinsicAttr 0x{{.+}} <<invalid sloc>> Implicit "indexnodehandle" "" {{[0-9]+}}
// CHECK-NEXT:|     `-HLSLCXXOverloadAttr 0x{{.+}} <<invalid sloc>> Implicit

// CHECK:`-FunctionDecl 0x{{.+}} <line:16:1, line:21:1> line:16:6 node_1_1 'void (NodeOutputArray<RECORD1>)'
// CHECK-NEXT:  |-ParmVarDecl 0x[[ParmVar:[0-9a-f]+]] <line:18:5, col:30> col:30 used OutputArray_1_1 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// CHECK-NEXT:  | |-HLSLMaxRecordsAttr 0x{{.+}} <line:17:29, col:42> 37
// CHECK-NEXT:  | `-HLSLUnboundedSparseNodesAttr 0x{{.+}} <col:6>
// CHECK-NEXT:  |-CompoundStmt 0x{{.+}} <line:18:47, line:21:1>
// CHECK-NEXT:  | |-DeclStmt 0x{{.+}} <line:19:3, col:93>
// CHECK-NEXT:  | | `-VarDecl 0x[[OutRec:[0-9a-f]+]] <col:3, col:92> col:36 used outRec 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>' cinit
// CHECK-NEXT:  | |   `-CXXMemberCallExpr 0x{{.+}} <col:45, col:92> 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>'
// CHECK-NEXT:  | |     |-MemberExpr 0x{{.+}} <col:45, col:64> '<bound member function type>' .GetThreadNodeOutputRecords 0x[[GetThreadNodeOutputRecords]]
// CHECK-NEXT:  | |     | `-CXXOperatorCallExpr 0x{{.+}} <col:45, col:62> 'NodeOutput<RECORD1>':'NodeOutput<RECORD1>'
// CHECK-NEXT:  | |     |   |-ImplicitCastExpr 0x{{.+}} <col:60, col:62> 'NodeOutput<RECORD1> (*)(unsigned int)' <FunctionToPointerDecay>
// CHECK-NEXT:  | |     |   | `-DeclRefExpr 0x{{.+}} <col:60, col:62> 'NodeOutput<RECORD1> (unsigned int)' lvalue CXXMethod 0x[[SUB]] 'operator[]' 'NodeOutput<RECORD1> (unsigned int)'
// CHECK-NEXT:  | |     |   |-DeclRefExpr 0x{{.+}} <col:45> 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>' lvalue ParmVar 0x[[ParmVar]] 'OutputArray_1_1' 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// CHECK-NEXT:  | |     |   `-ImplicitCastExpr 0x{{.+}} <col:61> 'unsigned int' <IntegralCast>
// CHECK-NEXT:  | |     |     `-IntegerLiteral 0x{{.+}} <col:61> 'literal int' 1
// CHECK-NEXT:  | |     `-ImplicitCastExpr 0x{{.+}} <col:91> 'unsigned int' <IntegralCast>
// CHECK-NEXT:  | |       `-IntegerLiteral 0x{{.+}} <col:91> 'literal int' 2
// CHECK-NEXT:  | `-CXXMemberCallExpr 0x{{.+}} <line:20:3, col:25> 'void'
// CHECK-NEXT:  |   `-MemberExpr 0x{{.+}} <col:3, col:10> '<bound member function type>' .OutputComplete 0x[[OutComplete]]
// CHECK-NEXT:  |     `-DeclRefExpr 0x{{.+}} <col:3> 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>' lvalue Var 0x[[OutRec]] 'outRec' 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>'
// CHECK-NEXT:  |-HLSLNumThreadsAttr 0x{{.+}} <line:15:2, col:20> 1 1 1
// CHECK-NEXT:  |-HLSLNodeDispatchGridAttr 0x{{.+}} <line:14:2, col:26> 1 1 1
// CHECK-NEXT:  |-HLSLNodeLaunchAttr 0x{{.+}} <line:13:2, col:27> "broadcasting"
// CHECK-NEXT:  `-HLSLShaderAttr 0x{{.+}} <line:12:2, col:15> "node"
