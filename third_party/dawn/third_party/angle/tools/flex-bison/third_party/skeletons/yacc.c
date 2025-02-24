#                                                            -*- C -*-
# Yacc compatible skeleton for Bison

# Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software
# Foundation, Inc.

m4_pushdef([b4_copyright_years],
           [1984, 1989-1990, 2000-2015, 2018-2021])

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

m4_include(b4_skeletonsdir/[c.m4])


## ---------- ##
## api.pure.  ##
## ---------- ##

b4_percent_define_default([[api.pure]], [[false]])
b4_percent_define_check_values([[[[api.pure]],
                                 [[false]], [[true]], [[]], [[full]]]])

m4_define([b4_pure_flag], [[0]])
m4_case(b4_percent_define_get([[api.pure]]),
        [false], [m4_define([b4_pure_flag], [[0]])],
        [true],  [m4_define([b4_pure_flag], [[1]])],
        [],      [m4_define([b4_pure_flag], [[1]])],
        [full],  [m4_define([b4_pure_flag], [[2]])])

m4_define([b4_pure_if],
[m4_case(b4_pure_flag,
         [0], [$2],
         [1], [$1],
         [2], [$1])])
         [m4_fatal([invalid api.pure value: ]$1)])])

## --------------- ##
## api.push-pull.  ##
## --------------- ##

# b4_pull_if, b4_push_if
# ----------------------
# Whether the pull/push APIs are needed.  Both can be enabled.

b4_percent_define_default([[api.push-pull]], [[pull]])
b4_percent_define_check_values([[[[api.push-pull]],
                                 [[pull]], [[push]], [[both]]]])
b4_define_flag_if([pull]) m4_define([b4_pull_flag], [[1]])
b4_define_flag_if([push]) m4_define([b4_push_flag], [[1]])
m4_case(b4_percent_define_get([[api.push-pull]]),
        [pull], [m4_define([b4_push_flag], [[0]])],
        [push], [m4_define([b4_pull_flag], [[0]])])

# Handle BISON_USE_PUSH_FOR_PULL for the test suite.  So that push parsing
# tests function as written, do not let BISON_USE_PUSH_FOR_PULL modify the
# behavior of Bison at all when push parsing is already requested.
b4_define_flag_if([use_push_for_pull])
b4_use_push_for_pull_if([
  b4_push_if([m4_define([b4_use_push_for_pull_flag], [[0]])],
             [m4_define([b4_push_flag], [[1]])])])

## ----------- ##
## parse.lac.  ##
## ----------- ##

b4_percent_define_default([[parse.lac]], [[none]])
b4_percent_define_default([[parse.lac.es-capacity-initial]], [[20]])
b4_percent_define_default([[parse.lac.memory-trace]], [[failures]])
b4_percent_define_check_values([[[[parse.lac]], [[full]], [[none]]]],
                               [[[[parse.lac.memory-trace]],
                                 [[failures]], [[full]]]])
b4_define_flag_if([lac])
m4_define([b4_lac_flag],
          [m4_if(b4_percent_define_get([[parse.lac]]),
                 [none], [[0]], [[1]])])

## ---------------- ##
## Default values.  ##
## ---------------- ##

# Stack parameters.
m4_define_default([b4_stack_depth_max], [10000])
m4_define_default([b4_stack_depth_init],  [200])


# b4_yyerror_arg_loc_if(ARG)
# --------------------------
# Expand ARG iff yyerror is to be given a location as argument.
m4_define([b4_yyerror_arg_loc_if],
[b4_locations_if([m4_case(b4_pure_flag,
                          [1], [m4_ifset([b4_parse_param], [$1])],
                          [2], [$1])])])

# b4_yyerror_formals
# ------------------
m4_define([b4_yyerror_formals],
[b4_pure_if([b4_locations_if([, [[const ]b4_api_PREFIX[LTYPE *yyllocp], [&yylloc]]])[]dnl
m4_ifdef([b4_parse_param], [, b4_parse_param])[]dnl
,])dnl
[[const char *msg], [msg]]])



# b4_yyerror_args
# ---------------
# Arguments passed to yyerror: user args plus yylloc.
m4_define([b4_yyerror_args],
[b4_yyerror_arg_loc_if([&yylloc, ])dnl
m4_ifset([b4_parse_param], [b4_args(b4_parse_param), ])])



## ----------------- ##
## Semantic Values.  ##
## ----------------- ##


# b4_accept([SYMBOL-NUM])
# -----------------------
# Used in actions of the rules of accept, the initial symbol, to call
# YYACCEPT.  If SYMBOL-NUM is specified, run "yyvalue->SLOT = $2;"
# before, using the slot of SYMBOL-NUM.
m4_define([b4_accept],
[m4_ifval([$1],
          [b4_symbol_value(yyimpl->yyvalue, [$1]) = b4_rhs_value(2, 1, [$1]); ]) YYACCEPT])


# b4_lhs_value(SYMBOL-NUM, [TYPE])
# --------------------------------
# See README.
m4_define([b4_lhs_value],
[b4_symbol_value(yyval, [$1], [$2])])


# b4_rhs_value(RULE-LENGTH, POS, [SYMBOL-NUM], [TYPE])
# ----------------------------------------------------
# See README.
m4_define([b4_rhs_value],
[b4_symbol_value([yyvsp@{b4_subtract([$2], [$1])@}], [$3], [$4])])


