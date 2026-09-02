// RUN: %dxc -T vs_6_0 -HV 2021 -ast-dump %s | FileCheck %s

// CHECK-NOT: error

// CHECK: FunctionDecl {{.*}} used genericDoStuff 'void (Foo)'
// CHECK-NEXT: TemplateArgument
// CHECK-NEXT: ParmVarDecl
// CHECK-NEXT: CompoundStmt
// CHECK-NEXT: CallExpr
// CHECK-NEXT: ImplicitCastExpr
// CHECK-NEXT: DeclRefExpr
// CHECK-NEXT: DeclRefExpr
// CHECK-SAME: 'Food' EnumConstant
// CHECK-SAME: 'Pizza' 'Food'

enum class Food { Pizza };

void write(Food f, uint val) {}

template <typename Generic>
void genericDoStuff(Generic g)
{
    write(Food::Pizza, g.get());
}

class Foo {
    uint get() { return 0; }
};

void main() {
    Foo foo;
    genericDoStuff(foo);
}
