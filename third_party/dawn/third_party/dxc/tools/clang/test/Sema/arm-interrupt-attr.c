// RUN: %clang_cc1 %s -triple arm-apple-darwin -verify -fsyntax-only

__attribute__((interrupt(IRQ))) void foo() {} // expected-error {{'interrupt' attribute requires a string}}
__attribute__((interrupt("irq"))) void foo1() {} // expected-warning {{'interrupt' attribute argument not supported: irq}}

__attribute__((interrupt("IRQ", 1))) void foo2() {} // expected-error {{'interrupt' attribute takes no more than 1 argument}}

__attribute__((interrupt("IRQ"))) void foo3() {}
__attribute__((interrupt("FIQ"))) void foo4() {}
__attribute__((interrupt("SWI"))) void foo5() {}
__attribute__((interrupt("ABORT"))) void foo6() {}
__attribute__((interrupt("UNDEF"))) void foo7() {}

__attribute__((interrupt)) void foo8() {}
__attribute__((interrupt())) void foo9() {}
__attribute__((interrupt(""))) void foo10() {}
