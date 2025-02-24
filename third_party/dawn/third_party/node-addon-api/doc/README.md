# node-addon-api Documents

* [Setup](#setup)
* [API Documentation](#api)
* [Examples](#examples)
* [ABI Stability Guideline](#abi-stability-guideline)
* [More resource and info about native Addons](#resources)

Node-API is an ABI stable C interface provided by Node.js for building native
addons. It is independent of the underlying JavaScript runtime (e.g. V8 or ChakraCore)
and is maintained as part of Node.js itself. It is intended to insulate
native addons from changes in the underlying JavaScript engine and allow
modules compiled for one version to run on later versions of Node.js without
recompilation.

The `node-addon-api` module, which is not part of Node.js, preserves the benefits
of the Node-API as it consists only of inline code that depends only on the stable API
provided by Node-API. As such, modules built against one version of Node.js
using node-addon-api should run without having to be rebuilt with newer versions
of Node.js.

## Setup
  - [Installation and usage](setup.md)
  - [node-gyp](node-gyp.md)
  - [cmake-js](cmake-js.md)
  - [Conversion tool](conversion-tool.md)
  - [Checker tool](checker-tool.md)
  - [Generator](generator.md)
  - [Prebuild tools](prebuild_tools.md)

<a name="api"></a>

## API Documentation

The following is the documentation for node-addon-api.

 - [Full Class Hierarchy](hierarchy.md)
 - [Addon Structure](addon.md)
 - Data Types:
    - [Env](env.md)
    - [CallbackInfo](callbackinfo.md)
    - [Reference](reference.md)
    - [Value](value.md)
        - [Name](name.md)
            - [Symbol](symbol.md)
            - [String](string.md)
        - [Number](number.md)
        - [Date](date.md)
        - [BigInt](bigint.md)
        - [Boolean](boolean.md)
        - [External](external.md)
        - [Object](object.md)
            - [Array](array.md)
            - [ObjectReference](object_reference.md)
    - [PropertyDescriptor](property_descriptor.md)
    - [Function](function.md)
        - [FunctionReference](function_reference.md)
    - [ObjectWrap](object_wrap.md)
        - [ClassPropertyDescriptor](class_property_descriptor.md)
    - [Buffer](buffer.md)
    - [ArrayBuffer](array_buffer.md)
    - [TypedArray](typed_array.md)
      - [TypedArrayOf](typed_array_of.md)
    - [DataView](dataview.md)
 - [Error Handling](error_handling.md)
    - [Error](error.md)
      - [TypeError](type_error.md)
      - [RangeError](range_error.md)
      - [SyntaxError](syntax_error.md)
 - [Object Lifetime Management](object_lifetime_management.md)
    - [HandleScope](handle_scope.md)
    - [EscapableHandleScope](escapable_handle_scope.md)
 - [Memory Management](memory_management.md)
 - [Async Operations](async_operations.md)
    - [AsyncWorker](async_worker.md)
    - [AsyncContext](async_context.md)
    - [AsyncWorker Variants](async_worker_variants.md)
 - [Thread-safe Functions](threadsafe.md)
    - [ThreadSafeFunction](threadsafe_function.md)
    - [TypedThreadSafeFunction](typed_threadsafe_function.md)
 - [Promises](promises.md)
 - [Version management](version_management.md)

<a name="examples"></a>

## Examples

Are you new to **node-addon-api**? Take a look at our **[examples](https://github.com/nodejs/node-addon-examples)**

- [Hello World](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/1_hello_world)
- [Pass arguments to a function](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/2_function_arguments/node-addon-api)
- [Callbacks](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/3_callbacks/node-addon-api)
- [Object factory](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/4_object_factory/node-addon-api)
- [Function factory](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/5_function_factory/node-addon-api)
- [Wrapping C++ Object](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/6_object_wrap/node-addon-api)
- [Factory of wrapped object](https://github.com/nodejs/node-addon-examples/tree/main/src/1-getting-started/7_factory_wrap/node-addon-api)
- [Passing wrapped object around](https://github.com/nodejs/node-addon-examples/tree/main/src/2-js-to-native-conversion/8_passing_wrapped/node-addon-api)

<a name="abi-stability-guideline"></a>

## ABI Stability Guideline

It is important to remember that *other* Node.js interfaces such as
`libuv` (included in a project via `#include <uv.h>`) are not ABI-stable across
Node.js major versions. Thus, an addon must use Node-API and/or `node-addon-api`
exclusively and build against a version of Node.js that includes an
implementation of Node-API (meaning an active LTS version of Node.js) in
order to benefit from ABI stability across Node.js major versions. Node.js
provides an [ABI stability guide][] containing a detailed explanation of ABI
stability in general, and the Node-API ABI stability guarantee in particular.

<a name="resources"></a>

## More resource and info about native Addons

There are three options for implementing addons: Node-API, nan, or direct
use of internal V8, libuv, and Node.js libraries. Unless there is a need for
direct access to functionality that is not exposed by Node-API as outlined
in [C/C++ addons](https://nodejs.org/dist/latest/docs/api/addons.html)
in Node.js core, use Node-API. Refer to
[C/C++ addons with Node-API](https://nodejs.org/dist/latest/docs/api/n-api.html)
for more information on Node-API.

- [C++ Addons](https://nodejs.org/dist/latest/docs/api/addons.html)
- [Node-API](https://nodejs.org/dist/latest/docs/api/n-api.html)
- [Node-API - Next Generation Node API for Native Modules](https://youtu.be/-Oniup60Afs)
- [How We Migrated Realm JavaScript From NAN to Node-API](https://developer.mongodb.com/article/realm-javascript-nan-to-n-api)

As node-addon-api's core mission is to expose the plain C Node-API as C++
wrappers, tools that facilitate n-api/node-addon-api providing more
convenient patterns for developing a Node.js add-on with n-api/node-addon-api
can be published to NPM as standalone packages. It is also recommended to tag
such packages with `node-addon-api` to provide more visibility to the community.

Quick links to NPM searches: [keywords:node-addon-api](https://www.npmjs.com/search?q=keywords%3Anode-addon-api).

<a name="other-bindings"></a>

## Other bindings

- [napi-rs](https://napi.rs) - (`Rust`)

[ABI stability guide]: https://nodejs.org/en/docs/guides/abi-stability/
