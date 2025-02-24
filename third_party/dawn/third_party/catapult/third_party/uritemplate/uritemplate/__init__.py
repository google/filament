#!/usr/bin/env python

"""
URI Template (RFC6570) Processor
"""

__copyright__ = """\
Copyright 2011-2013 Joe Gregorio

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import re
try:
   from urllib.parse import quote
except ImportError:
   from urllib import quote



__version__ = "0.6"

RESERVED = ":/?#[]@!$&'()*+,;="
OPERATOR = "+#./;?&|!@"
MODIFIER = ":^"
TEMPLATE = re.compile("{([^\}]+)}")


def variables(template):
    '''Returns the set of keywords in a uri template'''
    vars = set()
    for varlist in TEMPLATE.findall(template):
        if varlist[0] in OPERATOR:
            varlist = varlist[1:]
        varspecs = varlist.split(',')
        for var in varspecs:
            # handle prefix values
            var = var.split(':')[0]
            # handle composite values
            if var.endswith('*'):
                var = var[:-1]
            vars.add(var)
    return vars


def _quote(value, safe, prefix=None):
    if prefix is not None:
        return quote(str(value)[:prefix], safe)
    return quote(str(value), safe)


def _tostring(varname, value, explode, prefix, operator, safe=""):
    if isinstance(value, list):
        return ",".join([_quote(x, safe) for x in value])
    if isinstance(value, dict):
        keys = sorted(value.keys())
        if explode:
            return ",".join([_quote(key, safe) + "=" + \
                             _quote(value[key], safe) for key in keys])
        else:
            return ",".join([_quote(key, safe) + "," + \
                             _quote(value[key], safe) for key in keys])
    elif value is None:
        return
    else:
        return _quote(value, safe, prefix)


def _tostring_path(varname, value, explode, prefix, operator, safe=""):
    joiner = operator
    if isinstance(value, list):
        if explode:
            out = [_quote(x, safe) for x in value if value is not None]
        else:
            joiner = ","
            out = [_quote(x, safe) for x in value if value is not None]
        if out:
            return joiner.join(out)
        else:
            return
    elif isinstance(value, dict):
        keys = sorted(value.keys())
        if explode:
            out = [_quote(key, safe) + "=" + \
                   _quote(value[key], safe) for key in keys \
                   if value[key] is not None]
        else:
            joiner = ","
            out = [_quote(key, safe) + "," + \
                   _quote(value[key], safe) \
                   for key in keys if value[key] is not None]
        if out:
            return joiner.join(out)
        else:
            return
    elif value is None:
        return
    else:
        return _quote(value, safe, prefix)


def _tostring_semi(varname, value, explode, prefix, operator, safe=""):
    joiner = operator
    if operator == "?":
        joiner = "&"
    if isinstance(value, list):
        if explode:
            out = [varname + "=" + _quote(x, safe) \
                   for x in value if x is not None]
            if out:
                return joiner.join(out)
            else:
                return
        else:
            return varname + "=" + ",".join([_quote(x, safe) \
                                             for x in value])
    elif isinstance(value, dict):
        keys = sorted(value.keys())
        if explode:
            return joiner.join([_quote(key, safe) + "=" + \
                                _quote(value[key], safe) \
                                for key in keys if key is not None])
        else:
            return varname + "=" + ",".join([_quote(key, safe) + "," + \
                             _quote(value[key], safe) for key in keys \
                             if key is not None])
    else:
        if value is None:
            return
        elif value:
            return (varname + "=" + _quote(value, safe, prefix))
        else:
            return varname


def _tostring_query(varname, value, explode, prefix, operator, safe=""):
    joiner = operator
    if operator in ["?", "&"]:
        joiner = "&"
    if isinstance(value, list):
        if 0 == len(value):
            return None
        if explode:
            return joiner.join([varname + "=" + _quote(x, safe) \
                                for x in value])
        else:
            return (varname + "=" + ",".join([_quote(x, safe) \
                                             for x in value]))
    elif isinstance(value, dict):
        if 0 == len(value):
            return None
        keys = sorted(value.keys())
        if explode:
            return joiner.join([_quote(key, safe) + "=" + \
                                _quote(value[key], safe) \
                                for key in keys])
        else:
            return varname + "=" + \
                   ",".join([_quote(key, safe) + "," + \
                             _quote(value[key], safe) for key in keys])
    else:
        if value is None:
            return
        elif value:
            return (varname + "=" + _quote(value, safe, prefix))
        else:
            return (varname  + "=")


TOSTRING = {
    "" : _tostring,
    "+": _tostring,
    "#": _tostring,
    ";": _tostring_semi,
    "?": _tostring_query,
    "&": _tostring_query,
    "/": _tostring_path,
    ".": _tostring_path,
    }


def expand(template, variables):
    """
    Expand template as a URI Template using variables.
    """
    def _sub(match):
        expression = match.group(1)
        operator = ""
        if expression[0] in OPERATOR:
            operator = expression[0]
            varlist = expression[1:]
        else:
            varlist = expression

        safe = ""
        if operator in ["+", "#"]:
            safe = RESERVED
        varspecs = varlist.split(",")
        varnames = []
        defaults = {}
        for varspec in varspecs:
            default = None
            explode = False
            prefix = None
            if "=" in varspec:
                varname, default = tuple(varspec.split("=", 1))
            else:
                varname = varspec
            if varname[-1] == "*":
                explode = True
                varname = varname[:-1]
            elif ":" in varname:
                try:
                    prefix = int(varname[varname.index(":")+1:])
                except ValueError:
                    raise ValueError("non-integer prefix '{0}'".format(
                       varname[varname.index(":")+1:]))
                varname = varname[:varname.index(":")]
            if default:
                defaults[varname] = default
            varnames.append((varname, explode, prefix))

        retval = []
        joiner = operator
        start = operator
        if operator == "+":
            start = ""
            joiner = ","
        if operator == "#":
            joiner = ","
        if operator == "?":
            joiner = "&"
        if operator == "&":
            start = "&"
        if operator == "":
            joiner = ","
        for varname, explode, prefix in varnames:
            if varname in variables:
                value = variables[varname]
                if not value and value != "" and varname in defaults:
                    value = defaults[varname]
            elif varname in defaults:
                value = defaults[varname]
            else:
                continue
            expanded = TOSTRING[operator](
              varname, value, explode, prefix, operator, safe=safe)
            if expanded is not None:
                retval.append(expanded)
        if len(retval) > 0:
            return start + joiner.join(retval)
        else:
            return ""

    return TEMPLATE.sub(_sub, template)
