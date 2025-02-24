                                                            -*- Autoconf -*-

# C M4 Macros for Bison.

# Copyright (C) 2002, 2004-2015, 2018-2021 Free Software Foundation,
# Inc.

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

m4_include(b4_skeletonsdir/[c-like.m4])

# b4_tocpp(STRING)
# ----------------
# Convert STRING into a valid C macro name.
m4_define([b4_tocpp],
[m4_toupper(m4_bpatsubst(m4_quote($1), [[^a-zA-Z0-9]+], [_]))])


# b4_cpp_guard(FILE)
# ------------------
# A valid C macro name to use as a CPP header guard for FILE.
m4_define([b4_cpp_guard],
[[YY_]b4_tocpp(m4_defn([b4_prefix])/[$1])[_INCLUDED]])


# b4_cpp_guard_open(FILE)
# b4_cpp_guard_close(FILE)
# ------------------------
# If FILE does not expand to nothing, open/close CPP inclusion guards for FILE.
m4_define([b4_cpp_guard_open],
[m4_ifval(m4_quote($1),
[#ifndef b4_cpp_guard([$1])
# define b4_cpp_guard([$1])])])

m4_define([b4_cpp_guard_close],
[m4_ifval(m4_quote($1),
[#endif b4_comment([!b4_cpp_guard([$1])])])])


## ---------------- ##
## Identification.  ##
## ---------------- ##

# b4_identification
# -----------------
# Depends on individual skeletons to define b4_pure_flag, b4_push_flag, or
# b4_pull_flag if they use the values of the %define variables api.pure or
# api.push-pull.
m4_define([b4_identification],
[[/* Identify Bison output, and Bison version.  */
#define YYBISON ]b4_version[

/* Bison version string.  */
#define YYBISON_VERSION "]b4_version_string["

/* Skeleton name.  */
#define YYSKELETON_NAME ]b4_skeleton[]m4_ifdef([b4_pure_flag], [[

/* Pure parsers.  */
#define YYPURE ]b4_pure_flag])[]m4_ifdef([b4_push_flag], [[

/* Push parsers.  */
#define YYPUSH ]b4_push_flag])[]m4_ifdef([b4_pull_flag], [[

/* Pull parsers.  */
#define YYPULL ]b4_pull_flag])[
]])


## ---------------- ##
## Default values.  ##
## ---------------- ##

# b4_api_prefix, b4_api_PREFIX
# ----------------------------
# Corresponds to %define api.prefix
b4_percent_define_default([[api.prefix]], [[yy]])
m4_define([b4_api_prefix],
[b4_percent_define_get([[api.prefix]])])
m4_define([b4_api_PREFIX],
[m4_toupper(b4_api_prefix)])


# b4_prefix
# ---------
# If the %name-prefix is not given, it is api.prefix.
m4_define_default([b4_prefix], [b4_api_prefix])

# If the %union is not named, its name is YYSTYPE.
b4_percent_define_default([[api.value.union.name]],
                          [b4_api_PREFIX[][STYPE]])

b4_percent_define_default([[api.symbol.prefix]], [[YYSYMBOL_]])

## ------------------------ ##
## Pure/impure interfaces.  ##
## ------------------------ ##

# b4_yylex_formals
# ----------------
# All the yylex formal arguments.
# b4_lex_param arrives quoted twice, but we want to keep only one level.
m4_define([b4_yylex_formals],
[b4_pure_if([[[b4_api_PREFIX[STYPE *yylvalp]], [[&yylval]]][]dnl
b4_locations_if([, [b4_api_PREFIX[LTYPE *yyllocp], [&yylloc]]])])dnl
m4_ifdef([b4_lex_param], [, ]b4_lex_param)])


# b4_yylex
# --------
# Call yylex.
m4_define([b4_yylex],
[b4_function_call([yylex], [int], b4_yylex_formals)])


# b4_user_args
# ------------
m4_define([b4_user_args],
[m4_ifset([b4_parse_param], [, b4_user_args_no_comma])])

# b4_user_args_no_comma
# ---------------------
m4_define([b4_user_args_no_comma],
[m4_ifset([b4_parse_param], [b4_args(b4_parse_param)])])


# b4_user_formals
# ---------------
# The possible parse-params formal arguments preceded by a comma.
m4_define([b4_user_formals],
[m4_ifset([b4_parse_param], [, b4_formals(b4_parse_param)])])


# b4_parse_param
# --------------
# If defined, b4_parse_param arrives double quoted, but below we prefer
# it to be single quoted.
m4_define([b4_parse_param],
b4_parse_param)


# b4_parse_param_for(DECL, FORMAL, BODY)
# ---------------------------------------
# Iterate over the user parameters, binding the declaration to DECL,
# the formal name to FORMAL, and evaluating the BODY.
m4_define([b4_parse_param_for],
[m4_foreach([$1_$2], m4_defn([b4_parse_param]),
[m4_pushdef([$1], m4_unquote(m4_car($1_$2)))dnl
m4_pushdef([$2], m4_shift($1_$2))dnl
$3[]dnl
m4_popdef([$2])dnl
m4_popdef([$1])dnl
])])


# b4_use(EXPR)
# ------------
# Pacify the compiler about some maybe unused value.
m4_define([b4_use],
[YY_USE ($1)])

# b4_parse_param_use([VAL], [LOC])
# --------------------------------
# 'YY_USE' VAL, LOC if locations are enabled, and all the parse-params.
m4_define([b4_parse_param_use],
[m4_ifvaln([$1], [  b4_use([$1]);])dnl
b4_locations_if([m4_ifvaln([$2], [  b4_use([$2]);])])dnl
b4_parse_param_for([Decl], [Formal], [  b4_use(Formal);
])dnl
])


## ------------ ##
## Data Types.  ##
## ------------ ##

# b4_int_type(MIN, MAX)
# ---------------------
# Return a narrow int type able to handle integers ranging from MIN
# to MAX (included) in portable C code.  Assume MIN and MAX fall in
# 'int' range.
m4_define([b4_int_type],
[m4_if(b4_ints_in($@,   [-127],   [127]), [1], [signed char],
       b4_ints_in($@,      [0],   [255]), [1], [unsigned char],

       b4_ints_in($@, [-32767], [32767]), [1], [short],
       b4_ints_in($@,      [0], [65535]), [1], [unsigned short],

                                               [int])])

# b4_c99_int_type(MIN, MAX)
# -------------------------
# Like b4_int_type, but for C99.
# b4_c99_int_type_define replaces b4_int_type with this.
m4_define([b4_c99_int_type],
[m4_if(b4_ints_in($@,   [-127],   [127]), [1], [yytype_int8],
       b4_ints_in($@,      [0],   [255]), [1], [yytype_uint8],

       b4_ints_in($@, [-32767], [32767]), [1], [yytype_int16],
       b4_ints_in($@,      [0], [65535]), [1], [yytype_uint16],

                                               [int])])

# b4_c99_int_type_define
# ----------------------
# Define private types suitable for holding small integers in C99 or later.
m4_define([b4_c99_int_type_define],
[m4_copy_force([b4_c99_int_type], [b4_int_type])dnl
[#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif]])


# b4_sizes_types_define
# ---------------------
# Define YYPTRDIFF_T/YYPTRDIFF_MAXIMUM, YYSIZE_T/YYSIZE_MAXIMUM,
# and YYSIZEOF.
m4_define([b4_sizes_types_define],
[[#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))
]])


# b4_int_type_for(NAME)
# ---------------------
# Return a narrow int type able to handle numbers ranging from
# 'NAME_min' to 'NAME_max' (included).
m4_define([b4_int_type_for],
[b4_int_type($1_min, $1_max)])


# b4_table_value_equals(TABLE, VALUE, LITERAL, SYMBOL)
# ----------------------------------------------------
# Without inducing a comparison warning from the compiler, check if the
# literal value LITERAL equals VALUE from table TABLE, which must have
# TABLE_min and TABLE_max defined.  SYMBOL denotes
m4_define([b4_table_value_equals],
[m4_if(m4_eval($3 < m4_indir([b4_]$1[_min])
               || m4_indir([b4_]$1[_max]) < $3), [1],
       [[0]],
       [(($2) == $4)])])


## ----------------- ##
## Compiler issues.  ##
## ----------------- ##

# b4_attribute_define([noreturn])
# -------------------------------
# Provide portable compiler "attributes".  If "noreturn" is passed, define
# _Noreturn.
m4_define([b4_attribute_define],
[[#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

]m4_bmatch([$1], [\bnoreturn\b], [[/* The _Noreturn keyword of C11.  */
]dnl This is close to lib/_Noreturn.h, except that we do enable
dnl the use of [[noreturn]], because _Noreturn is used in places
dnl where [[noreturn]] works in C++.  We need this in particular
dnl because of glr.cc which compiles code from glr.c in C++.
dnl And the C++ compiler chokes on _Noreturn.  Also, we do not
dnl use C' _Noreturn in C++, to avoid -Wc11-extensions warnings.
[#ifndef _Noreturn
# if (defined __cplusplus \
      && ((201103 <= __cplusplus && !(__GNUC__ == 4 && __GNUC_MINOR__ == 7)) \
          || (defined _MSC_VER && 1900 <= _MSC_VER)))
#  define _Noreturn [[noreturn]]
# elif ((!defined __cplusplus || defined __clang__) \
        && (201112 <= (defined __STDC_VERSION__ ? __STDC_VERSION__ : 0) \
            || (!defined __STRICT_ANSI__ \
                && (4 < __GNUC__ + (7 <= __GNUC_MINOR__) \
                    || (defined __apple_build_version__ \
                        ? 6000000 <= __apple_build_version__ \
                        : 3 < __clang_major__ + (5 <= __clang_minor__))))))
   /* _Noreturn works as-is.  */
# elif (2 < __GNUC__ + (8 <= __GNUC_MINOR__) || defined __clang__ \
        || 0x5110 <= __SUNPRO_C)
#  define _Noreturn __attribute__ ((__noreturn__))
# elif 1200 <= (defined _MSC_VER ? _MSC_VER : 0)
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif

]])[/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif
]])


# b4_cast_define
# --------------
m4_define([b4_cast_define],
[# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif[]dnl
])


# b4_null_define
# --------------
# Portability issues: define a YY_NULLPTR appropriate for the current
# language (C, C++98, or C++11).
#
# In C++ pre C++11 it is standard practice to use 0 (not NULL) for the
# null pointer.  In C, prefer ((void*)0) to avoid having to include stdlib.h.
m4_define([b4_null_define],
[# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif[]dnl
])


# b4_null
# -------
# Return a null pointer constant.
m4_define([b4_null], [YY_NULLPTR])



## ---------##
## Values.  ##
## ---------##

# b4_integral_parser_table_define(TABLE-NAME, CONTENT, COMMENT)
# -------------------------------------------------------------
# Define "yy<TABLE-NAME>" whose contents is CONTENT.
m4_define([b4_integral_parser_table_define],
[m4_ifvaln([$3], [b4_comment([$3])])dnl
static const b4_int_type_for([$2]) yy$1[[]] =
{
  $2
};dnl
])


## ------------- ##
## Token kinds.  ##
## ------------- ##

# Because C enums are not scoped, because tokens are exposed in the
# header, and because these tokens are common to all the parsers, we
# need to make sure their names don't collide: use the api.prefix.
# YYEOF is special, since the user may give it a different name.
m4_define([b4_symbol(-2, id)],  [b4_api_PREFIX[][EMPTY]])
m4_define([b4_symbol(-2, tag)], [[No symbol.]])

m4_if(b4_symbol(eof, id), [YYEOF],
     [m4_define([b4_symbol(0, id)],  [b4_api_PREFIX[][EOF]])])
m4_define([b4_symbol(1, id)],  [b4_api_PREFIX[][error]])
m4_define([b4_symbol(2, id)],  [b4_api_PREFIX[][UNDEF]])


# b4_token_define(TOKEN-NUM)
# --------------------------
# Output the definition of this token as #define.
m4_define([b4_token_define],
[b4_token_format([#define %s %s], [$1])])

# b4_token_defines
# ----------------
# Output the definition of the tokens.
m4_define([b4_token_defines],
[[/* Token kinds.  */
#define ]b4_symbol(empty, [id])[ -2
]m4_join([
], b4_symbol_map([b4_token_define]))
])


# b4_token_enum(TOKEN-NUM)
# ------------------------
# Output the definition of this token as an enum.
m4_define([b4_token_enum],
[b4_token_visible_if([$1],
    [m4_format([    %-30s %s],
               m4_format([[%s = %s%s%s]],
                         b4_symbol([$1], [id]),
                         b4_symbol([$1], b4_api_token_raw_if([[number]], [[code]])),
                         m4_if([$1], b4_last_enum_token, [], [[,]])),
               [b4_symbol_tag_comment([$1])])])])


# b4_token_enums
# --------------
# The definition of the token kinds.
m4_define([b4_token_enums],
[b4_any_token_visible_if([[/* Token kinds.  */
#ifndef ]b4_api_PREFIX[TOKENTYPE
# define ]b4_api_PREFIX[TOKENTYPE
  enum ]b4_api_prefix[tokentype
  {
    ]b4_symbol(empty, [id])[ = -2,
]b4_symbol_foreach([b4_token_enum])dnl
[  };
  typedef enum ]b4_api_prefix[tokentype ]b4_api_prefix[token_kind_t;
#endif
]])])


# b4_token_enums_defines
# ----------------------
# The definition of the tokens (if there are any) as enums and,
# if POSIX Yacc is enabled, as #defines.
m4_define([b4_token_enums_defines],
[b4_token_enums[]b4_yacc_if([b4_token_defines])])


# b4_symbol_translate(STRING)
# ---------------------------
# Used by "bison" in the array of symbol names to mark those that
# require translation.
m4_define([b4_symbol_translate],
[[N_($1)]])



## -------------- ##
## Symbol kinds.  ##
## -------------- ##

# b4_symbol_enum(SYMBOL-NUM)
# --------------------------
# Output the definition of this symbol as an enum.
m4_define([b4_symbol_enum],
[m4_format([  %-40s %s],
           m4_format([[%s = %s%s%s]],
                     b4_symbol([$1], [kind_base]),
                     [$1],
                     m4_if([$1], b4_last_symbol, [], [[,]])),
           [b4_symbol_tag_comment([$1])])])


# b4_declare_symbol_enum
# ----------------------
# The definition of the symbol internal numbers as an enum.
# Defining YYEMPTY here is important: it forces the compiler
# to use a signed type, which matters for yytoken.
m4_define([b4_declare_symbol_enum],
[[/* Symbol kind.  */
enum yysymbol_kind_t
{
  ]b4_symbol(empty, [kind_base])[ = -2,
]b4_symbol_foreach([b4_symbol_enum])dnl
[};
typedef enum yysymbol_kind_t yysymbol_kind_t;
]])])


## ----------------- ##
## Semantic Values.  ##
## ----------------- ##


# b4_symbol_value(VAL, [SYMBOL-NUM], [TYPE-TAG])
# ----------------------------------------------
# See README.
m4_define([b4_symbol_value],
[m4_ifval([$3],
          [($1.$3)],
          [m4_ifval([$2],
                    [b4_symbol_if([$2], [has_type],
                                  [($1.b4_symbol([$2], [type]))],
                                  [$1])],
                    [$1])])])


## ---------------------- ##
## Defining C functions.  ##
## ---------------------- ##


# b4_formals([DECL1, NAME1], ...)
# -------------------------------
# The formal arguments of a C function definition.
m4_define([b4_formals],
[m4_if([$#], [0], [void],
       [$#$1], [1], [void],
               [m4_map_sep([b4_formal], [, ], [$@])])])

m4_define([b4_formal],
[$1])


# b4_function_declare(NAME, RETURN-VALUE, [DECL1, NAME1], ...)
# ------------------------------------------------------------
# Declare the function NAME.
m4_define([b4_function_declare],
[$2 $1 (b4_formals(m4_shift2($@)));[]dnl
])



## --------------------- ##
## Calling C functions.  ##
## --------------------- ##


# b4_function_call(NAME, RETURN-VALUE, [DECL1, NAME1], ...)
# -----------------------------------------------------------
# Call the function NAME with arguments NAME1, NAME2 etc.
m4_define([b4_function_call],
[$1 (b4_args(m4_shift2($@)))[]dnl
])


# b4_args([DECL1, NAME1], ...)
# ----------------------------
# Output the arguments NAME1, NAME2...
m4_define([b4_args],
[m4_map_sep([b4_arg], [, ], [$@])])

m4_define([b4_arg],
[$2])


## ----------- ##
## Synclines.  ##
## ----------- ##

# b4_sync_start(LINE, FILE)
# -------------------------
m4_define([b4_sync_start], [[#]line $1 $2])


## -------------- ##
## User actions.  ##
## -------------- ##

# b4_case(LABEL, STATEMENTS, [COMMENTS])
# --------------------------------------
m4_define([b4_case],
[  case $1:m4_ifval([$3], [ b4_comment([$3])])
$2
b4_syncline([@oline@], [@ofile@])dnl
    break;])


# b4_predicate_case(LABEL, CONDITIONS)
# ------------------------------------
m4_define([b4_predicate_case],
[  case $1:
    if (! (
$2)) YYERROR;
b4_syncline([@oline@], [@ofile@])dnl
    break;])


# b4_yydestruct_define
# --------------------
# Define the "yydestruct" function.
m4_define_default([b4_yydestruct_define],
[[/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep]b4_locations_if(dnl
[[, YYLTYPE *yylocationp]])[]b4_user_formals[)
{
]b4_parse_param_use([yyvaluep], [yylocationp])dnl
[  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  ]b4_symbol_actions([destructor])[
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}]dnl
])


# b4_yy_symbol_print_define
# -------------------------
# Define the "yy_symbol_print" function.
m4_define_default([b4_yy_symbol_print_define],
[[
/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep]b4_locations_if(dnl
[[, YYLTYPE const * const yylocationp]])[]b4_user_formals[)
{
  FILE *yyoutput = yyo;
]b4_parse_param_use([yyoutput], [yylocationp])dnl
[  if (!yyvaluep)
    return;]
b4_percent_code_get([[pre-printer]])dnl
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  b4_symbol_actions([printer])
  YY_IGNORE_MAYBE_UNINITIALIZED_END
b4_percent_code_get([[post-printer]])dnl
[}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep]b4_locations_if(dnl
[[, YYLTYPE const * const yylocationp]])[]b4_user_formals[)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

]b4_locations_if([  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
])dnl
[  yy_symbol_value_print (yyo, yykind, yyvaluep]dnl
b4_locations_if([, yylocationp])[]b4_user_args[);
  YYFPRINTF (yyo, ")");
}]dnl
])


## ---------------- ##
## api.value.type.  ##
## ---------------- ##


# ---------------------- #
# api.value.type=union.  #
# ---------------------- #

# b4_symbol_type_register(SYMBOL-NUM)
# -----------------------------------
# Symbol SYMBOL-NUM has a type (for variant) instead of a type-tag.
# Extend the definition of %union's body (b4_union_members) with a
# field of that type, and extend the symbol's "type" field to point to
# the field name, instead of the type name.
m4_define([b4_symbol_type_register],
[m4_define([b4_symbol($1, type_tag)],
           [b4_symbol_if([$1], [has_id],
                         [b4_symbol([$1], [id])],
                         [yykind_[]b4_symbol([$1], [number])])])dnl
m4_append([b4_union_members],
m4_expand([m4_format([  %-40s %s],
                     m4_expand([b4_symbol([$1], [type]) b4_symbol([$1], [type_tag]);]),
                     [b4_symbol_tag_comment([$1])])]))
])


# b4_type_define_tag(SYMBOL1-NUM, ...)
# ------------------------------------
# For the batch of symbols SYMBOL1-NUM... (which all have the same
# type), enhance the %union definition for each of them, and set
# there "type" field to the field tag name, instead of the type name.
m4_define([b4_type_define_tag],
[b4_symbol_if([$1], [has_type],
              [m4_map([b4_symbol_type_register], [$@])])
])


# b4_symbol_value_union(VAL, SYMBOL-NUM, [TYPE])
# ----------------------------------------------
# Same of b4_symbol_value, but when api.value.type=union.
m4_define([b4_symbol_value_union],
[m4_ifval([$3],
          [(*($3*)(&$1))],
          [m4_ifval([$2],
                    [b4_symbol_if([$2], [has_type],
                                  [($1.b4_symbol([$2], [type_tag]))],
                                  [$1])],
                    [$1])])])


# b4_value_type_setup_union
# -------------------------
# Setup support for api.value.type=union.  Symbols are defined with a
# type instead of a union member name: build the corresponding union,
# and give the symbols their tag.
m4_define([b4_value_type_setup_union],
[m4_define([b4_union_members])
b4_type_foreach([b4_type_define_tag])
m4_copy_force([b4_symbol_value_union], [b4_symbol_value])
])


# -------------------------- #
# api.value.type = variant.  #
# -------------------------- #

# b4_value_type_setup_variant
# ---------------------------
# Setup support for api.value.type=variant.  By default, fail, specialized
# by other skeletons.
m4_define([b4_value_type_setup_variant],
[b4_complain_at(b4_percent_define_get_loc([[api.value.type]]),
                [['%s' does not support '%s']],
                [b4_skeleton],
                [%define api.value.type variant])])


# _b4_value_type_setup_keyword
# ----------------------------
# api.value.type is defined with a keyword/string syntax.  Check if
# that is properly defined, and prepare its use.
m4_define([_b4_value_type_setup_keyword],
[b4_percent_define_check_values([[[[api.value.type]],
                                  [[none]],
                                  [[union]],
                                  [[union-directive]],
                                  [[variant]],
                                  [[yystype]]]])dnl
m4_case(b4_percent_define_get([[api.value.type]]),
        [union],   [b4_value_type_setup_union],
        [variant], [b4_value_type_setup_variant])])


# b4_value_type_setup
# -------------------
# Check if api.value.type is properly defined, and possibly prepare
# its use.
b4_define_silent([b4_value_type_setup],
[# Define default value.
b4_percent_define_ifdef([[api.value.type]], [],
[# %union => api.value.type=union-directive
m4_ifdef([b4_union_members],
[m4_define([b4_percent_define_kind(api.value.type)], [keyword])
m4_define([b4_percent_define(api.value.type)], [union-directive])],
[# no tag seen => api.value.type={int}
m4_if(b4_tag_seen_flag, 0,
[m4_define([b4_percent_define_kind(api.value.type)], [code])
m4_define([b4_percent_define(api.value.type)], [int])],
[# otherwise api.value.type=yystype
m4_define([b4_percent_define_kind(api.value.type)], [keyword])
m4_define([b4_percent_define(api.value.type)], [yystype])])])])

# Set up.
m4_bmatch(b4_percent_define_get_kind([[api.value.type]]),
   [keyword\|string], [_b4_value_type_setup_keyword])
])


## -------------- ##
## Declarations.  ##
## -------------- ##


# b4_value_type_define
# --------------------
m4_define([b4_value_type_define],
[b4_value_type_setup[]dnl
/* Value type.  */
m4_bmatch(b4_percent_define_get_kind([[api.value.type]]),
[code],
[[#if ! defined ]b4_api_PREFIX[STYPE && ! defined ]b4_api_PREFIX[STYPE_IS_DECLARED
typedef ]b4_percent_define_get([[api.value.type]])[ ]b4_api_PREFIX[STYPE;
# define ]b4_api_PREFIX[STYPE_IS_TRIVIAL 1
# define ]b4_api_PREFIX[STYPE_IS_DECLARED 1
#endif
]],
[m4_bmatch(b4_percent_define_get([[api.value.type]]),
[union\|union-directive],
[[#if ! defined ]b4_api_PREFIX[STYPE && ! defined ]b4_api_PREFIX[STYPE_IS_DECLARED
]b4_percent_define_get_syncline([[api.value.union.name]])dnl
[union ]b4_percent_define_get([[api.value.union.name]])[
{
]b4_user_union_members[
};
]b4_percent_define_get_syncline([[api.value.union.name]])dnl
[typedef union ]b4_percent_define_get([[api.value.union.name]])[ ]b4_api_PREFIX[STYPE;
# define ]b4_api_PREFIX[STYPE_IS_TRIVIAL 1
# define ]b4_api_PREFIX[STYPE_IS_DECLARED 1
#endif
]])])])


# b4_location_type_define
# -----------------------
m4_define([b4_location_type_define],
[[/* Location type.  */
]b4_percent_define_ifdef([[api.location.type]],
[[typedef ]b4_percent_define_get([[api.location.type]])[ ]b4_api_PREFIX[LTYPE;
]],
[[#if ! defined ]b4_api_PREFIX[LTYPE && ! defined ]b4_api_PREFIX[LTYPE_IS_DECLARED
typedef struct ]b4_api_PREFIX[LTYPE ]b4_api_PREFIX[LTYPE;
struct ]b4_api_PREFIX[LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define ]b4_api_PREFIX[LTYPE_IS_DECLARED 1
# define ]b4_api_PREFIX[LTYPE_IS_TRIVIAL 1
#endif
]])])


# b4_declare_yylstype
# -------------------
# Declarations that might either go into the header (if --header) or
# in the parser body.  Declare YYSTYPE/YYLTYPE, and yylval/yylloc.
m4_define([b4_declare_yylstype],
[b4_value_type_define[]b4_locations_if([
b4_location_type_define])

b4_pure_if([], [[extern ]b4_api_PREFIX[STYPE ]b4_prefix[lval;
]b4_locations_if([[extern ]b4_api_PREFIX[LTYPE ]b4_prefix[lloc;]])])[]dnl
])


# b4_YYDEBUG_define
# -----------------
m4_define([b4_YYDEBUG_define],
[[/* Debug traces.  */
]m4_if(b4_api_prefix, [yy],
[[#ifndef YYDEBUG
# define YYDEBUG ]b4_parse_trace_if([1], [0])[
#endif]],
[[#ifndef ]b4_api_PREFIX[DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ]b4_api_PREFIX[DEBUG 1
#  else
#   define ]b4_api_PREFIX[DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ]b4_api_PREFIX[DEBUG ]b4_parse_trace_if([1], [0])[
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ]b4_api_PREFIX[DEBUG */]])[]dnl
])

# b4_declare_yydebug
# ------------------
m4_define([b4_declare_yydebug],
[b4_YYDEBUG_define[
#if ]b4_api_PREFIX[DEBUG
extern int ]b4_prefix[debug;
#endif][]dnl
])

# b4_yylloc_default_define
# ------------------------
# Define YYLLOC_DEFAULT.
m4_define([b4_yylloc_default_define],
[[/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif
]])

# b4_yylocation_print_define
# --------------------------
# Define YYLOCATION_PRINT.
m4_define([b4_yylocation_print_define],
[b4_locations_if([[
/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined ]b4_api_PREFIX[LTYPE_IS_TRIVIAL && ]b4_api_PREFIX[LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */]])
])

# b4_yyloc_default
# ----------------
# Expand to a possible default value for yylloc.
m4_define([b4_yyloc_default],
[[
# if defined ]b4_api_PREFIX[LTYPE_IS_TRIVIAL && ]b4_api_PREFIX[LTYPE_IS_TRIVIAL
  = { ]m4_join([, ],
               m4_defn([b4_location_initial_line]),
               m4_defn([b4_location_initial_column]),
               m4_defn([b4_location_initial_line]),
               m4_defn([b4_location_initial_column]))[ }
# endif
]])
