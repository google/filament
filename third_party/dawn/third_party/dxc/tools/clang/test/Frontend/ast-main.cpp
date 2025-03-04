// RUN: %clang -emit-llvm -S -o %t1.ll -x c++ - < %s
// RUN: %clang -fno-delayed-template-parsing -emit-ast -o %t.ast %s
// RUN: %clang -emit-llvm -S -o %t2.ll -x ast - < %t.ast
// RUN: diff %t1.ll %t2.ll

// http://llvm.org/bugs/show_bug.cgi?id=15377
template<typename T>
struct S {
    T *mf();
};
template<typename T>
T *S<T>::mf() {
    // warning: control reaches end of non-void function [-Wreturn-type]
}

void f() {
    S<int>().mf();
}

int main() {
  return 0;
}
