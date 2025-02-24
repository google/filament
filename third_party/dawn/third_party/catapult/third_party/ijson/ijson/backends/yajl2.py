'''
Wrapper for YAJL C library version 2.x.
'''

from ctypes import Structure, c_uint, c_ubyte, c_int, c_long, c_double, c_char, \
                   c_void_p, c_char_p, CFUNCTYPE, POINTER, byref, string_at, cast

from ijson import common, backends
from ijson.compat import b2s


yajl = backends.find_yajl_ctypes(2)

yajl.yajl_alloc.restype = POINTER(c_char)
yajl.yajl_get_error.restype = POINTER(c_char)

C_EMPTY = CFUNCTYPE(c_int, c_void_p)
C_INT = CFUNCTYPE(c_int, c_void_p, c_int)
C_LONG = CFUNCTYPE(c_int, c_void_p, c_long)
C_DOUBLE = CFUNCTYPE(c_int, c_void_p, c_double)
C_STR = CFUNCTYPE(c_int, c_void_p, POINTER(c_ubyte), c_uint)


_callback_data = [
    # Mapping of JSON parser events to callback C types and value converters.
    # Used to define the Callbacks structure and actual callback functions
    # inside the parse function.
    ('null', C_EMPTY, lambda: None),
    ('boolean', C_INT, lambda v: bool(v)),
    # "integer" and "double" aren't actually yielded by yajl since "number"
    # takes precedence if defined
    ('integer', C_LONG, lambda v, l: int(string_at(v, l))),
    ('double', C_DOUBLE, lambda v, l: float(string_at(v, l))),
    ('number', C_STR, lambda v, l: common.number(b2s(string_at(v, l)))),
    ('string', C_STR, lambda v, l: string_at(v, l).decode('utf-8')),
    ('start_map', C_EMPTY, lambda: None),
    ('map_key', C_STR, lambda v, l: b2s(string_at(v, l))),
    ('end_map', C_EMPTY, lambda: None),
    ('start_array', C_EMPTY, lambda: None),
    ('end_array', C_EMPTY, lambda: None),
]

class Callbacks(Structure):
    _fields_ = [(name, type) for name, type, func in _callback_data]

YAJL_OK = 0
YAJL_CANCELLED = 1
YAJL_INSUFFICIENT_DATA = 2
YAJL_ERROR = 3

# constants defined in yajl_parse.h
YAJL_ALLOW_COMMENTS = 1
YAJL_MULTIPLE_VALUES = 8


def basic_parse(f, allow_comments=False, buf_size=64 * 1024,
                multiple_values=False):
    '''
    Iterator yielding unprefixed events.

    Parameters:

    - f: a readable file-like object with JSON input
    - allow_comments: tells parser to allow comments in JSON input
    - buf_size: a size of an input buffer
    - multiple_values: allows the parser to parse multiple JSON objects
    '''
    events = []

    def callback(event, func_type, func):
        def c_callback(context, *args):
            events.append((event, func(*args)))
            return 1
        return func_type(c_callback)

    callbacks = Callbacks(*[callback(*data) for data in _callback_data])
    handle = yajl.yajl_alloc(byref(callbacks), None, None)
    if allow_comments:
        yajl.yajl_config(handle, YAJL_ALLOW_COMMENTS, 1)
    if multiple_values:
        yajl.yajl_config(handle, YAJL_MULTIPLE_VALUES, 1)
    try:
        while True:
            buffer = f.read(buf_size)
            if buffer:
                result = yajl.yajl_parse(handle, buffer, len(buffer))
            else:
                result = yajl.yajl_complete_parse(handle)
            if result != YAJL_OK:
                perror = yajl.yajl_get_error(handle, 1, buffer, len(buffer))
                error = cast(perror, c_char_p).value
                yajl.yajl_free_error(handle, perror)
                exception = common.IncompleteJSONError if result == YAJL_INSUFFICIENT_DATA else common.JSONError
                raise exception(error.decode('utf-8'))
            if not buffer and not events:
                break

            for event in events:
                yield event
            events = []
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
