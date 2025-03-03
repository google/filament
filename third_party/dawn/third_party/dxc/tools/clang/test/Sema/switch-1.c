// RUN: %clang_cc1 -fsyntax-only -verify -triple x86_64-apple-darwin10 %s
// RUN: %clang_cc1 -x c++ -fsyntax-only -verify -triple x86_64-apple-darwin10 %s
// rdar://11577384
// rdar://13423975

int f(int i) {
  switch (i) {
    case 2147483647 + 2: // expected-warning {{overflow in expression; result is -2147483647 with type 'int'}}
      return 1;
    case 9223372036854775807L * 4: // expected-warning {{overflow in expression; result is -4 with type 'long'}}
      return 2;
    case (123456 *789012) + 1:  // expected-warning {{overflow in expression; result is -1375982336 with type 'int'}}
      return 3;
    case (2147483647*4)/4: 	// expected-warning {{overflow in expression; result is -4 with type 'int'}}
    case (2147483647*4)%4: 	// expected-warning {{overflow in expression; result is -4 with type 'int'}}
      return 4;
    case 2147483647:
      return 0;
  }
  return (i, 65537) * 65537; // expected-warning {{overflow in expression; result is 131073 with type 'int'}} \
			     // expected-warning {{expression result unused}}
}

// rdar://18405357
unsigned long long l = 65536 * 65536; // expected-warning {{overflow in expression; result is 0 with type 'int'}}
unsigned long long l2 = 65536 * (unsigned)65536;
unsigned long long l3 = 65536 * 65536ULL;
