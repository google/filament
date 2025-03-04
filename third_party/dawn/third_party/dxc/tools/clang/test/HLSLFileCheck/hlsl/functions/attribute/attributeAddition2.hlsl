// RUN: %dxc -T cs_6_6 -E mymain -ast-dump %s | FileCheck %s

// the purpose of this test is to verify that the implicit 
// compute hlsl shader attribute doesn't get incorrectly 
// added to declarations that are not the global-level
// entry point declaration, and the attribute should
// really only be added to exactly one decl.

// this parses through the class definition
// CHECK-LABEL: -HLSLNumThreadsAttr
// CHECK-NOT: -HLSLShaderAttr

// after the class, there should be no
// shader attribute, so we go to the next decl,
// the namespace decl:
// CHECK-LABEL: -ReturnStmt
// CHECK-NOT: -HLSLShaderAttr

// Now, we should check that the attribute got added correctly to the 
// entry point decl at the global level:

// CHECK-LABEL: -HLSLNumThreadsAttr
// CHECK-LABEL: -HLSLShaderAttr
// CHECK-SAME: "compute"

class foo {
	[numthreads(1,1,1)]
	void mymain(){
		return;
	}
};

namespace bar {
	void mymain(){
		return;
	}
};

[numthreads(1,1,1)]
void mymain(){
	return;
}
