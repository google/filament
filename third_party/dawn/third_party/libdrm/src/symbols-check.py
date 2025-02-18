#!/usr/bin/env python3

import argparse
import os
import platform
import subprocess

# This list contains symbols that _might_ be exported for some platforms
PLATFORM_SYMBOLS = [
    '_GLOBAL_OFFSET_TABLE_',
    '__bss_end__',
    '__bss_start__',
    '__bss_start',
    '__end__',
    '_bss_end__',
    '_edata',
    '_end',
    '_fini',
    '_init',
    '_fbss',
    '_fdata',
    '_ftext',
]


def get_symbols(nm, lib):
    '''
    List all the (non platform-specific) symbols exported by the library
    '''
    symbols = []
    platform_name = platform.system()
    output = subprocess.check_output([nm, '-gP', lib],
                                     stderr=open(os.devnull, 'w')).decode("ascii")
    for line in output.splitlines():
        fields = line.split()
        if len(fields) == 2 or fields[1] == 'U':
            continue
        symbol_name = fields[0]
        if platform_name == 'Linux':
            if symbol_name in PLATFORM_SYMBOLS:
                continue
        elif platform_name == 'Darwin':
            assert symbol_name[0] == '_'
            symbol_name = symbol_name[1:]
        symbols.append(symbol_name)

    return symbols


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--symbols-file',
                        action='store',
                        required=True,
                        help='path to file containing symbols')
    parser.add_argument('--lib',
                        action='store',
                        required=True,
                        help='path to library')
    parser.add_argument('--nm',
                        action='store',
                        required=True,
                        help='path to binary (or name in $PATH)')
    args = parser.parse_args()

    try:
        lib_symbols = get_symbols(args.nm, args.lib)
    except:
        # We can't run this test, but we haven't technically failed it either
        # Return the GNU "skip" error code
        exit(77)
    mandatory_symbols = []
    optional_symbols = []
    with open(args.symbols_file) as symbols_file:
        qualifier_optional = '(optional)'
        for line in symbols_file.readlines():

            # Strip comments
            line = line.split('#')[0]
            line = line.strip()
            if not line:
                continue

            # Line format:
            # [qualifier] symbol
            qualifier = None
            symbol = None

            fields = line.split()
            if len(fields) == 1:
                symbol = fields[0]
            elif len(fields) == 2:
                qualifier = fields[0]
                symbol = fields[1]
            else:
                print(args.symbols_file + ': invalid format: ' + line)
                exit(1)

            # The only supported qualifier is 'optional', which means the
            # symbol doesn't have to be exported by the library
            if qualifier and not qualifier == qualifier_optional:
                print(args.symbols_file + ': invalid qualifier: ' + qualifier)
                exit(1)

            if qualifier == qualifier_optional:
                optional_symbols.append(symbol)
            else:
                mandatory_symbols.append(symbol)

    unknown_symbols = []
    for symbol in lib_symbols:
        if symbol in mandatory_symbols:
            continue
        if symbol in optional_symbols:
            continue
        unknown_symbols.append(symbol)

    missing_symbols = [
        sym for sym in mandatory_symbols if sym not in lib_symbols
    ]

    for symbol in unknown_symbols:
        print(args.lib + ': unknown symbol exported: ' + symbol)

    for symbol in missing_symbols:
        print(args.lib + ': missing symbol: ' + symbol)

    if unknown_symbols or missing_symbols:
        exit(1)
    exit(0)


if __name__ == '__main__':
    main()
