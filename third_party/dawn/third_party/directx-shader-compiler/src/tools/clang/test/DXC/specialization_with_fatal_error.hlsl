// RUN: %dxc -T lib_6_8 -verify %s

// Clang suppresses template specialization if a fatal error has been
// registered (this reduces the risk of a cascade of secondary errors).
// However, DXC asserted if a template specialization failed - which
// prevented the error diagnostic being generated.
// We check here that an assert is no longer raised if a fatal error
// has been registered, and that the error diagnostic is generated.

float a;

// the include file doesn't exist - this should produce a fatal error diagnostic
// expected-error@+1 {{'a.h' file not found}}
#include "a.h"

void b() {};

int3 c(int X) {
  // an assert was triggered for the expression below when include file a.h
  // doesn't exist, and the error diagnostic expected above was not produced.
  return X.xxx;
}
