// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Regression test for a bug where two !DIFile debug metadata
// entries were created for the same file when a full path was passed in
// the command line, the first of which bogusly having the directory part twice:
// !DIFile(filename: "E:\5CdirE:\5Cdir\5Cfile.hlsl", directory: "")
// !DIFile(filename: "E:\5Cdir\5Cfile.hlsl", directory: "")

// Ensure we have only one DIFile and it has a path with a single drive component
// Note that we have to be careful not to match the "!DIFile" strings in the CHECK
// statements below, since the source code for this test will be preserved in a
// metadata element.

// CHECK-NOT: !DIFile(
// CHECK: !DIFile(filename: "{{[^:]*}}:{{[^:]*}}"
// CHECK-NOT: !DIFile(

// Exclude quoted source file (see readme)
// CHECK: {{!"[^"]*\\0A[^"]*"}}

Texture2D tex; // This is necessary for the second, non-bogus !DIFile
void main() {}