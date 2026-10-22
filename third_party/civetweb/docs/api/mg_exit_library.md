# Civetweb API Reference

### `mg_exit_library( );`

### Parameters

none

### Return Value

| Type | Description |
| :--- | :--- |
|`unsigned`| **0** is returned or error |

### Description

The function `mg_exit_library()` should be called from an application program, when the library should be unloaded.
It must be called only from one thread (it is not guaranteed to be thread safe).

Only use `mg_exit_library( );` when you used [`mg_init_library( feature );`](api/mg_init_library.md) before.

The library init and exit functions are new in version 1.9 (as dummy implementation) and effective only from version 1.10.
For compatibility reasons, other functions (such as [`mg_start();`](mg_start.md)) will initialize the required features as well,
but they will no longer do a de-initialization, leaving a memory leak when the library is unloaded.

### See Also

* [`mg_init_library( feature );`](mg_init_library.md)
* [`mg_check_feature( feature );`](mg_check_feature.md)
