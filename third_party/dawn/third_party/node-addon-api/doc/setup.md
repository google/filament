# Setup

## Prerequisites

Before starting to use **Node-API** you need to assure you have the following
prerequisites:

* **Node.JS** see: [Installing Node.js](https://nodejs.org/)

* **Node.js native addon build tool**

  - **[node-gyp](node-gyp.md)**

## Installation and usage

To use **Node-API** in a native module:

  1. Add a dependency on this package to `package.json`:

     ```json
       "dependencies": {
         "node-addon-api": "*",
       }
     ```

  2. Decide whether the package will enable C++ exceptions in the Node-API
     wrapper, and reference this package as a dependency in `binding.gyp`.
     The base ABI-stable C APIs do not throw or handle C++ exceptions, but the
     Node-API C++ wrapper classes may _optionally_
     [integrate C++ and JavaScript exception-handling
     ](https://github.com/nodejs/node-addon-api/blob/HEAD/doc/error_handling.md).

     To use without C++ exceptions, add the following to `binding.gyp`:

     ```gyp
       'dependencies': [
         "<!(node -p \"require('node-addon-api').targets\"):node_addon_api",
       ],
     ```

     To enable that capability, add an alternative dependency in `binding.gyp`:

     ```gyp
       'dependencies': [
         "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except",
       ],
     ```

     If you decide to use node-addon-api without C++ exceptions enabled, please
     consider enabling node-addon-api safe API type guards to ensure the proper
     exception handling pattern:

     ```gyp
       'dependencies': [
         "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_maybe",
       ],
     ```

  3. If you would like your native addon to support OSX, please also add the
     following settings in the `binding.gyp` file:

       ```gyp
       'conditions': [
         ['OS=="mac"', {
             'cflags+': ['-fvisibility=hidden'],
             'xcode_settings': {
               'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
             }
         }]
       ]
       ```

  4. Include `napi.h` in the native module code.
     To ensure only ABI-stable APIs are used, DO NOT include
     `node.h`, `nan.h`, or `v8.h`.

     ```C++
     #include "napi.h"
     ```

At build time, the Node-API back-compat library code will be used only when the
targeted node version *does not* have Node-API built-in.

The preprocessor directive `NODE_ADDON_API_DISABLE_DEPRECATED` can be defined at
compile time before including `napi.h` to skip the definition of deprecated APIs.

By default, throwing an exception on a terminating environment (eg. worker
threads) will cause a fatal exception, terminating the Node process. This is to
provide feedback to the user of the runtime error, as it is impossible to pass
the error to JavaScript when the environment is terminating. In order to bypass
this behavior such that the Node process will not terminate, define the
preprocessor directive `NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS`.
