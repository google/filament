// RUN: %dxc -Tlib_6_3 -Wno-unused-value -HV 2016 -verify %s
// RUN: %dxc -Tvs_6_0 -Wno-unused-value -HV 2016 -verify %s

void dead()
{
    // expected-note@+2 {{array 'array' declared here}}
    // expected-note@+1 {{array 'array' declared here}}
    int array[2];
    array[-1] = 0;                                          /* expected-warning {{array index -1 is before the beginning of the array}} fxc-pass {{}} */
    array[0] = 0;
    array[1] = 0;
    array[2] = 0;                                           /* expected-warning {{array index 2 is past the end of the array (which contains 2 elements)}} fxc-pass {{}} */
}
[shader("vertex")]
void main() {}
