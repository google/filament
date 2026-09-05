There is a confirmation bias problem when testing debug info using file-check.
Say your test file contains:

  // RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s
  // CHECK: foo
  void main() {}

Due to /Zi, the !dx.source.contents metadata will be present and contain a string
with the original source file. This means that the generated file contains your
"// CHECK: foo", and hence the "foo" itself, so the check will succeed by default!

The current workaround is to include the following in your test to explicitly match
the quoted source file:

  // Exclude quoted source file (see readme)
  // CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

This will match a metadata string containing \0A (newline), which should only appear
in the quoted source file. It will not match itself in the quoted source file because
the regex won't match itself, and even less the escaped version of itself.

Note that if you see a failure on that line, it means that something else before that
CHECK failed to match.
