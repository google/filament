'''
CFFI-Wrapper for YAJL C library version 2.x.
'''

from cffi import FFI
import functools
import sys

from ijson import common, backends
from ijson.compat import b2s


ffi = FFI()
ffi.cdef("""
typedef void * (*yajl_malloc_func)(void *ctx, size_t sz);
typedef void (*yajl_free_func)(void *ctx, void * ptr);
typedef void * (*yajl_realloc_func)(void *ctx, void * ptr, size_t sz);
typedef struct
{
    yajl_malloc_func malloc;
    yajl_realloc_func realloc;
    yajl_free_func free;
    void * ctx;
} yajl_alloc_funcs;
typedef struct yajl_handle_t * yajl_handle;
typedef enum {
    yajl_status_ok,
    yajl_status_client_canceled,
    yajl_status_error
} yajl_status;
typedef enum {
    yajl_allow_comments = 0x01,
    yajl_dont_validate_strings     = 0x02,
    yajl_allow_trailing_garbage = 0x04,
    yajl_allow_multiple_values = 0x08,
    yajl_allow_partial_values = 0x10
} yajl_option;
typedef struct {
    int (* yajl_null)(void * ctx);
    int (* yajl_boolean)(void * ctx, int boolVal);
    int (* yajl_integer)(void * ctx, long long integerVal);
    int (* yajl_double)(void * ctx, double doubleVal);
    int (* yajl_number)(void * ctx, const char * numberVal,
                        size_t numberLen);
    int (* yajl_string)(void * ctx, const unsigned char * stringVal,
                        size_t stringLen);
    int (* yajl_start_map)(void * ctx);
    int (* yajl_map_key)(void * ctx, const unsigned char * key,
                         size_t stringLen);
    int (* yajl_end_map)(void * ctx);
    int (* yajl_start_array)(void * ctx);
    int (* yajl_end_array)(void * ctx);
} yajl_callbacks;
int yajl_version(void);
yajl_handle yajl_alloc(const yajl_callbacks *callbacks, yajl_alloc_funcs *afs, void *ctx);
int yajl_config(yajl_handle h, yajl_option opt, ...);
yajl_status yajl_parse(yajl_handle hand, const unsigned char *jsonText, size_t jsonTextLength);
yajl_status yajl_complete_parse(yajl_handle hand);
unsigned char* yajl_get_error(yajl_handle hand, int verbose, const unsigned char *jsonText, size_t jsonTextLength);
void yajl_free_error(yajl_handle hand, unsigned char * str);
void yajl_free(yajl_handle handle);
""")


yajl = backends.find_yajl_cffi(ffi, 2)

YAJL_OK = 0
YAJL_CANCELLED = 1
YAJL_INSUFFICIENT_DATA = 2
YAJL_ERROR = 3

# constants defined in yajl_parse.h
YAJL_ALLOW_COMMENTS = 1
YAJL_MULTIPLE_VALUES = 8


def append_event_to_ctx(event):
    def wrapper(func):
        @functools.wraps(func)
        def wrapped(ctx, *args, **kwargs):
            value = func(*args, **kwargs)
            ctx = ffi.from_handle(ctx)
            ctx.append((event, value))
            return 1
        return wrapped
    return wrapper


@ffi.callback('int(void *ctx)')
@append_event_to_ctx('null')
def null():
    return None


@ffi.callback('int(void *ctx, int val)')
@append_event_to_ctx('boolean')
def boolean(val):
    return bool(val)


@ffi.callback('int(void *ctx, long long integerVal)')
@append_event_to_ctx('integer')
def integer(val):
    return int(val)


@ffi.callback('int(void *ctx, double doubleVal)')
@append_event_to_ctx('double')
def double(val):
    return float(val)


@ffi.callback('int(void *ctx, const char *numberVal, size_t numberLen)')
@append_event_to_ctx('number')
def number(val, length):
    return common.number(b2s(ffi.string(val, maxlen=length)))


@ffi.callback('int(void *ctx, const unsigned char *stringVal, size_t stringLen)')
@append_event_to_ctx('string')
def string(val, length):
    return ffi.string(val, maxlen=length).decode('utf-8')


@ffi.callback('int(void *ctx)')
@append_event_to_ctx('start_map')
def start_map():
    return None


@ffi.callback('int(void *ctx, const unsigned char *key, size_t stringLen)')
@append_event_to_ctx('map_key')
def map_key(key, length):
    return b2s(ffi.string(key, maxlen=length))


@ffi.callback('int(void *ctx)')
@append_event_to_ctx('end_map')
def end_map():
    return None


@ffi.callback('int(void *ctx)')
@append_event_to_ctx('start_array')
def start_array():
    return None


@ffi.callback('int(void *ctx)')
@append_event_to_ctx('end_array')
def end_array():
    return None


_callback_data = (
    # For more information about callbacks,
    # take a look at the ctypes backend
    null, boolean, integer, double, number, string,
    start_map, map_key, end_map, start_array, end_array
)


_asd = list()
def yajl_init(scope, events, allow_comments=False, multiple_values=False):
    scope.ctx = ffi.new_handle(events)
    scope.callbacks = ffi.new('yajl_callbacks*', _callback_data)
    handle = yajl.yajl_alloc(scope.callbacks, ffi.NULL, scope.ctx)

    if allow_comments:
        yajl.yajl_config(handle, YAJL_ALLOW_COMMENTS, ffi.cast('int', 1))
    if multiple_values:
        yajl.yajl_config(handle, YAJL_MULTIPLE_VALUES, ffi.cast('int', 1))

    return handle


def yajl_parse(handle, buffer):
    if buffer:
        result = yajl.yajl_parse(handle, buffer, len(buffer))
    else:
        result = yajl.yajl_complete_parse(handle)

    if result != YAJL_OK:
        perror = yajl.yajl_get_error(handle, 1, buffer, len(buffer))
        error = b2s(ffi.string(perror))
        yajl.yajl_free_error(handle, perror)
        exception = common.IncompleteJSONError if result == YAJL_INSUFFICIENT_DATA else common.JSONError
        raise exception(error)


class Container(object):
    pass


def basic_parse(f, buf_size=64*1024, **config):
    '''
    Iterator yielding unprefixed events.

    Parameters:

    - f: a readable file-like object with JSON input
    - allow_comments: tells parser to allow comments in JSON input
    - buf_size: a size of an input buffer
    - multiple_values: allows the parser to parse multiple JSON objects
    '''

    # the scope objects makes sure the C objects allocated in _yajl.init
    # are kept alive until this function is done
    scope = Container()
    events = []

    handle = yajl_init(scope, events, **config)
    try:
        while True:
            buffer = f.read(buf_size)
            # this calls the callbacks which will
            # fill the events list
            yajl_parse(handle, buffer)

            if not buffer and not events:
                break

            for event in events:
                yield event

            # clear all events, but don't replace the
            # the events list instance
            del events[:]
    finally:
        yajl.yajl_free(handle)


def parse(file, **kwargs):
    '''
    Backend-specific wrapper for ijson.common.parse.
    '''
    return common.parse(basic_parse(file, **kwargs))

def items(file, prefix):
    '''
    Backend-specific wrapper for ijson.common.items.
    '''
    return common.items(parse(file), prefix)
