// RUN: %clang_cc1 -fprofile-instr-generate -fcoverage-mapping -dump-coverage-mapping -emit-llvm-only -main-file-name switch.c %s | FileCheck %s
                    // CHECK: foo
void foo(int i) {   // CHECK-NEXT: File 0, [[@LINE]]:17 -> [[@LINE+8]]:2 = #0
  switch(i) {
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+3]]:10 = #2
    return;
  case 2:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:10 = #3
    break;
  }
  int x = 0;        // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:2 = #1
}

void nop() {}

                    // CHECK: bar
void bar(int i) {   // CHECK-NEXT: File 0, [[@LINE]]:17 -> [[@LINE+20]]:2 = #0
  switch (i)
    ;               // CHECK-NEXT: File 0, [[@LINE]]:5 -> [[@LINE]]:6 = 0

  switch (i) {      // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+16]]:2 = #1
  }

  switch (i)        // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+13]]:2 = #2
    nop();          // CHECK-NEXT: File 0, [[@LINE]]:5 -> [[@LINE]]:10 = 0

  switch (i)        // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+10]]:2 = #3
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:10 = #5
    nop();

  switch (i) {      // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+6]]:2 = #4
    nop();          // CHECK-NEXT: File 0, [[@LINE]]:5 -> [[@LINE+2]]:10 = 0
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:10 = #7
    nop();
  }
  nop();            // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:2 = #6
}

                    // CHECK-NEXT: main
int main() {        // CHECK-NEXT: File 0, [[@LINE]]:12 -> [[@LINE+34]]:2 = #0
  int i = 0;
  switch(i) {
  case 0:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+7]]:10 = #2
    i = 1;
    break;
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+2]]:10 = #3
    i = 2;
    break;
  default:          // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:10 = #4
    break;
  }
  switch(i) {       // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+22]]:2 = #1
  case 0:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+6]]:10 = #6
    i = 1;
    break;
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+3]]:10 = #7
    i = 2;
  default:          // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:10 = (#7 + #8)
    break;
  }

  switch(i) {       // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+12]]:2 = #5
  case 1:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+5]]:11 = #10
  case 2:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+4]]:11 = (#10 + #11)
    i = 11;
  case 3:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+2]]:11 = ((#10 + #11) + #12)
  case 4:           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+1]]:11 = (((#10 + #11) + #12) + #13)
    i = 99;
  }

  foo(1);           // CHECK-NEXT: File 0, [[@LINE]]:3 -> [[@LINE+2]]:11 = #9
  bar(1);
  return 0;
}
