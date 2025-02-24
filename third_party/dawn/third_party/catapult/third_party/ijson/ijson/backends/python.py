'''
Pure-python parsing backend.
'''
from __future__ import unicode_literals
import decimal
import re
from codecs import getreader
from json.decoder import scanstring

from ijson import common
from ijson.compat import bytetype


BUFSIZE = 16 * 1024
LEXEME_RE = re.compile(r'[a-z0-9eE\.\+-]+|\S')


class UnexpectedSymbol(common.JSONError):
    def __init__(self, symbol, pos):
        super(UnexpectedSymbol, self).__init__(
            'Unexpected symbol %r at %d' % (symbol, pos)
        )


def Lexer(f, buf_size=BUFSIZE):
    if type(f.read(0)) == bytetype:
        f = getreader('utf-8')(f)
    buf = f.read(buf_size)
    pos = 0
    discarded = 0
    while True:
        match = LEXEME_RE.search(buf, pos)
        if match:
            lexeme = match.group()
            if lexeme == '"':
                pos = match.start()
                start = pos + 1
                while True:
                    try:
                        end = buf.index('"', start)
                        escpos = end - 1
                        while buf[escpos] == '\\':
                            escpos -= 1
                        if (end - escpos) % 2 == 0:
                            start = end + 1
                        else:
                            break
                    except ValueError:
                        data = f.read(buf_size)
                        if not data:
                            raise common.IncompleteJSONError('Incomplete string lexeme')
                        buf += data
                yield discarded + pos, buf[pos:end + 1]
                pos = end + 1
            else:
                while match.end() == len(buf):
                    data = f.read(buf_size)
                    if not data:
                        break
                    buf += data
                    match = LEXEME_RE.search(buf, pos)
                    lexeme = match.group()
                yield discarded + match.start(), lexeme
                pos = match.end()
        else:
            data = f.read(buf_size)
            if not data:
                break
            discarded += len(buf)
            buf = data
            pos = 0


def parse_value(lexer, symbol=None, pos=0):
    try:
        if symbol is None:
            pos, symbol = next(lexer)
        if symbol == 'null':
            yield ('null', None)
        elif symbol == 'true':
            yield ('boolean', True)
        elif symbol == 'false':
            yield ('boolean', False)
        elif symbol == '[':
            for event in parse_array(lexer):
                yield event
        elif symbol == '{':
            for event in parse_object(lexer):
                yield event
        elif symbol[0] == '"':
            yield ('string', parse_string(symbol))
        else:
            try:
                yield ('number', common.number(symbol))
            except decimal.InvalidOperation:
                raise UnexpectedSymbol(symbol, pos)
    except StopIteration:
        raise common.IncompleteJSONError('Incomplete JSON data')


def parse_string(symbol):
    return scanstring(symbol, 1)[0]


def parse_array(lexer):
    yield ('start_array', None)
    try:
        pos, symbol = next(lexer)
        if symbol != ']':
            while True:
                for event in parse_value(lexer, symbol, pos):
                    yield event
                pos, symbol = next(lexer)
                if symbol == ']':
                    break
                if symbol != ',':
                    raise UnexpectedSymbol(symbol, pos)
                pos, symbol = next(lexer)
        yield ('end_array', None)
    except StopIteration:
        raise common.IncompleteJSONError('Incomplete JSON data')


def parse_object(lexer):
    yield ('start_map', None)
    try:
        pos, symbol = next(lexer)
        if symbol != '}':
            while True:
                if symbol[0] != '"':
                    raise UnexpectedSymbol(symbol, pos)
                yield ('map_key', parse_string(symbol))
                pos, symbol = next(lexer)
                if symbol != ':':
                    raise UnexpectedSymbol(symbol, pos)
                for event in parse_value(lexer, None, pos):
                    yield event
                pos, symbol = next(lexer)
                if symbol == '}':
                    break
                if symbol != ',':
                    raise UnexpectedSymbol(symbol, pos)
                pos, symbol = next(lexer)
        yield ('end_map', None)
    except StopIteration:
        raise common.IncompleteJSONError('Incomplete JSON data')


def basic_parse(file, buf_size=BUFSIZE):
    '''
    Iterator yielding unprefixed events.

    Parameters:

    - file: a readable file-like object with JSON input
    '''
    lexer = iter(Lexer(file, buf_size))
    for value in parse_value(lexer):
        yield value
    try:
        next(lexer)
    except StopIteration:
        pass
    else:
        raise common.JSONError('Additional data')


def parse(file, buf_size=BUFSIZE):
    '''
    Backend-specific wrapper for ijson.common.parse.
    '''
    return common.parse(basic_parse(file, buf_size=buf_size))


def items(file, prefix):
    '''
    Backend-specific wrapper for ijson.common.items.
    '''
    return common.items(parse(file), prefix)
