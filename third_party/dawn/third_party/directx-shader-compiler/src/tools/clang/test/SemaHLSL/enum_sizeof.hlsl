// RUN: %dxc -T cs_6_9 -E main %s -ast-dump-implicit | FileCheck %s --check-prefix AST

enum E1 : uint64_t
{
    v1 = 0,
};

enum E2 : uint32_t
{
    v2 = 0,
};

struct S {
  E1 e1;
  E2 e2;
};

RWBuffer<int> b;

[numthreads(128, 1, 1)]
void main()
{
// AST: UnaryExprOrTypeTraitExpr {{.*}} 'unsigned long' sizeof 'E1'
    b[0] = sizeof(E1);

// AST: UnaryExprOrTypeTraitExpr {{.*}} 'unsigned long' sizeof 'E2'
    b[1] = sizeof(E2);

// AST: UnaryExprOrTypeTraitExpr {{.*}} 'unsigned long' sizeof 'S'
    b[2] = sizeof(S);
}
