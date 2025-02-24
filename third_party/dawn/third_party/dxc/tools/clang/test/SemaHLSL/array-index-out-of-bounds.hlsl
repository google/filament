// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tvs_6_0  -Wno-unused-value -verify %s

[shader("vertex")]
void main()
{
    // expected-note@+2 {{array 'array' declared here}}
    // expected-note@+1 {{array 'array' declared here}}
    int array[2];
    array[-1] = 0;                                          /* expected-error {{array index -1 is out of bounds}} fxc-error {{X3504: array index out of bounds}} */
    array[0] = 0;
    array[1] = 0;
    array[2] = 0;                                           /* expected-error {{array index 2 is out of bounds}} fxc-error {{X3504: array index out of bounds}} */
}
