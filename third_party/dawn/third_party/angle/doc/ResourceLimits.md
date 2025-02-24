# Resource Limits

OpenGL ES 2.0 API is quite powerful but there are still some features that are
optional or allow for wide variability between implementations.

Applications that need more than the minimum values for these limits should
query the capabilities of the GL device and scale their usage based on the
device’s feature set. Failing to do so and assuming sufficient limits typically
results in reduced portability.

The various implementation dependent limits can be found in Tables 6.18 – 6.20
of the [OpenGL ES 2.0.25 specification]
(http://www.khronos.org/registry/gles/specs/2.0/es_full_spec_2.0.25.pdf).

# Capabilities

Capability                                 | ES 2.0 Minimum | ANGLE            | SM2   | SM3      | SM4+
:----------------------------------------- | :------------- | :--------------- | :---- | :------- | :-------
GL\_MAX\_VERTEX\_ATTRIBS                   | 8              | 16               |       |          |
GL\_MAX\_VERTEX\_UNIFORM\_VECTORS          | 128            | 254              |       |          |
GL\_MAX\_VERTEX\_TEXTURE\_IMAGE\_UNITS     | 0              | (fn1)            | 0     | 0        | 4
GL\_MAX\_VARYING\_VECTORS                  | 8              | (fn1)            | 8     | 10       | 10
GL\_MAX\_FRAGMENT\_UNIFORM\_VECTORS        | 16             | (fn1)            | 29    | 221      | 221
GL\_MAX\_TEXTURE\_IMAGE\_UNITS             | 8              | 16               |       |          |
GL\_MAX\_TEXTURE\_SIZE                     | 64             | 2048-16384 (fn1) |       |          |
GL\_MAX\_CUBE\_MAP\_SIZE                   | 16             | 2048-16384 (fn1) |       |          |
GL\_MAX\_RENDERBUFFER\_SIZE                | 1              | 2048-16384 (fn1) |       |          |
GL\_ALIASED\_POINT\_SIZE\_RANGE (min, max) | (1, 1)         | (fn2)            | (1,1) | (1, fn2) | (1, fn2)
GL\_ALIASED\_LINE\_WIDTH\_RANGE (min, max) | (1, 1)         | (1, 1)           |       |          |

## Notes

*   fn1: limits vary based on the underlying hardware capabilities
*   fn2: on SM3 or better hardware the max point size is D3DCAPS9.MaxPointSize