## ----------- ##
## Locations.  ##
## ----------- ##

# b4_lhs_location()
# -----------------
# Expansion of @$.
# Overparenthetized to avoid obscure problems with "foo$$bar = foo$1bar".
m4_define([b4_lhs_location],
[(yyloc)])


# b4_rhs_location(RULE-LENGTH, POS)
# ---------------------------------
# Expansion of @POS, where the current rule has RULE-LENGTH symbols
# on RHS.
# Overparenthetized to avoid obscure problems with "foo$$bar = foo$1bar".
m4_define([b4_rhs_location],
[(yylsp@{b4_subtract([$2], [$1])@})])


## -------------- ##
## Declarations.  ##
## -------------- ##

# _b4_declare_sub_yyparse(START-SYMBOL-NUM, SWITCHING-TOKEN-SYMBOL-NUM)
# ---------------------------------------------------------------------
# Define the return type of the parsing function for SYMBOL-NUM, and
# declare its parsing function.
m4_define([_b4_declare_sub_yyparse],
[[
// Return type when parsing one ]_b4_symbol($1, tag)[.
typedef struct
{]b4_symbol_if([$1], [has_type], [[
  ]_b4_symbol($1, type)[ yyvalue;]])[
  int yystatus;
  int yynerrs;
} ]b4_prefix[parse_]_b4_symbol($1, id)[_t;

// Parse one ]_b4_symbol($1, tag)[.
]b4_prefix[parse_]_b4_symbol($1, id)[_t ]b4_prefix[parse_]_b4_symbol($1, id)[ (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[);
]])


# _b4_first_switching_token
# -------------------------
m4_define([b4_first], [$1])
m4_define([b4_second], [$2])
m4_define([_b4_first_switching_token],
[b4_second(b4_first(b4_start_symbols))])


# _b4_define_sub_yyparse(START-SYMBOL-NUM, SWITCHING-TOKEN-SYMBOL-NUM)
# --------------------------------------------------------------------
# Define the parsing function for START-SYMBOL-NUM.
m4_define([_b4_define_sub_yyparse],
[[
]b4_prefix[parse_]_b4_symbol($1, id)[_t
]b4_prefix[parse_]_b4_symbol($1, id)[ (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[)
{
  ]b4_prefix[parse_]_b4_symbol($1, id)[_t yyres;
  yy_parse_impl_t yyimpl;
  yyres.yystatus = yy_parse_impl (]b4_symbol($2, id)[, &yyimpl]m4_ifset([b4_parse_param],
                           [[, ]b4_args(b4_parse_param)])[);]b4_symbol_if([$1], [has_type], [[
  yyres.yyvalue = yyimpl.yyvalue.]b4_symbol($1, slot)[;]])[
  yyres.yynerrs = yyimpl.yynerrs;
  return yyres;
}
]])


# b4_declare_scanner_communication_variables
# ------------------------------------------
# Declare the variables that are global, or local to YYPARSE if
# pure-parser.
m4_define([b4_declare_scanner_communication_variables], [[
]m4_ifdef([b4_start_symbols], [],
[[/* Lookahead token kind.  */
int yychar;
]])[
]b4_pure_if([[
/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);]b4_locations_if([[

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default]b4_yyloc_default[;
YYLTYPE yylloc = yyloc_default;]])],
[[/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;]b4_locations_if([[
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc]b4_yyloc_default[;]])[
/* Number of syntax errors so far.  */
int yynerrs;]])])


# b4_declare_parser_state_variables([INIT])
# -----------------------------------------
# Declare all the variables that are needed to maintain the parser state
# between calls to yypush_parse.
# If INIT is non-null, initialize these variables.
m4_define([b4_declare_parser_state_variables],
[b4_pure_if([[
    /* Number of syntax errors so far.  */
    int yynerrs]m4_ifval([$1], [ = 0])[;
]])[
    yy_state_fast_t yystate]m4_ifval([$1], [ = 0])[;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus]m4_ifval([$1], [ = 0])[;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize]m4_ifval([$1], [ = YYINITDEPTH])[;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss]m4_ifval([$1], [ = yyssa])[;
    yy_state_t *yyssp]m4_ifval([$1], [ = yyss])[;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs]m4_ifval([$1], [ = yyvsa])[;
    YYSTYPE *yyvsp]m4_ifval([$1], [ = yyvs])[;]b4_locations_if([[

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls]m4_ifval([$1], [ = yylsa])[;
    YYLTYPE *yylsp]m4_ifval([$1], [ = yyls])[;]])[]b4_lac_if([[

    yy_state_t yyesa@{]b4_percent_define_get([[parse.lac.es-capacity-initial]])[@};
    yy_state_t *yyes]m4_ifval([$1], [ = yyesa])[;
    YYPTRDIFF_T yyes_capacity][]m4_ifval([$1],
            [m4_do([ = b4_percent_define_get([[parse.lac.es-capacity-initial]]) < YYMAXDEPTH],
                   [ ? b4_percent_define_get([[parse.lac.es-capacity-initial]])],
                   [ : YYMAXDEPTH])])[;]])])


m4_define([b4_macro_define],
[[#]define $1 $2])

m4_define([b4_macro_undef],
[[#]undef $1])

m4_define([b4_pstate_macro_define],
[b4_macro_define([$1], [yyps->$1])])

# b4_parse_state_variable_macros(b4_macro_define|b4_macro_undef)
# --------------------------------------------------------------
m4_define([b4_parse_state_variable_macros],
[b4_pure_if([$1([b4_prefix[]nerrs])])
$1([yystate])
$1([yyerrstatus])
$1([yyssa])
$1([yyss])
$1([yyssp])
$1([yyvsa])
$1([yyvs])
$1([yyvsp])[]b4_locations_if([
$1([yylsa])
$1([yyls])
$1([yylsp])])
$1([yystacksize])[]b4_lac_if([
$1([yyesa])
$1([yyes])
$1([yyes_capacity])])])




# _b4_declare_yyparse_push
# ------------------------
# Declaration of yyparse (and dependencies) when using the push parser
# (including in pull mode).
m4_define([_b4_declare_yyparse_push],
[[#ifndef YYPUSH_MORE_DEFINED
# define YYPUSH_MORE_DEFINED
enum { YYPUSH_MORE = 4 };
#endif

typedef struct ]b4_prefix[pstate ]b4_prefix[pstate;

]b4_pull_if([[
int ]b4_prefix[parse (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[);]])[
int ]b4_prefix[push_parse (]b4_prefix[pstate *ps]b4_pure_if([[,
                  int pushed_char, ]b4_api_PREFIX[STYPE const *pushed_val]b4_locations_if([[, ]b4_api_PREFIX[LTYPE *pushed_loc]])])b4_user_formals[);
]b4_pull_if([[int ]b4_prefix[pull_parse (]b4_prefix[pstate *ps]b4_user_formals[);]])[
]b4_prefix[pstate *]b4_prefix[pstate_new (void);
void ]b4_prefix[pstate_delete (]b4_prefix[pstate *ps);
]])


# _b4_declare_yyparse
# -------------------
# When not the push parser.
m4_define([_b4_declare_yyparse],
[[int ]b4_prefix[parse (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[);
]m4_ifdef([b4_start_symbols],
          [m4_map([_b4_declare_sub_yyparse], m4_defn([b4_start_symbols]))])])


# b4_declare_yyparse
# ------------------
m4_define([b4_declare_yyparse],
[b4_push_if([_b4_declare_yyparse_push],
            [_b4_declare_yyparse])[]dnl
])


# b4_declare_yyerror_and_yylex
# ----------------------------
# Comply with POSIX Yacc.
# <https://austingroupbugs.net/view.php?id=1388#c5220>
m4_define([b4_declare_yyerror_and_yylex],
[b4_posix_if([[#if !defined ]b4_prefix[error && !defined ]b4_api_PREFIX[ERROR_IS_DECLARED
]b4_function_declare([b4_prefix[error]], void, b4_yyerror_formals)[
#endif
#if !defined ]b4_prefix[lex && !defined ]b4_api_PREFIX[LEX_IS_DECLARED
]b4_function_declare([b4_prefix[lex]], int, b4_yylex_formals)[
#endif
]])dnl
])


# b4_shared_declarations
# ----------------------
# Declarations that might either go into the header (if --header)
# or into the implementation file.
m4_define([b4_shared_declarations],
[b4_cpp_guard_open([b4_spec_mapped_header_file])[
]b4_declare_yydebug[
]b4_percent_code_get([[requires]])[
]b4_token_enums_defines[
]b4_declare_yylstype[
]b4_declare_yyerror_and_yylex[
]b4_declare_yyparse[
]b4_percent_code_get([[provides]])[
]b4_cpp_guard_close([b4_spec_mapped_header_file])[]dnl
])


# b4_header_include_if(IF-TRUE, IF-FALSE)
# ---------------------------------------
# Run IF-TRUE if we generate an output file and api.header.include
# is defined.
m4_define([b4_header_include_if],
[m4_ifval(m4_quote(b4_spec_header_file),
          [b4_percent_define_ifdef([[api.header.include]],
                                   [$1],
                                   [$2])],
          [$2])])

m4_if(b4_spec_header_file, [y.tab.h], [],
      [b4_percent_define_default([[api.header.include]],
                                 [["@basename(]b4_spec_header_file[@)"]])])




## -------------- ##
## Output files.  ##
## -------------- ##


b4_header_if([[
]b4_output_begin([b4_spec_header_file])[
]b4_copyright([Bison interface for Yacc-like parsers in C])[
]b4_disclaimer[
]b4_shared_declarations[
]b4_output_end[
]])# b4_header_if

b4_output_begin([b4_parser_file_name])[
]b4_copyright([Bison implementation for Yacc-like parsers in C])[
/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

]b4_disclaimer[
/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

]b4_identification[
]b4_percent_code_get([[top]])[]dnl
m4_if(b4_api_prefix, [yy], [],
[[/* Substitute the type names.  */
#define YYSTYPE         ]b4_api_PREFIX[STYPE]b4_locations_if([[
#define YYLTYPE         ]b4_api_PREFIX[LTYPE]])])[
]m4_if(b4_prefix, [yy], [],
[[/* Substitute the variable and function names.  */]b4_pull_if([[
#define yyparse         ]b4_prefix[parse]])b4_push_if([[
#define yypush_parse    ]b4_prefix[push_parse]b4_pull_if([[
#define yypull_parse    ]b4_prefix[pull_parse]])[
#define yypstate_new    ]b4_prefix[pstate_new
#define yypstate_clear  ]b4_prefix[pstate_clear
#define yypstate_delete ]b4_prefix[pstate_delete
#define yypstate        ]b4_prefix[pstate]])[
#define yylex           ]b4_prefix[lex
#define yyerror         ]b4_prefix[error
#define yydebug         ]b4_prefix[debug
#define yynerrs         ]b4_prefix[nerrs]]b4_pure_if([], [[
#define yylval          ]b4_prefix[lval
#define yychar          ]b4_prefix[char]b4_locations_if([[
#define yylloc          ]b4_prefix[lloc]])]))[

]b4_user_pre_prologue[
]b4_cast_define[
]b4_null_define[

]b4_header_include_if([[#include ]b4_percent_define_get([[api.header.include]])],
                      [m4_ifval(m4_quote(b4_spec_header_file),
                                [/* Use api.header.include to #include this header
   instead of duplicating it here.  */
])b4_shared_declarations])[
]b4_declare_symbol_enum[

]b4_user_post_prologue[
]b4_percent_code_get[
]b4_c99_int_type_define[

]b4_sizes_types_define[

/* Stored state numbers (used for stacks). */
typedef ]b4_int_type(0, m4_eval(b4_states_number - 1))[ yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif
]b4_has_translations_if([
#ifndef N_
# define N_(Msgid) Msgid
#endif
])[

]b4_attribute_define[

]b4_parse_assert_if([[#ifdef NDEBUG
# define YY_ASSERT(E) ((void) (0 && (E)))
#else
# include <assert.h> /* INFRINGES ON USER NAME SPACE */
# define YY_ASSERT(E) assert (E)
#endif
]],
[[#define YY_ASSERT(E) ((void) (0 && (E)))]])[

#if ]b4_lac_if([[1]], [b4_parse_error_case([simple], [[!defined yyoverflow]], [[1]])])[

/* The parser invokes alloca or malloc; define the necessary symbols.  */]dnl
b4_push_if([], [b4_lac_if([], [[

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif]])])[

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif]b4_lac_if([[
# define YYCOPY_NEEDED 1]])[
#endif /* ]b4_lac_if([[1]], [b4_parse_error_case([simple], [[!defined yyoverflow]], [[1]])])[ */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (]b4_locations_if([[defined ]b4_api_PREFIX[LTYPE_IS_TRIVIAL && ]b4_api_PREFIX[LTYPE_IS_TRIVIAL \
             && ]])[defined ]b4_api_PREFIX[STYPE_IS_TRIVIAL && ]b4_api_PREFIX[STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;]b4_locations_if([
  YYLTYPE yyls_alloc;])[
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
]b4_locations_if(
[# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)],
[# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)])[

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  ]b4_final_state_number[
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   ]b4_last[

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  ]b4_tokens_number[
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  ]b4_nterms_number[
/* YYNRULES -- Number of rules.  */
#define YYNRULES  ]b4_rules_number[
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  ]b4_states_number[

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   ]b4_code_max[


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
]b4_api_token_raw_if(dnl
[[#define YYTRANSLATE(YYX) YY_CAST (yysymbol_kind_t, YYX)]],
[[#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : ]b4_symbol_prefix[YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const ]b4_int_type_for([b4_translate])[ yytranslate[] =
{
  ]b4_translate[
};]])[

#if ]b4_api_PREFIX[DEBUG
]b4_integral_parser_table_define([rline], [b4_rline],
     [[YYRLINE[YYN] -- Source line where rule number YYN was defined.]])[
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if ]b4_parse_error_case([simple], [b4_api_PREFIX[DEBUG || ]b4_token_table_flag], [[1]])[
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

]b4_parse_error_bmatch([simple\|verbose],
[[/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  ]b4_tname[
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}]],
[[static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  ]b4_symbol_names[
  };]b4_has_translations_if([[
  /* YYTRANSLATABLE[SYMBOL-NUM] -- Whether YY_SNAME[SYMBOL-NUM] is
     internationalizable.  */
  static ]b4_int_type_for([b4_translatable])[ yytranslatable[] =
  {
  ]b4_translatable[
  };
  return (yysymbol < YYNTOKENS && yytranslatable[yysymbol]
          ? _(yy_sname[yysymbol])
          : yy_sname[yysymbol]);]], [[
  return yy_sname[yysymbol];]])[
}]])[
#endif

#define YYPACT_NINF (]b4_pact_ninf[)

#define yypact_value_is_default(Yyn) \
  ]b4_table_value_equals([[pact]], [[Yyn]], [b4_pact_ninf], [YYPACT_NINF])[

#define YYTABLE_NINF (]b4_table_ninf[)

#define yytable_value_is_error(Yyn) \
  ]b4_table_value_equals([[table]], [[Yyn]], [b4_table_ninf], [YYTABLE_NINF])[

]b4_parser_tables_define[

enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ]b4_symbol(empty, id)[)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ]b4_symbol(empty, id)[)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \]b4_lac_if([[
        YY_LAC_DISCARD ("YYBACKUP");                              \]])[
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (]b4_yyerror_args[YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ]b4_symbol(error, id)[ or ]b4_symbol(undef, id)[. */
#define YYERRCODE ]b4_symbol(undef, id)[
]b4_locations_if([[
]b4_yylloc_default_define[
#define YYRHSLOC(Rhs, K) ((Rhs)[K])
]])[

/* Enable debugging if requested.  */
#if ]b4_api_PREFIX[DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

]b4_yylocation_print_define[

# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value]b4_locations_if([, Location])[]b4_user_args[); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)

]b4_yy_symbol_print_define[

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,]b4_locations_if([[ YYLTYPE *yylsp,]])[
                 int yyrule]b4_user_formals[)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &]b4_rhs_value(yynrhs, yyi + 1)[]b4_locations_if([,
                       &]b4_rhs_location(yynrhs, yyi + 1))[]b4_user_args[);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, ]b4_locations_if([yylsp, ])[Rule]b4_user_args[); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !]b4_api_PREFIX[DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !]b4_api_PREFIX[DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH ]b4_stack_depth_init[
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH ]b4_stack_depth_max[
#endif]b4_push_if([[
/* Parser data structure.  */
struct yypstate
  {]b4_declare_parser_state_variables[
    /* Whether this instance has not started parsing yet.
     * If 2, it corresponds to a finished parsing.  */
    int yynew;
  };]b4_pure_if([], [[

/* Whether the only allowed instance of yypstate is allocated.  */
static char yypstate_allocated = 0;]])])[
]b4_lac_if([[

/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return YYENOMEM if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYPTRDIFF_T *yycapacity, YYPTRDIFF_T yyadd,
#if ]b4_api_PREFIX[DEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yy_state_t **yybottom,
                      yy_state_t *yybottom_no_free,
                      yy_state_t **yytop, yy_state_t *yytop_empty)
{
  YYPTRDIFF_T yysize_old =
    *yytop == yytop_empty ? 0 : *yytop - *yybottom + 1;
  YYPTRDIFF_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYPTRDIFF_T yyalloc = 2 * yysize_new;
      yy_state_t *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        YY_CAST (yy_state_t *,
                 YYSTACK_ALLOC (YY_CAST (YYSIZE_T,
                                         yyalloc * YYSIZEOF (*yybottom_new))));
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return YYENOMEM;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;]m4_if(b4_percent_define_get([[parse.lac.memory-trace]]),
                                   [full], [[
      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "%srealloc to %ld%s", yydebug_prefix,
                  YY_CAST (long, yyalloc), yydebug_suffix));
      YY_IGNORE_USELESS_CAST_END]])[
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                                \
do {                                                                    \
  if (!yy_lac_established)                                              \
    {                                                                   \
      YYDPRINTF ((stderr,                                               \
                  "LAC: initial context established for %s\n",          \
                  yysymbol_name (yytoken)));                            \
      yy_lac_established = 1;                                           \
      switch (yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken))    \
        {                                                               \
        case YYENOMEM:                                                  \
          YYNOMEM;                                                      \
        case 1:                                                         \
          goto yyerrlab;                                                \
        }                                                               \
    }                                                                   \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if ]b4_api_PREFIX[DEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      YYDPRINTF ((stderr, "LAC: initial context discarded due to "       \
                  Event "\n"));                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return YYENOMEM if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yy_state_t *yyesa, yy_state_t **yyes,
        YYPTRDIFF_T *yyes_capacity, yy_state_t *yyssp, yysymbol_kind_t yytoken)
{
  yy_state_t *yyes_prev = yyssp;
  yy_state_t *yyesp = yyes_prev;
  /* Reduce until we encounter a shift and thereby accept the token.  */
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yysymbol_name (yytoken)));
  if (yytoken == ]b4_symbol_prefix[YYUNDEF)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[+*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          /* Use the default action.  */
          yyrule = yydefact[+*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          /* Use the action from yytable.  */
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      /* By now we know we have to simulate a reduce.  */
      YYDPRINTF ((stderr, " R%d", yyrule - 1));
      {
        /* Pop the corresponding number of values from the stack.  */
        YYPTRDIFF_T yylen = yyr2[yyrule];
        /* First pop from the LAC stack as many tokens as possible.  */
        if (yyesp != yyes_prev)
          {
            YYPTRDIFF_T yysize = yyesp - *yyes + 1;
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yyesp = yyes_prev;
                yylen -= yysize;
              }
          }
        /* Only afterwards look at the main stack.  */
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      /* Push the resulting state of the reduction.  */
      {
        yy_state_fast_t yystate;
        {
          const int yylhs = yyr1[yyrule] - YYNTOKENS;
          const int yyi = yypgoto[yylhs] + *yyesp;
          yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyesp
                     ? yytable[yyi]
                     : yydefgoto[yylhs]);
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            YY_IGNORE_USELESS_CAST_BEGIN
            *yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if ]b4_api_PREFIX[DEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return YYENOMEM;
              }
            YY_IGNORE_USELESS_CAST_BEGIN
            *++yyesp = YY_CAST (yy_state_t, yystate);
            YY_IGNORE_USELESS_CAST_END
          }
        YYDPRINTF ((stderr, " G%d", yystate));
      }
    }
}]])[

]b4_parse_error_case([simple], [],
[[/* Context of a parse error.  */
typedef struct
{]b4_push_if([[
  yypstate* yyps;]], [[
  yy_state_t *yyssp;]b4_lac_if([[
  yy_state_t *yyesa;
  yy_state_t **yyes;
  YYPTRDIFF_T *yyes_capacity;]])])[
  yysymbol_kind_t yytoken;]b4_locations_if([[
  YYLTYPE *yylloc;]])[
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */]b4_push_if([[
static int
yypstate_expected_tokens (yypstate *yyps,
                          yysymbol_kind_t yyarg[], int yyargn)]], [[
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)]])[
{
  /* Actual size of YYARG. */
  int yycount = 0;
]b4_lac_if([[
  int yyx;
  for (yyx = 0; yyx < YYNTOKENS; ++yyx)
    {
      yysymbol_kind_t yysym = YY_CAST (yysymbol_kind_t, yyx);
      if (yysym != ]b4_symbol(error, kind)[ && yysym != ]b4_symbol_prefix[YYUNDEF)
        switch (yy_lac (]b4_push_if([[yyps->yyesa, &yyps->yyes, &yyps->yyes_capacity, yyps->yyssp, yysym]],
                                    [[yyctx->yyesa, yyctx->yyes, yyctx->yyes_capacity, yyctx->yyssp, yysym]])[))
          {
          case YYENOMEM:
            return YYENOMEM;
          case 1:
            continue;
          default:
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
    }]],
[[  int yyn = yypact@{+*]b4_push_if([yyps], [yyctx])[->yyssp@};
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != ]b4_symbol(error, kind)[
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }]])[
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = ]b4_symbol(empty, kind)[;
  return yycount;
}

]b4_push_if([[
/* Similar to the previous function.  */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  return yypstate_expected_tokens (yyctx->yyps, yyarg, yyargn);
}]])[
]])[

]b4_parse_error_bmatch(
         [custom],
[[/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}

]b4_locations_if([[/* The location of the lookahead of this context.  */
static YYLTYPE *
yypcontext_location (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static YYLTYPE *
yypcontext_location (const yypcontext_t *yyctx)
{
  return yyctx->yylloc;
}]])[

/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx]b4_user_formals[);]],
         [detailed\|verbose],
[[#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

]b4_parse_error_case(
         [verbose],
[[#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif
]])[

static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.]b4_lac_if([[
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.]], [[
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.]])[
  */
  if (yyctx->yytoken != ]b4_symbol(empty, kind)[)
    {
      int yyn;]b4_lac_if([[
      YYDPRINTF ((stderr, "Constructing syntax error message\n"));]])[
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;]b4_lac_if([[
      else if (yyn == 0)
        YYDPRINTF ((stderr, "No expected tokens.\n"));]])[
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.]b4_lac_if([[  In order to see if a particular token T is a
   valid looakhead, invoke yy_lac (YYESA, YYES, YYES_CAPACITY, YYSSP, T).]])[

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store]b4_lac_if([[ or if
   yy_lac returned YYENOMEM]])[.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + ]b4_parse_error_case(
                              [verbose], [[yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]])]],
                              [[yystrlen (yysymbol_name (yyarg[yyi]))]]);[
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {]b4_parse_error_case([verbose], [[
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);]], [[
          yyp = yystpcpy (yyp, yysymbol_name (yyarg[yyi++]));]])[
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
]])[

]b4_yydestruct_define[

]b4_pure_if([], [b4_declare_scanner_communication_variables])[

]b4_push_if([b4_pull_if([[

int
yyparse (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[)
{
  yypstate *yyps = yypstate_new ();
  if (!yyps)
    {]b4_pure_if([b4_locations_if([[
      static YYLTYPE yyloc_default][]b4_yyloc_default[;
      YYLTYPE yylloc = yyloc_default;]])[
      yyerror (]b4_yyerror_args[YY_("memory exhausted"));]], [[
      if (!yypstate_allocated)
        yyerror (]b4_yyerror_args[YY_("memory exhausted"));]])[
      return 2;
    }
  int yystatus = yypull_parse (yyps]b4_user_args[);
  yypstate_delete (yyps);
  return yystatus;
}

int
yypull_parse (yypstate *yyps]b4_user_formals[)
{
  YY_ASSERT (yyps);]b4_pure_if([b4_locations_if([[
  static YYLTYPE yyloc_default][]b4_yyloc_default[;
  YYLTYPE yylloc = yyloc_default;]])])[
  int yystatus;
  do {
]b4_pure_if([[    YYSTYPE yylval;
    int ]])[yychar = ]b4_yylex[;
    yystatus = yypush_parse (yyps]b4_pure_if([[, yychar, &yylval]b4_locations_if([[, &yylloc]])])m4_ifset([b4_parse_param], [, b4_args(b4_parse_param)])[);
  } while (yystatus == YYPUSH_MORE);
  return yystatus;
}]])[

]b4_parse_state_variable_macros([b4_pstate_macro_define])[

/* Initialize the parser data structure.  */
static void
yypstate_clear (yypstate *yyps)
{
  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yyssp = yyss;
  yyvsp = yyvs;]b4_locations_if([[
  yylsp = yyls;]])[

  /* Initialize the state stack, in case yypcontext_expected_tokens is
     called before the first call to yyparse. */
  *yyssp = 0;
  yyps->yynew = 1;
}

/* Initialize the parser data structure.  */
yypstate *
yypstate_new (void)
{
  yypstate *yyps;]b4_pure_if([], [[
  if (yypstate_allocated)
    return YY_NULLPTR;]])[
  yyps = YY_CAST (yypstate *, YYMALLOC (sizeof *yyps));
  if (!yyps)
    return YY_NULLPTR;]b4_pure_if([], [[
  yypstate_allocated = 1;]])[
  yystacksize = YYINITDEPTH;
  yyss = yyssa;
  yyvs = yyvsa;]b4_locations_if([[
  yyls = yylsa;]])[]b4_lac_if([[
  yyes = yyesa;
  yyes_capacity = ]b4_percent_define_get([[parse.lac.es-capacity-initial]])[;
  if (YYMAXDEPTH < yyes_capacity)
    yyes_capacity = YYMAXDEPTH;]])[
  yypstate_clear (yyps);
  return yyps;
}

void
yypstate_delete (yypstate *yyps)
{
  if (yyps)
    {
#ifndef yyoverflow
      /* If the stack was reallocated but the parse did not complete, then the
         stack still needs to be freed.  */
      if (yyss != yyssa)
        YYSTACK_FREE (yyss);
#endif]b4_lac_if([[
      if (yyes != yyesa)
        YYSTACK_FREE (yyes);]])[
      YYFREE (yyps);]b4_pure_if([], [[
      yypstate_allocated = 0;]])[
    }
}
]])[

]b4_push_if([[
/*---------------.
| yypush_parse.  |
`---------------*/

int
yypush_parse (yypstate *yyps]b4_pure_if([[,
              int yypushed_char, YYSTYPE const *yypushed_val]b4_locations_if([[, YYLTYPE *yypushed_loc]])])b4_user_formals[)]],
[[
/*----------.
| yyparse.  |
`----------*/

]m4_ifdef([b4_start_symbols],
[[// Extract data from the parser.
typedef struct
{
  YYSTYPE yyvalue;
  int yynerrs;
} yy_parse_impl_t;

// Run a full parse, using YYCHAR as switching token.
static int
yy_parse_impl (int yychar, yy_parse_impl_t *yyimpl]m4_ifset([b4_parse_param], [, b4_formals(b4_parse_param)])[);

]m4_map([_b4_define_sub_yyparse], m4_defn([b4_start_symbols]))[

int
yyparse (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[)
{
  return yy_parse_impl (]b4_symbol(_b4_first_switching_token, id)[, YY_NULLPTR]m4_ifset([b4_parse_param],
                                                    [[, ]b4_args(b4_parse_param)])[);
}

static int
yy_parse_impl (int yychar, yy_parse_impl_t *yyimpl]m4_ifset([b4_parse_param], [, b4_formals(b4_parse_param)])[)]],
[[int
yyparse (]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param)], [void])[)]])])[
{]b4_pure_if([b4_declare_scanner_communication_variables
])b4_push_if([b4_pure_if([], [[
  int yypushed_char = yychar;
  YYSTYPE yypushed_val = yylval;]b4_locations_if([[
  YYLTYPE yypushed_loc = yylloc;]])
])],
  [b4_declare_parser_state_variables([init])
])b4_lac_if([[
  /* Whether LAC context is established.  A Boolean.  */
  int yy_lac_established = 0;]])[
  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = ]b4_symbol(empty, kind)[;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;]b4_locations_if([[
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];]])[

]b4_parse_error_bmatch([detailed\|verbose],
[[  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;]])[

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N)]b4_locations_if([, yylsp -= (N)])[)

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;]b4_push_if([[

  switch (yyps->yynew)
    {
    case 0:
      yyn = yypact[yystate];
      goto yyread_pushed_token;

    case 2:
      yypstate_clear (yyps);
      break;

    default:
      break;
    }]])[

  YYDPRINTF ((stderr, "Starting parse\n"));

]m4_ifdef([b4_start_symbols], [],
[[  yychar = ]b4_symbol(empty, id)[; /* Cause a token to be read.  */
]])[
]m4_ifdef([b4_initial_action], [
b4_dollar_pushdef([m4_define([b4_dollar_dollar_used])yylval], [], [],
                  [b4_push_if([b4_pure_if([*])yypushed_loc], [yylloc])])dnl
b4_user_initial_action
b4_dollar_popdef[]dnl
m4_ifdef([b4_dollar_dollar_used],[[  yyvsp[0] = yylval;
]])])dnl
b4_locations_if([[  yylsp[0] = ]b4_push_if([b4_pure_if([*])yypushed_loc], [yylloc])[;
]])dnl
[  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;]b4_locations_if([
        YYLTYPE *yyls1 = yyls;])[

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),]b4_locations_if([
                    &yyls1, yysize * YYSIZEOF (*yylsp),])[
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;]b4_locations_if([
        yyls = yyls1;])[
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);]b4_locations_if([
        YYSTACK_RELOCATE (yyls_alloc, yyls);])[
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;]b4_locations_if([
      yylsp = yyls + yysize - 1;])[

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

]m4_ifdef([b4_start_symbols], [], [[
  if (yystate == YYFINAL)
    YYACCEPT;]])[

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ]b4_symbol(empty, id)[)
    {]b4_push_if([[
      if (!yyps->yynew)
        {]b4_use_push_for_pull_if([], [[
          YYDPRINTF ((stderr, "Return for a new token:\n"));]])[
          yyresult = YYPUSH_MORE;
          goto yypushreturn;
        }
      yyps->yynew = 0;]b4_pure_if([], [[
      /* Restoring the pushed token is only necessary for the first
         yypush_parse invocation since subsequent invocations don't overwrite
         it before jumping to yyread_pushed_token.  */
      yychar = yypushed_char;
      yylval = yypushed_val;]b4_locations_if([[
      yylloc = yypushed_loc;]])])[
yyread_pushed_token:]])[
      YYDPRINTF ((stderr, "Reading a token\n"));]b4_push_if([b4_pure_if([[
      yychar = yypushed_char;
      if (yypushed_val)
        yylval = *yypushed_val;]b4_locations_if([[
      if (yypushed_loc)
        yylloc = *yypushed_loc;]])])], [[
      yychar = ]b4_yylex[;]])[
    }

  if (yychar <= ]b4_symbol(eof, [id])[)
    {
      yychar = ]b4_symbol(eof, [id])[;
      yytoken = ]b4_symbol(eof, [kind])[;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ]b4_symbol(error, [id])[)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ]b4_symbol(undef, [id])[;
      yytoken = ]b4_symbol(error, [kind])[;]b4_locations_if([[
      yyerror_range[1] = yylloc;]])[
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)]b4_lac_if([[
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }]], [[
    goto yydefault;]])[
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;]b4_lac_if([[
      YY_LAC_ESTABLISH;]])[
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END]b4_locations_if([
  *++yylsp = yylloc;])[

  /* Discard the shifted token.  */
  yychar = ]b4_symbol(empty, id)[;]b4_lac_if([[
  YY_LAC_DISCARD ("shift");]])[
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

]b4_locations_if(
[[  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;]])[
  YY_REDUCE_PRINT (yyn);]b4_lac_if([[
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
]b4_user_actions[
        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
  }]], [[
  switch (yyn)
    {
]b4_user_actions[
      default: break;
    }]])[
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;]b4_locations_if([
  *++yylsp = yyloc;])[

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ]b4_symbol(empty, id)[ ? ]b4_symbol(empty, kind)[ : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
]b4_parse_error_case(
         [custom],
[[      {
        yypcontext_t yyctx
          = {]b4_push_if([[yyps]], [[yyssp]b4_lac_if([[, yyesa, &yyes, &yyes_capacity]])])[, yytoken]b4_locations_if([[, &yylloc]])[};]b4_lac_if([[
        if (yychar != ]b4_symbol(empty, id)[)
          YY_LAC_ESTABLISH;]])[
        if (yyreport_syntax_error (&yyctx]m4_ifset([b4_parse_param],
                                   [[, ]b4_args(b4_parse_param)])[) == 2)
          YYNOMEM;
      }]],
         [simple],
[[      yyerror (]b4_yyerror_args[YY_("syntax error"));]],
[[      {
        yypcontext_t yyctx
          = {]b4_push_if([[yyps]], [[yyssp]b4_lac_if([[, yyesa, &yyes, &yyes_capacity]])])[, yytoken]b4_locations_if([[, &yylloc]])[};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;]b4_lac_if([[
        if (yychar != ]b4_symbol(empty, id)[)
          YY_LAC_ESTABLISH;]])[
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (]b4_yyerror_args[yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }]])[
    }
]b4_locations_if([[
  yyerror_range[1] = yylloc;]])[
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= ]b4_symbol(eof, [id])[)
        {
          /* Return failure if at end of input.  */
          if (yychar == ]b4_symbol(eof, [id])[)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval]b4_locations_if([, &yylloc])[]b4_user_args[);
          yychar = ]b4_symbol(empty, id)[;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += ]b4_symbol(error, kind)[;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == ]b4_symbol(error, kind)[)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

]b4_locations_if([[      yyerror_range[1] = *yylsp;]])[
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp]b4_locations_if([, yylsp])[]b4_user_args[);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }]b4_lac_if([[

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");]])[

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
]b4_locations_if([[
  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);]])[

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (]b4_yyerror_args[YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ]b4_symbol(empty, id)[)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval]b4_locations_if([, &yylloc])[]b4_user_args[);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp]b4_locations_if([, yylsp])[]b4_user_args[);
      YYPOPSTACK (1);
    }]b4_push_if([[
  yyps->yynew = 2;
  goto yypushreturn;


/*-------------------------.
| yypushreturn -- return.  |
`-------------------------*/
yypushreturn:]], [[
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif]b4_lac_if([[
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);]])])[
]b4_parse_error_bmatch([detailed\|verbose],
[[  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);]])[]m4_ifdef([b4_start_symbols], [[
  if (yyimpl)
    yyimpl->yynerrs = yynerrs;]])[
  return yyresult;
}
]b4_push_if([b4_parse_state_variable_macros([b4_macro_undef])])[
]b4_percent_code_get([[epilogue]])[]dnl
b4_epilogue[]dnl
b4_output_end
