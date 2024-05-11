#ifdef __wasi__
#include <stdlib.h>
#include <string.h>

#include <wasi/api.h>

extern "C" void __cxa_throw(void* ptr, void* type, void* destructor)
{
	abort();
}

extern "C" void* __cxa_allocate_exception(size_t thrown_size)
{
	abort();
}

extern "C" int32_t __wasi_path_open32(int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, int32_t arg6, int32_t arg7, int32_t arg8)
    __attribute__((
        __import_module__("wasi_snapshot_preview1"),
        __import_name__("path_open32"),
        __warn_unused_result__));

extern "C" int32_t __imported_wasi_snapshot_preview1_path_open(int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int64_t arg5, int64_t arg6, int32_t arg7, int32_t arg8)
{
	return __wasi_path_open32(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

extern "C" int32_t __wasi_fd_seek32(int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3)
    __attribute__((
        __import_module__("wasi_snapshot_preview1"),
        __import_name__("fd_seek32"),
        __warn_unused_result__));

extern "C" int32_t __imported_wasi_snapshot_preview1_fd_seek(int32_t arg0, int64_t arg1, int32_t arg2, int32_t arg3)
{
	*(uint64_t*)arg3 = 0;
	return __wasi_fd_seek32(arg0, arg1, arg2, arg3);
}

extern "C" int32_t __imported_wasi_snapshot_preview1_clock_time_get(int32_t arg0, int64_t arg1, int32_t arg2)
{
	return __WASI_ERRNO_NOSYS;
}

#endif
