# Dawn's code generators.

Dawn relies on a lot of code generation to produce boilerplate code, especially webgpu.h-related code. They start by reading some JSON files (and sometimes XML too), process the data into an in-memory representation that's then used by some [Jinja2](https://jinja.palletsprojects.com/) templates to generate the code. This is similar to the model/view separation in Web development.

Generators are based on [generator_lib.py](../../generator/generator_lib.py) which provides facilities for integrating in build systems and using Jinja2. Templates can be found in [`generator/templates`](../../generator/templates) and the generated files are in `out/<Debug/Release/foo>/gen/src` when building Dawn in standalone. Generated files can also be found in [Chromium's code search](https://source.chromium.org/chromium/chromium/src/+/main:out/Debug/gen/third_party/dawn/src/).

## Dawn "JSON API" generators

Most of the code generation is done from [`dawn.json`](../../src/dawn/dawn.json) which is a JSON description of the WebGPU API with extra annotation used by some of the generators. The code for all the "Dawn JSON" generators is in [`dawn_json_generator.py`](../../generator/dawn_json_generator.py) (with templates in the regular template dir).

At this time it is used to generate:

 - the Dawn, Emscripten, and upstream webgpu-native `webgpu.h` C header
 - the Dawn and Emscripten `webgpu_cpp.h` C++ wrapper over the C header
 - libraries that implements `webgpu.h` by calling in a static or `thread_local` proc table
 - the [Emscripten](https://emscripten.org/) WebGPU binding implementation
 - a GMock version of the API with its proc table for testing
 - validation helper functions for dawn_native
 - the definition of dawn_native's proc table
 - dawn_native's internal version of the webgpu.h types
 - utilities for working with dawn_native's chained structs
 - a lot of dawn_wire parts, see below

Internally `dawn.json` is a dictionary from the "canonical name" of things to their definition. The "canonical name" is a space-separated (mostly) lower-case version of the name that's parsed into a `Name` Python object. Then that name can be turned into various casings with `.CamelCase()` `.SNAKE_CASE()`, etc. When `dawn.json` things reference each other, it is always via these "canonical names".

The `"_metadata"` key in the JSON file is used by flexible templates for generating various Web Standard API that contains following metadata:

 - `"api"` a string, the name of the Web API
 - `"namespace"` a string, the namespace of C++ wrapper
 - `"c_prefix"` (optional) a string, the prefix of C function and data type, it will default to upper-case of `"namespace"` if it's not provided.
 - `"proc_table_prefix"` a string, the prefix of proc table.
 - `"impl_dir"` a string, the directory of API implementation
 - `"native_namespace"` a string, the namespace of native implementation
 - `"copyright_year"` (optional) a string, templates will use the year of copyright.

The basic schema is that every entry is a thing with a `"category"` key what determines the sub-schema to apply to that thing. Categories and their sub-shema are defined below. Several parts of the schema use the concept of "record" which is a list of "record members" which are a combination of a type, a name and other metadata. For example the list of arguments of a function is a record. The list of structure members is a record. This combined concept is useful for the dawn_wire generator to generate code for structure and function calls in a very similar way.

Most items and sub-items can include a list of `"tags"`, which, if specified, conditionally includes the item if any of its tags appears in the `enabled_tags` configuration passed to `parse_json`. This is used to include and exclude various items for Dawn, Emscripten, or upstream header variants. Tags are applied in the "parse_json" step ([rather than later](https://docs.google.com/document/d/1fBniVOxx3-hQbxHMugEPcQsaXaKBZYVO8yG9iXJp-fU/edit?usp=sharing)): this has the benefit of automatically catching when, for a particular tag configuration, an included item references an excluded item.

When used on enum values, `"tags"` may add an additional prefix to the enum value. This is to implement [implementation-specific ranges of enums](https://github.com/webgpu-native/webgpu-headers/issues/214). All compat enums are to start at `0x0002_0000`, Emscripten enums are to start at `0x0004_0000`, and all Dawn enums are to start at `0x0005_0000`. Use of `"compat"`, `"emscripten"`, or `"dawn"` tags will add these values, respectively. So, an enum with value `"3"` but tag `"emscripten"` will actually use value `0x0002_0003`. Multi-implementation native enums start at `0x0001_0000`, so this value is added if the `"native"` tag is present and no other value-impacting tag is applied.

A **record** is a list of **record members**, each of which is a dictionary with the following schema:
 - `"name"` a string
 - `"type"` a string, the name of the base type for this member
 - `"annotation"` a string, default to "value". Define the C annotation to apply to the base type. Allowed annotations are `"value"` (the default), `"*"`, `"const*"`
 - `"length"` (default to 1 if not set), a string. Defines length of the array pointed to for pointer arguments. If not set the length is implicitly 1 (so not an array), but otherwise it can be set to the name of another member in the same record that will contain the length of the array (this is heavily used in the `fooCount` `foos` pattern in the API).
 - `"optional"` (default to false) a boolean that says whether this member is optional. Member records can be optional if they are pointers (otherwise dawn_wire will always try to dereference them), objects (otherwise dawn_wire will always try to encode their ID and crash), or if they have a `"default"` key. Optional pointers and objects will always default to `nullptr` (unless `"no_default"` is set to `true`).
 - `"default"` (optional) a number or string. If set the record member will use that value as default value. Depending on the member's category it can be a number, a string containing a number, or the name of an enum/bitmask value.
   - Dawn implements "trivial defaulting" for enums, similarly to the upstream WebGPU spec's WebIDL: if a zero-valued enum (usually called `Undefined`) is passed in, Dawn applies the default value specified here. See `WithTrivialFrontendDefaults()` in `api_structs.h` for how this works.
 - `"wire_is_data_only"` (default to false) a boolean that says whether it is safe to directly return a pointer of this member that is pointing to a piece of memory in the transfer buffer into dawn_wire. To prevent TOCTOU attacks, by default in dawn_wire we must ensure every single value returned to dawn_native a copy of what's in the wire, so `"wire_is_data_only"` is set to true only when the member is data-only and don't impact control flow.

**`"native"`** native types that can be referenced by name in other things.
 - `"wire transparent"` (defaults to true) a boolean that indicates whether the native type should be transparent (serialized as is) on the wire.

**`"typedef"`** (usually only used for gradual deprecations):
 - `"type"`: the name of the things this is a typedef for.

**`"enum"`** an `uint32_t`-based enum value.
 - `"values"` an array of enum values. Each value is a dictionary containing:
   - `"name"` a string
   - `"value"` a number that can be decimal or hexadecimal
   - `"jsrepr"` (optional) a string to allow overriding how this value map to Javascript for the Emscripten bits
   - `"valid"` (defaults to true) a boolean that controls whether the dawn_native validation utilities will consider this enum value valid.
 - `"emscripten_no_enum_table"` (optional) if true, skips generating an enum table in `library_webgpu_enum_tables.js`

**`"bitmask"`** an `uint32_t`-based bitmask. It is similar to **`"enum"`** but can be output differently.

**`"function pointer"`** defines a function pointer type that can be used by other things.
 - `"returns"` a string that's the name of the return type
 - `"args"` a **record**, so an array of **record members**

**`"structure"`**
 - `"members"` a **record**, so an array of **record members**
 - `"extensible"` (defaults to false) a boolean defining if this is an "extensible" WebGPU structure (i.e. has `nextInChain`). "descriptor" structures should usually have this set to true.
 - `"chained"` (defaults to None) a string defining if this is a structure that can be "chained" in a WebGPU structure (i.e. has `nextInChain` and `sType`) and in which direction ('in' for inputs to WebGPU, 'out' for outputs)
 - `"chain roots"` (defaults to []) a list of strings that are the canonical names of structures that can be extended by this structure.

**`"object"`**
 - `**methods**` an array of methods for this object. Note that "release" and "reference" don't need to be specified. Each method is a dictionary containing:
   - `"name"` a string
   - `"return_type"` (default to no return type) a string that's the name of the return type.
   - `"arguments"` a **record**, so an array of **record members**
   - `"no autolock"`: a boolean flag (default is false) indicates that the method's generated code won't automatically do thread synchronization. This flag can only be true on device or device child objects currently.
 - `"no autolock"`: a boolean flag (default is false) to indicate that the object's generated code won't automatically do thread synchronization. This will override individual method's `"no autolock"` flag. This flag can only be true on device or device child objects currently.

**`"constant"`**
 - `"type"`: a string, the name of the base data type
 - `"value"`: a string, the value is defined with preprocessor macro

**`"function"`** declares a function that not belongs to any class.
 - `"returns"` a string that's the name of the return type
 - `"args"` a **record**, so an array of **record members**

## Dawn "wire" generators

The generator for the pieces of dawn_wire need additional data which is found in [`dawn_wire_json`](../../src/dawn/dawn_wire.json). Examples of pieces that are generated are:

 - `WireCmd.cpp/.h` the most important piece: the meat of the serialization / deserialization code for WebGPU structures and commands
 - `ServerHandlers/Doers.cpp` that does the complete handling of all regular WebGPU methods in the server
 - `ApiProcs.cpp` that implements the complete handling of all regular WebGPU methods in the client

Most of the WebGPU methods can be handled automatically by the wire client/server but some of them need custom handling (for example because they handle callbacks or need client-side state tracking). `dawn_wire.json` defines which methods need special handling, and extra wire commands that can be used by that special handling (and will get `WireCmd` support).

The schema of `dawn_wire.json` is a dictionary with the following keys:
 - `"commands"` an array of **records** defining extra client->server commands that can be used in special-cased code path.
   - Each **record member** can have an extra `"skip_serialize"` key that's a boolean that default to false and makes `WireCmd` skip it on its on-wire format.
 - `"return commands"` like `"commands"` but in revers, an array of **records** defining extra server->client commands
 - `"special items"` a dictionary containing various lists of methods or object that require special handling in places in the dawn_wire autogenerated files
   - `"client_side_structures"`: a list of structure that we shouldn't generate serialization/deserialization code for because they are client-side only
   - `"client_handwritten_commands"`: a list of methods that are written manually and won't be automatically generated in the client
   - `"client_side_commands"`: a list of methods that won't be automatically generated in the server. Gets added to `"client_handwritten_commands"`
   - `"client_special_objects"`: a list of objects that need special manual state-tracking in the client and won't be autogenerated
   - `"server_custom_pre_handler_commands"`: a list of methods that will run custom "pre-handlers" before calling the autogenerated handlers in the server
   - `"server_handwrittten_commands"`: a list of methods that are written manually and won't be automatically generated in the server.
   - `server_reverse_object_lookup_objects`: a list of objects for which the server will maintain an object -> ID mapping.

## OpenGL loader generator

The code to load OpenGL entrypoints from a `GetProcAddress` function is generated from [`gl.xml`](../../third_party/khronos/gl.xml) and the [list of extensions](../../src/dawn/native/opengl/supported_extensions.json) it supports.
