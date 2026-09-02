// RUN: %dxc -T lib_6_5 -verify %s

// expect no diagnostics here, since global variables
// are not picked up through the recursive AST visitor's
// traversal of the exported function.
int x = RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;

export float4 MyExportedFunction(float4 color) {
    // expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model lib_6_5; introduced in shader model 6.9}}
    return color * RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;
}
