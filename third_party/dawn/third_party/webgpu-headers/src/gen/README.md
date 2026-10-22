# gen

The generator for generating `webgpu.h` header and other extension headers or implementation specific headers from the yaml spec files.

# Generate implementation specific header

The generator also allows generating custom implementation specific headers that build on top of `webgpu.h` header. The generator accepts the combination of `-yaml` & `-header` flags in sequence, which it uses to validate the specifications and then generate their headers.

For example, if `wgpu.yml` contains the implementation specific API, the header can be generated using:

```shell
> go run ./gen -schema schema.json -yaml webgpu.yml -header webgpu.h -yaml wgpu.yml -header wgpu.h
```

Since the generator does some duplication validation, the order of the files matter, so generator mandates the core `webgpu.yml` to be first in the sequence.

# yaml spec

### Types

| Primitive types | Complex types     |
| --------------- | ----------------- |
| `bool`          | `enum.*`          |
| `string`        | `bitflag.*`       |
| `uint16`        | `struct.*`        |
| `uint32`        | `function_type.*` |
| `uint64`        | `object.*`        |
| `usize`         |
| `int16`         |
| `int32`         |
| `float32`       |
| `float64`       |
| `c_void`        |

| Structure types           |                                                        |
| ------------------------- | ------------------------------------------------------ |
| `extensible`              | structs that can be extended                           |
| `extensible_callback_arg` | structs used as callback args that can be extended     |
| `extension`               | extension structs used in `extensible` structs' chains |
| `standalone`              | structs that are neither extensions nor extensible     |

#### Arrays

Arrays are supported using a special syntax, for example `array<uint32>`. The generator will emit two fields for array types, one for element count and another for the pointer to the type.
