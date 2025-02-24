                                                            -*- Autoconf -*-

# Common code for C-like languages (C, C++, Java, etc.)

# Copyright (C) 2012-2015, 2018-2021 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


# _b4_comment(TEXT, OPEN, CONTINUE, END)
# --------------------------------------
# Put TEXT in comment.  Avoid trailing spaces: don't indent empty lines.
# Avoid adding indentation to the first line, as the indentation comes
# from OPEN.  That's why we don't patsubst([$1], [^\(.\)], [   \1]).
# Turn "*/" in TEXT into "* /" so that we don't unexpectedly close
# the comments before its end.
#
# Prefix all the output lines with PREFIX.
m4_define([_b4_comment],
[$2[]b4_gsub(m4_expand([$1]),
            [[*]/], [*\\/],
            [/[*]], [/\\*],
            [
\(.\)], [
$3\1])$4])


# b4_comment(TEXT, [PREFIX])
# --------------------------
# Put TEXT in comment.  Prefix all the output lines with PREFIX.
m4_define([b4_comment],
[_b4_comment([$1], [$2/* ], [$2   ], [  */])])




# _b4_dollar_dollar(VALUE, SYMBOL-NUM, FIELD, DEFAULT-FIELD)
# ----------------------------------------------------------
# If FIELD (or DEFAULT-FIELD) is non-null, return "VALUE.FIELD",
# otherwise just VALUE.  Be sure to pass "(VALUE)" if VALUE is a
# pointer.
m4_define([_b4_dollar_dollar],
[b4_symbol_value([$1],
                 [$2],
                 m4_if([$3], [[]],
                       [[$4]], [[$3]]))])

# b4_dollar_pushdef(VALUE-POINTER, SYMBOL-NUM, [TYPE_TAG], LOCATION)
# b4_dollar_popdef
# ------------------------------------------------------------------
# Define b4_dollar_dollar for VALUE-POINTER and DEFAULT-FIELD,
# and b4_at_dollar for LOCATION.
m4_define([b4_dollar_pushdef],
[m4_pushdef([b4_dollar_dollar],
            [_b4_dollar_dollar([$1], [$2], m4_dquote($][1), [$3])])dnl
m4_pushdef([b4_at_dollar], [$4])dnl
])
m4_define([b4_dollar_popdef],
[m4_popdef([b4_at_dollar])dnl
m4_popdef([b4_dollar_dollar])dnl
])
