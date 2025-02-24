#C++ GLR skeleton for Bison

#Copyright(C) 2002 - 2015, 2018 - 2021 Free Software Foundation, Inc.

#This program is free software : you can redistribute it and / or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.If not, see < https:  // www.gnu.org/licenses/>.

m4_include(b4_skeletonsdir/[c++.m4])

#api.value.type = variant is valid.
m4_define([b4_value_type_setup_variant])

#b4_tname_if(TNAME - NEEDED, TNAME - NOT - NEEDED)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
m4_define([b4_tname_if],
[m4_case(b4_percent_define_get([[parse.error]]),
         [verbose],         [$1],
         [b4_token_table_if([$1],
                            [$2])])])

b4_bison_locations_if([
   m4_define([b4_location_constructors])
   m4_include(b4_skeletonsdir/[location.cc])])
b4_variant_if([m4_include(b4_skeletonsdir/[variant.hh])])

m4_define([b4_parser_class],
          [b4_percent_define_get([[api.parser.class]])])

]m4_define([b4_define_symbol_kind],
[m4_format([#define %-15s %s],
           b4_symbol($][1, kind_base),
           b4_namespace_ref[::]b4_parser_class[::symbol_kind::]b4_symbol($1, kind_base))
])

#b4_integral_parser_table_define(TABLE - NAME, CONTENT, COMMENT)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Define "yy<TABLE-NAME>" whose contents is CONTENT.Does not use "static",
#should be in unnamed namespace.
m4_define([b4_integral_parser_table_define],
[m4_ifvaln([$3], [  b4_comment([$3])])dnl
  const b4_int_type_for([$2]) yy$1[[]] =
  {
  $2
  };dnl
])


## ---------------- ##
## Default values.  ##
## ---------------- ##

#Stack parameters.
m4_define_default([b4_stack_depth_max], [10000])
m4_define_default([b4_stack_depth_init],  [200])



## ------------ ##
## Interfaces.  ##
## ------------ ##

#b4_user_formals
#-- -- -- -- -- -- -- -
#The possible parse - params formal arguments preceded by a comma.
#
#This is not shared with yacc.c in c.m4 because GLR relies on ISO C
#formal argument declarations.
m4_define([b4_user_formals],
[m4_ifset([b4_parse_param], [, b4_formals(b4_parse_param)])])

#b4_symbol_kind(NUM)
#-- -- -- -- -- -- -- -- -- -
m4_define([b4_symbol_kind],
[symbol_kind::b4_symbol_kind_base($@)])


## ----------------- ##
## Semantic Values.  ##
## ----------------- ##

#b4_lhs_value(SYMBOL - NUM, [TYPE])
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#See README.
m4_define([b4_lhs_value],
[b4_symbol_value([(*yyvalp)], [$1], [$2])])

#b4_rhs_data(RULE - LENGTH, POS)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#See README.
m4_define([b4_rhs_data],
[(static_cast<glr_stack_item const *>(yyvsp))@{YYFILL (b4_subtract([$2], [$1]))@}.getState()])

#b4_rhs_value(RULE - LENGTH, POS, SYMBOL - NUM, [TYPE])
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Expansion of $$ or $ < TYPE> $, for symbol SYMBOL - NUM.
m4_define([b4_rhs_value],
[b4_symbol_value([b4_rhs_data([$1], [$2]).value ()], [$3], [$4])])



## ----------- ##
## Locations.  ##
## ----------- ##

#b4_lhs_location()
#-- -- -- -- -- -- -- -- -
#Expansion of @$.
m4_define([b4_lhs_location],
[(*yylocp)])

#b4_rhs_location(RULE - LENGTH, NUM)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Expansion of @NUM, where the current rule has RULE - LENGTH symbols
#on RHS.
m4_define([b4_rhs_location],
[(b4_rhs_data([$1], [$2]).yyloc)])

#b4_symbol_action(SYMBOL - NUM, KIND)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Run the action KIND(destructor or printer) for SYMBOL - NUM.
#Same as in C, but using references instead of pointers.
#
#Currently we need two different b4_symbol_action : once for the
#self - contained symbols, and another time for yy_destroy_ and
#yy_symbol_value_print_, which don't use genuine symbols yet.
m4_define([b4_symbol_action],
[b4_symbol_if([$1], [has_$2],
[m4_pushdef([b4_symbol_value], m4_defn([b4_symbol_value_template]))[]dnl
b4_dollar_pushdef([yysym.value],
                  [$1],
                  [],
                  [yysym.location])dnl
      _b4_symbol_case([$1])[]dnl
b4_syncline([b4_symbol([$1], [$2_line])], [b4_symbol([$1], [$2_file])])dnl
        b4_symbol([$1], [$2])
b4_syncline([@oline@], [@ofile@])dnl
        break;

m4_popdef([b4_symbol_value])[]dnl
b4_dollar_popdef[]dnl
])])

#b4_symbol_action_for_yyval(SYMBOL - NUM, KIND)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Run the action KIND(destructor or printer) for SYMBOL - NUM.
#Same as in C, but using references instead of pointers.
m4_define([b4_symbol_action_for_yyval],
[b4_symbol_if([$1], [has_$2],
[b4_dollar_pushdef([yyval],
                   [$1],
                   [],
                   [yyloc])dnl
      _b4_symbol_case([$1])[]dnl
b4_syncline([b4_symbol([$1], [$2_line])], [b4_symbol([$1], [$2_file])])dnl
        b4_symbol([$1], [$2])
b4_syncline([@oline@], [@ofile@])dnl
        break;

b4_dollar_popdef[]dnl
])])

#b4_call_merger(MERGER - NUM, MERGER - NAME, SYMBOL - SUM)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
m4_define([b4_call_merger],
[b4_case([$1],
         [    b4_symbol_if([$3], [has_type],
                           [b4_variant_if([yy0.as< b4_symbol($3, type) > () = $2 (yy0.as< b4_symbol($3, type) >(), yy1.as< b4_symbol($3, type) >());],
                                          [yy0.b4_symbol($3, slot) = $2 (yy0, yy1);])],
                           [yy0 = $2 (yy0, yy1);])])])

#b4_yylex
#-- -- -- --
#Call yylex.
m4_define([b4_yylex],
[b4_token_ctor_if(
[b4_function_call([yylex],
                  [symbol_type], m4_ifdef([b4_lex_param], b4_lex_param))],
[b4_function_call([yylex], [int],
                  [[value_type *], [&this->yyla.value]][]dnl
b4_locations_if([, [[location_type *], [&this->yyla.location]]])dnl
m4_ifdef([b4_lex_param], [, ]b4_lex_param))])])

#b4_shared_declarations(hh | cc)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Declaration that might either go into the header(if --header, $1 = hh)
# or in the implementation file.
m4_define([b4_shared_declarations],
[b4_percent_code_get([[requires]])[
#include <stdint.h>
#include <algorithm>
#include <cstddef>  // ptrdiff_t
#include <cstring>  // memcpy
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

]b4_cxx_portability[
]m4_ifdef([b4_location_include],
          [[# include ]b4_location_include])[
]b4_variant_if([b4_variant_includes])[

]b4_YYDEBUG_define[

]b4_namespace_open[

]b4_bison_locations_if([m4_ifndef([b4_location_file],
                                  [b4_location_define])])[

  /// A Bison parser.
  class ]b4_parser_class[
  {
  public:
]b4_public_types_declare[
]b4_symbol_type_define[

    // FIXME: should be private eventually.
    class glr_stack;
    class glr_state;

    /// Build a parser object.
    ]b4_parser_class[ (]b4_parse_param_decl[);
    ~]b4_parser_class[ ();

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    int parse ();

#if] b4_api_PREFIX[DEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    using debug_level_type = int;
    /// The current debugging level.
    debug_level_type debug_level () const;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.]b4_locations_if([[
    /// \param loc    where the syntax error is found.]])[
    /// \param msg    a description of the syntax error.
    void error (]b4_locations_if([[const location_type& loc, ]])[const std::string& msg);

]b4_parse_error_bmatch(
[custom\|detailed],
[[    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *symbol_name (symbol_kind_type yysymbol);]],
[simple],
[[#if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *symbol_name (symbol_kind_type yysymbol);
#endif  // #if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
]],
[verbose],
[[    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);]])[

]b4_token_constructor_define[
]b4_parse_error_bmatch([custom\|detailed\|verbose], [[
    class context
    {
      public:
        context(glr_stack & yystack, const symbol_type &yyla);
        const symbol_type &lookahead() const YY_NOEXCEPT
        {
            return yyla_;
        }
        symbol_kind_type token() const YY_NOEXCEPT
        {
            return yyla_.kind();
        }]b4_locations_if([[
      const location_type& location () const YY_NOEXCEPT {
            return yyla_.location; }
]])[
      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      glr_stack& yystack_;
      const symbol_type& yyla_;
    };
]])[
#if] b4_api_PREFIX[DEBUG
  public:
    /// \brief Report a symbol value on the debug stream.
    /// \param yykind   The symbol kind.
    /// \param yyval    Its semantic value.]b4_locations_if([[
    /// \param yyloc    Its location.]])[
    void yy_symbol_value_print_ (symbol_kind_type yykind,
                                 const value_type& yyval]b4_locations_if([[,
                                 const location_type& yyloc]])[) const;
    /// \brief Report a symbol on the debug stream.
    /// \param yykind   The symbol kind.
    /// \param yyval    Its semantic value.]b4_locations_if([[
    /// \param yyloc    Its location.]])[
    void yy_symbol_print_ (symbol_kind_type yykind,
                           const value_type& yyval]b4_locations_if([[,
                           const location_type& yyloc]])[) const;
  private:
    /// Debug stream.
    std::ostream* yycdebug_;
#endif

]b4_parse_error_bmatch(
[custom], [[
  private:
    /// Report a syntax error
    /// \param yyctx     the context in which the error occurred.
    void report_syntax_error (const context& yyctx) const;]],
[detailed\|verbose], [[
  private:
    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;]])[

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_kind_type enum.
    static symbol_kind_type yytranslate_ (int t) YY_NOEXCEPT;

]b4_parse_error_bmatch(
[simple],
[[#if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
    /// For a symbol, its name in clear.
    static const char* const yytname_[];
#endif  // #if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
]],
[verbose],
[[    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];
]])[

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yykind    The symbol kind.
    void yy_destroy_ (const char* yymsg, symbol_kind_type yykind,
                      value_type& yyval]b4_locations_if([[,
                      location_type& yyloc]])[);

]b4_parse_param_vars[
    // Needs access to yy_destroy_, report_syntax_error, etc.
    friend glr_stack;
  };

]b4_token_ctor_if([b4_yytranslate_define([$1])[
]b4_public_types_define([$1])])[
]b4_namespace_close[

]b4_percent_code_get([[provides]])[
]])[


## -------------- ##
## Output files.  ##
## -------------- ##

#-- -- -- -- -- -- - #
#Header file.#
#-- -- -- -- -- -- - #

]b4_header_if([[
]b4_output_begin([b4_spec_header_file])[
]b4_copyright([Skeleton interface for Bison GLR parsers in C++],
             [2002-2015, 2018-2021])[
// C++ GLR parser skeleton written by Valentin Tolmer.

]b4_disclaimer[
]b4_cpp_guard_open([b4_spec_mapped_header_file])[
]b4_shared_declarations([hh])[
]b4_cpp_guard_close([b4_spec_mapped_header_file])[
]b4_output_end])[

#-- -- -- -- -- -- -- -- -- -- - #
#Implementation file.#
#-- -- -- -- -- -- -- -- -- -- - #

]b4_output_begin([b4_parser_file_name])[
]b4_copyright([Skeleton implementation for Bison GLR parsers in C],
              [2002-2015, 2018-2021])[
// C++ GLR parser skeleton written by Valentin Tolmer.

]b4_disclaimer[
]b4_identification[

]b4_percent_code_get([[top]])[
]m4_if(b4_prefix, [yy], [],
[[/* Substitute the variable and function names.  */
#define yyparse ]b4_prefix[parse
#define yylex   ]b4_prefix[lex
#define yyerror ]b4_prefix[error
#define yydebug ]b4_prefix[debug]])[

]b4_user_pre_prologue[

]b4_null_define[

]b4_header_if([[#include "@basename(]b4_spec_header_file[@)"]],
              [b4_shared_declarations([cc])])[

namespace
{
    /* Default (constant) value used for initialization for null
       right-hand sides.  Unlike the standard yacc.c template, here we set
       the default value of $$ to a zeroed-out value.  Since the default
       value is undefined, this behavior is technically correct.  */
  ]b4_namespace_ref[::]b4_parser_class[::value_type yyval_default;
}

]b4_user_post_prologue[
]b4_percent_code_get[

#include <cstdio>
#include <cstdlib>

#ifndef YY_
#    if defined YYENABLE_NLS && YYENABLE_NLS
#        if ENABLE_NLS
#            include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#            define YY_(Msgid) dgettext("bison-runtime", Msgid)
#        endif
#    endif
#    ifndef YY_
#        define YY_(Msgid) Msgid
#    endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
#    if defined __GNUC__ && !defined __EXCEPTIONS
#        define YY_EXCEPTIONS 0
#    else
#        define YY_EXCEPTIONS 1
#    endif
#endif

#ifndef YYFREE
#    define YYFREE free
#endif
#ifndef YYMALLOC
#    define YYMALLOC malloc
#endif

#ifndef YYSETJMP
#    include <setjmp.h>
#    define YYJMP_BUF jmp_buf
#    define YYSETJMP(Env) setjmp(Env)
/* Pacify Clang and ICC.  */
#    define YYLONGJMP(Env, Val) \
        do                      \
        {                       \
            longjmp(Env, Val);  \
            YYASSERT(0);        \
        } while (false)
#endif

]b4_attribute_define([noreturn])[

#if defined __GNUC__ && !defined __ICC && 6 <= __GNUC__
#    define YY_IGNORE_NULL_DEREFERENCE_BEGIN \
        _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wnull-dereference\"")
#    define YY_IGNORE_NULL_DEREFERENCE_END _Pragma("GCC diagnostic pop")
#else
#    define YY_IGNORE_NULL_DEREFERENCE_BEGIN
#    define YY_IGNORE_NULL_DEREFERENCE_END
#endif

]b4_null_define[
]b4_cast_define[

// FIXME: Use the same conventions as lalr1.cc.
]b4_parse_assert_if[
#ifndef YYASSERT
#    define YYASSERT(Condition) ((void)((Condition) || (abort(), 0)))
#endif

#ifdef YYDEBUG
#    define YYDASSERT(Condition) YYASSERT(Condition)
#else
#    define YYDASSERT(Condition)
#endif

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
/* YYMAXRHS -- Maximum number of symbols on right-hand side of rule.  */
#define YYMAXRHS ]b4_r2_max[
/* YYMAXLEFT -- Maximum number of symbols to the left of a handle
   accessed by $0, $-1, etc., in any rule.  */
#define YYMAXLEFT ]b4_max_left_semantic_context[

namespace
{
#if] b4_api_PREFIX[DEBUG
    /* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
  const ]b4_int_type_for([b4_rline])[ yyrline[] =
  {
  ]b4_rline[
  };
#endif

#define YYPACT_NINF ]b4_pact_ninf[
#define YYTABLE_NINF ]b4_table_ninf[

]b4_parser_tables_define[

  /* YYDPREC[RULE-NUM] -- Dynamic precedence of rule #RULE-NUM (0 if none).  */
  const ]b4_int_type_for([b4_dprec])[ yydprec[] =
  {
  ]b4_dprec[
  };

  /* YYMERGER[RULE-NUM] -- Index of merging function for rule #RULE-NUM.  */
  const ]b4_int_type_for([b4_merger])[ yymerger[] =
  {
  ]b4_merger[
  };

  /* YYIMMEDIATE[RULE-NUM] -- True iff rule #RULE-NUM is not to be deferred, as
     in the case of predicates.  */
  const bool yyimmediate[] =
  {
  ]b4_immediate[
  };

  /* YYCONFLP[YYPACT[STATE-NUM]] -- Pointer into YYCONFL of start of
     list of conflicting reductions corresponding to action entry for
     state STATE-NUM in yytable.  0 means no conflicts.  The list in
     yyconfl is terminated by a rule number of 0.  */
  const ]b4_int_type_for([b4_conflict_list_heads])[ yyconflp[] =
  {
  ]b4_conflict_list_heads[
  };

  /* YYCONFL[I] -- lists of conflicting rule numbers, each terminated by
     0, pointed into by YYCONFLP.  */
  ]dnl Do not use b4_int_type_for here, since there are places where
  dnl pointers onto yyconfl are taken, whose type is "short*".
  dnl We probably ought to introduce a type for confl.
  [const short yyconfl[] =
  {
  ]b4_conflicting_rules[
  };
} // namespace

/* Error token number */
#define YYTERROR 1

]b4_locations_if([[
]b4_yylloc_default_define[
#define YYRHSLOC(Rhs, K) ((Rhs)[K].getState().yyloc)
]])[

enum YYRESULTTAG { yyok, yyaccept, yyabort, yyerr };

#define YYCHK(YYE)                    \
    do                                \
    {                                 \
        YYRESULTTAG yychk_flag = YYE; \
        if (yychk_flag != yyok)       \
            return yychk_flag;        \
    } while (false)

#if] b4_api_PREFIX[DEBUG

#    define YYCDEBUG  \
        if (!yydebug) \
        {}            \
        else          \
            std::cerr

#    define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                       \
        do                                                                      \
        {                                                                       \
            if (yydebug)                                                        \
            {                                                                   \
                std::cerr << Title << ' ';                                      \
        yyparser.yy_symbol_print_ (Kind, Value]b4_locations_if([, Location])[); \
                std::cerr << '\n';                                              \
            }                                                                   \
        } while (false)

#    define YY_REDUCE_PRINT(Args)                  \
        do                                         \
        {                                          \
            if (yydebug)                           \
                yystateStack.yy_reduce_print Args; \
        } while (false)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;

namespace
{
  using glr_stack = ]b4_namespace_ref[::]b4_parser_class[::glr_stack;
  using glr_state = ]b4_namespace_ref[::]b4_parser_class[::glr_state;

  void yypstack (const glr_stack& yystack, size_t yyk)
    YY_ATTRIBUTE_UNUSED;
  void yypdumpstack (const glr_stack& yystack)
    YY_ATTRIBUTE_UNUSED;
}

#else /* !]b4_api_PREFIX[DEBUG */

#    define YYCDEBUG \
        if (true)    \
        {}           \
        else         \
            std::cerr
#    define YY_SYMBOL_PRINT(Title, Kind, Value, Location) \
        {}
#    define YY_REDUCE_PRINT(Args) \
        {}

#endif /* !]b4_api_PREFIX[DEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
#    define YYINITDEPTH ]b4_stack_depth_init[
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYMAXDEPTH * sizeof (GLRStackItem)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#    define YYMAXDEPTH ]b4_stack_depth_max[
#endif

/* Minimum number of free items on the stack allowed after an
   allocation.  This is to allow allocation and initialization
   to be completed by functions that call yyexpandGLRStack before the
   stack is expanded, thus insuring that all necessary pointers get
   properly redirected to new data.  */
#define YYHEADROOM 2

#ifndef YYSTACKEXPANDABLE
#    define YYSTACKEXPANDABLE 1
#endif

namespace
{
    template <typename Parameter>
    class strong_index_alias
    {
      public:
        static strong_index_alias create(std::ptrdiff_t value)
        {
            strong_index_alias result;
            result.value_ = value;
            return result;
        }

        std::ptrdiff_t const &get() const { return value_; }

        size_t uget() const { return static_cast<size_t>(value_); }

        strong_index_alias operator+(std::ptrdiff_t other) const
        {
            return strong_index_alias(get() + other);
        }

        void operator+=(std::ptrdiff_t other) { value_ += other; }

        strong_index_alias operator-(std::ptrdiff_t other)
        {
            return strong_index_alias(get() - other);
        }

        void operator-=(std::ptrdiff_t other) { value_ -= other; }

        size_t operator-(strong_index_alias other)
        {
            return strong_index_alias(get() - other.get());
        }

        strong_index_alias &operator++()
        {
            ++value_;
            return *this;
        }

        bool isValid() const { return value_ != INVALID_INDEX; }

        void setInvalid() { value_ = INVALID_INDEX; }

        bool operator==(strong_index_alias other) { return get() == other.get(); }

        bool operator!=(strong_index_alias other) { return get() != other.get(); }

        bool operator<(strong_index_alias other) { return get() < other.get(); }

      private:
        static const std::ptrdiff_t INVALID_INDEX;

        // WARNING: 0-initialized.
        std::ptrdiff_t value_;
    };  // class strong_index_alias

    template <typename T>
    const std::ptrdiff_t strong_index_alias<T>::INVALID_INDEX =
        std::numeric_limits<std::ptrdiff_t>::max();

    using state_set_index = strong_index_alias<struct glr_state_set_tag>;

    state_set_index create_state_set_index(std::ptrdiff_t value)
    {
        return state_set_index::create(value);
    }

    /** State numbers, as in LALR(1) machine */
    using state_num = int;

    /** Rule numbers, as in LALR(1) machine */
    using rule_num = int;

  using parser_type = ]b4_namespace_ref[::]b4_parser_class[;
  using glr_state = parser_type::glr_state;
  using symbol_kind = parser_type::symbol_kind;
  using symbol_kind_type = parser_type::symbol_kind_type;
  using symbol_type = parser_type::symbol_type;
  using value_type = parser_type::value_type;]b4_locations_if([[
  using location_type = parser_type::location_type;]])[

  // Forward declarations.
  class glr_stack_item;
  class semantic_option;
} // namespace

namespace
{
    /** Accessing symbol of state YYSTATE.  */
    inline symbol_kind_type yy_accessing_symbol(state_num yystate)
    {
        return YY_CAST(symbol_kind_type, yystos[yystate]);
    }

    /** Left-hand-side symbol for rule #YYRULE.  */
    inline symbol_kind_type yylhsNonterm(rule_num yyrule)
    {
        return static_cast<symbol_kind_type>(yyr1[yyrule]);
    }

    /** Number of symbols composing the right hand side of rule #RULE.  */
    inline int yyrhsLength(rule_num yyrule)
    {
        return yyr2[yyrule];
    }
}

namespace ]b4_namespace_ref[
{
  class ]b4_parser_class[::glr_state
  {
    public:
    glr_state ()
      : yyresolved (false)
      , yylrState (0)
      , yyposn (0)
      , yypred (0)
      , yyfirstVal (0)]b4_locations_if([[
      , yyloc ()]])[]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {}

    /// Build with a semantic value.
    glr_state (state_num lrState, size_t posn, const value_type& val]b4_locations_if([[, const location_type& loc]])[)
      : yyresolved (true)
      , yylrState (lrState)
      , yyposn (posn)
      , yypred (0)
      , yyval (]b4_variant_if([], [[val]])[)]b4_locations_if([[
      , yyloc (loc)]])[]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {]b4_variant_if([[
      ]b4_symbol_variant([yy_accessing_symbol (lrState)],
                         [yyval], [copy], [val])])[}

    /// Build with a semantic option.
    glr_state (state_num lrState, size_t posn)
      : yyresolved (false)
      , yylrState (lrState)
      , yyposn (posn)
      , yypred (0)
      , yyfirstVal (0)]b4_locations_if([[
      , yyloc ()]])[]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {}

    glr_state (const glr_state& other)
      : yyresolved (other.yyresolved)
      , yylrState (other.yylrState)
      , yyposn (other.yyposn)
      , yypred (0)]b4_locations_if([[
      , yyloc (other.yyloc)]])[]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {
            setPred(other.pred());
            if (other.yyresolved)]b4_variant_if([[
        {
                    new (&yyval) value_type();
          ]b4_symbol_variant([yy_accessing_symbol (other.yylrState)],
                             [yyval], [copy], [other.value ()])[
        }]], [[
        new (&yyval) value_type (other.value ());]])[
      else
        {
                    yyfirstVal = 0;
                    setFirstVal(other.firstVal());
        }]b4_parse_assert_if([[
      check_();]])[
    }

    ~glr_state ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      if (yyresolved)
        {]b4_variant_if([[
          symbol_kind_type yykind = yy_accessing_symbol (yylrState);
          // FIXME: User destructors.
          // Value type destructor.
          ]b4_symbol_variant([[yykind]], [[yyval]], [[template destroy]])])[
          yyval.~value_type ();
        }]b4_parse_assert_if([[
      magic_ = 0;]])[
    }

    glr_state& operator= (const glr_state& other)
    {]b4_parse_assert_if([[
      check_ ();
      other.check_ ();]])[
      if (!yyresolved && other.yyresolved)
        new (&yyval) value_type;
      yyresolved = other.yyresolved;
      yylrState = other.yylrState;
      yyposn = other.yyposn;
      setPred (other.pred ());
      if (other.yyresolved)]b4_variant_if([[
        ]b4_symbol_variant([yy_accessing_symbol (other.yylrState)],
                           [yyval], [copy], [other.value ()])], [[
        value () = other.value ();]])[
      else
        setFirstVal (other.firstVal ());]b4_locations_if([[
      yyloc = other.yyloc;]])[
      return *this;
    }

    /** Type tag for the semantic value.  If true, yyval applies, otherwise
     *  yyfirstVal applies.  */
    bool yyresolved;
    /** Number of corresponding LALR(1) machine state.  */
    state_num yylrState;
    /** Source position of the last token produced by my symbol */
    size_t yyposn;

    /// Only call pred() and setPred() on objects in yyitems, not temporaries.
    glr_state* pred ();
    const glr_state* pred () const;
    void setPred (const glr_state* state);

    /// Only call firstVal() and setFirstVal() on objects in yyitems, not
    /// temporaries.
    semantic_option* firstVal ();
    const semantic_option* firstVal () const;
    void setFirstVal (const semantic_option* option);

    value_type& value ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      return yyval;
    }

    const value_type& value () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      return yyval;
    }

    void
    destroy (char const *yymsg, ]b4_namespace_ref[::]b4_parser_class[& yyparser);

        /* DEBUGGING ONLY */
#if] b4_api_PREFIX[DEBUG
    void yy_yypstack () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      if (pred () != YY_NULLPTR)
        {
                pred()->yy_yypstack();
                std::cerr << " -> ";
        }
      std::cerr << yylrState << "@@" << yyposn;
    }
#endif

    std::ptrdiff_t indexIn (const glr_stack_item* array) const YY_ATTRIBUTE_UNUSED;

    glr_stack_item* asItem ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      return asItem(this);
    }

    const glr_stack_item* asItem () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      return asItem (this);
    }

  private:
    template <typename T>
    static const glr_stack_item* asItem (const T* state)
    {
            return reinterpret_cast<const glr_stack_item *>(state);
    }
    template <typename T>
    static glr_stack_item* asItem (T* state)
    {
            return reinterpret_cast<glr_stack_item *>(state);
    }
    static const char *as_pointer_ (const glr_state *state)
    {
            return reinterpret_cast<const char *>(state);
    }
    static char *as_pointer_ (glr_state *state)
    {
            return reinterpret_cast<char *>(state);
    }
    /** Preceding state in this stack */
    std::ptrdiff_t yypred;
    union {
            /** First in a chain of alternative reductions producing the
             *  nonterminal corresponding to this state, threaded through
             *  yyfirstVal.  Value "0" means empty.  */
            std::ptrdiff_t yyfirstVal;
            /** Semantic value for this state.  */
            value_type yyval;
    };]b4_locations_if([[
   // FIXME: Why public?
   public:
    /** Source location for this state.  */
    location_type yyloc;]])[

]b4_parse_assert_if([[
  public:
    // Check invariants.
    void check_ () const
    {
            YY_IGNORE_NULL_DEREFERENCE_BEGIN
            YYASSERT(this->magic_ == MAGIC);
            YY_IGNORE_NULL_DEREFERENCE_END
    }

    // A magic number to check our pointer arithmetic is sane.
    enum { MAGIC = 713705 };
    unsigned int magic_;]])[
  };  // class ]b4_parser_class[::glr_state
} // namespace ]b4_namespace_ref[


namespace
{
    /** A stack of GLRState representing the different heads during
     * nondeterministic evaluation. */
    class glr_state_set
    {
      public:
        /** Initialize YYSET to a singleton set containing an empty stack.  */
        glr_state_set() : yylastDeleted(YY_NULLPTR)
        {
            yystates.push_back(YY_NULLPTR);
            yylookaheadNeeds.push_back(false);
        }

        // Behave like a vector of states.
        glr_state *&operator[](state_set_index index) { return yystates[index.uget()]; }

        glr_state *operator[](state_set_index index) const { return yystates[index.uget()]; }

        size_t size() const { return yystates.size(); }

        std::vector<glr_state *>::iterator begin() { return yystates.begin(); }

        std::vector<glr_state *>::iterator end() { return yystates.end(); }

        bool lookaheadNeeds(state_set_index index) const { return yylookaheadNeeds[index.uget()]; }

        bool setLookaheadNeeds(state_set_index index, bool value)
        {
            return yylookaheadNeeds[index.uget()] = value;
        }

        /** Invalidate stack #YYK.  */
        void yymarkStackDeleted(state_set_index yyk)
        {
            size_t k = yyk.uget();
            if (yystates[k] != YY_NULLPTR)
                yylastDeleted = yystates[k];
            yystates[k] = YY_NULLPTR;
        }

        /** Undelete the last stack in *this that was marked as deleted.  Can
            only be done once after a deletion, and only when all other stacks have
            been deleted.  */
        void yyundeleteLastStack()
        {
            if (yylastDeleted == YY_NULLPTR || !yystates.empty())
                return;
            yystates.push_back(yylastDeleted);
            YYCDEBUG << "Restoring last deleted stack as stack #0.\n";
            clearLastDeleted();
        }

        /** Remove the dead stacks (yystates[i] == YY_NULLPTR) and shift the later
         * ones.  */
        void yyremoveDeletes()
        {
            size_t newsize = yystates.size();
            /* j is the number of live stacks we have seen.  */
            for (size_t i = 0, j = 0; j < newsize; ++i)
            {
                if (yystates[i] == YY_NULLPTR)
                {
                    if (i == j)
                    {
                        YYCDEBUG << "Removing dead stacks.\n";
                    }
                    newsize -= 1;
                }
                else
                {
                    yystates[j] = yystates[i];
                    /* In the current implementation, it's unnecessary to copy
                       yylookaheadNeeds[i] since, after
                       yyremoveDeletes returns, the parser immediately either enters
                       deterministic operation or shifts a token.  However, it doesn't
                       hurt, and the code might evolve to need it.  */
                    yylookaheadNeeds[j] = yylookaheadNeeds[i];
                    if (j != i)
                    {
                        YYCDEBUG << "Rename stack " << i << " -> " << j << ".\n";
                    }
                    j += 1;
                }
            }
            yystates.resize(newsize);
            yylookaheadNeeds.resize(newsize);
        }

        state_set_index yysplitStack(state_set_index yyk)
        {
            const size_t k = yyk.uget();
            yystates.push_back(yystates[k]);
            yylookaheadNeeds.push_back(yylookaheadNeeds[k]);
            return create_state_set_index(static_cast<std::ptrdiff_t>(yystates.size() - 1));
        }

        void clearLastDeleted() { yylastDeleted = YY_NULLPTR; }

      private:
        std::vector<glr_state *> yystates;
        /** During nondeterministic operation, yylookaheadNeeds tracks which
         *  stacks have actually needed the current lookahead.  During deterministic
         *  operation, yylookaheadNeeds[0] is not maintained since it would merely
         *  duplicate !yyla.empty ().  */
        std::vector<bool> yylookaheadNeeds;

        /** The last stack we invalidated.  */
        glr_state *yylastDeleted;
    };  // class glr_state_set
} // namespace

namespace
{
    class semantic_option
    {
      public:
    semantic_option ()
      : yyrule (0)
      , yystate (0)
      , yynext (0)
      , yyla ()]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {}

    semantic_option (rule_num rule)
      : yyrule (rule)
      , yystate (0)
      , yynext (0)
      , yyla ()]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {}

    semantic_option (const semantic_option& that)
      : yyrule (that.yyrule)
      , yystate (that.yystate)
      , yynext (that.yynext)
      , yyla (that.yyla)]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {]b4_parse_assert_if([[
      that.check_ ();]])[
    }

    // Needed for the assignment in yynewSemanticOption.
    semantic_option& operator= (const semantic_option& that)
    {]b4_parse_assert_if([[
      check_ ();
      that.check_ ();]])[
      yyrule = that.yyrule;
      yystate = that.yystate;
      yynext = that.yynext;
      yyla = that.yyla;
      return *this;
    }

    /// Only call state() and setState() on objects in yyitems, not temporaries.
    glr_state* state();
    const glr_state* state() const;
    void setState(const glr_state* s);

    const semantic_option* next () const YY_ATTRIBUTE_UNUSED;
    semantic_option* next ();
    void setNext (const semantic_option* s);

    std::ptrdiff_t indexIn (const glr_stack_item* array) const YY_ATTRIBUTE_UNUSED;

    /** True iff YYY0 and YYY1 represent identical options at the top level.
     *  That is, they represent the same rule applied to RHS symbols
     *  that produce the same terminal symbols.  */
    bool
    isIdenticalTo (const semantic_option& yyy1) const
    {]b4_parse_assert_if([[
      check_ ();
      yyy1.check_ ();]])[
      if (this->yyrule == yyy1.yyrule)
        {
                const glr_state *yys0, *yys1;
                int yyn;
                for (yys0 = this->state(), yys1 = yyy1.state(), yyn = yyrhsLength(this->yyrule);
                     yyn > 0; yys0 = yys0->pred(), yys1 = yys1->pred(), yyn -= 1)
                    if (yys0->yyposn != yys1->yyposn)
                        return false;
                return true;
        }
      else
        return false;
    }

    /** Assuming identicalOptions (YYY0,YYY1), destructively merge the
     *  alternative semantic values for the RHS-symbols of YYY1 and YYY0.  */
    void
    mergeWith (semantic_option& yyy1)
    {]b4_parse_assert_if([[
      check_ ();
      yyy1.check_ ();]])[
      glr_state *yys0 = this->state ();
      glr_state *yys1 = yyy1.state ();
      for (int yyn = yyrhsLength (this->yyrule);
           yyn > 0;
           yyn -= 1, yys0 = yys0->pred (), yys1 = yys1->pred ())
        {
                if (yys0 == yys1)
                    break;
                else if (yys0->yyresolved)
                {
                    yys1->yyresolved = true;]b4_variant_if([[
              YYASSERT (yys1->yylrState == yys0->yylrState);
              ]b4_symbol_variant([yy_accessing_symbol (yys0->yylrState)],
                                 [yys1->value ()], [copy], [yys0->value ()])], [[
              yys1->value () = yys0->value ();]])[
                }
                else if (yys1->yyresolved)
                {
                    yys0->yyresolved = true;]b4_variant_if([[
              YYASSERT (yys0->yylrState == yys1->yylrState);
              ]b4_symbol_variant([yy_accessing_symbol (yys1->yylrState)],
                                 [yys0->value ()], [copy], [yys1->value ()])], [[
              yys0->value () = yys1->value ();]])[
                }
                else
                {
                    semantic_option *yyz0prev = YY_NULLPTR;
                    semantic_option *yyz0     = yys0->firstVal();
                    semantic_option *yyz1     = yys1->firstVal();
                    while (true)
                    {
                        if (yyz1 == yyz0 || yyz1 == YY_NULLPTR)
                            break;
                        else if (yyz0 == YY_NULLPTR)
                        {
                            if (yyz0prev != YY_NULLPTR)
                                yyz0prev->setNext(yyz1);
                            else
                                yys0->setFirstVal(yyz1);
                            break;
                        }
                        else if (yyz0 < yyz1)
                        {
                            semantic_option *yyz = yyz0;
                            if (yyz0prev != YY_NULLPTR)
                                yyz0prev->setNext(yyz1);
                            else
                                yys0->setFirstVal(yyz1);
                            yyz1 = yyz1->next();
                            yyz0->setNext(yyz);
                        }
                        yyz0prev = yyz0;
                        yyz0     = yyz0->next();
                    }
                    yys1->setFirstVal(yys0->firstVal());
                }
        }
    }

#if] b4_api_PREFIX[DEBUG
    void yyreportTree (size_t yyindent = 2) const
    {]b4_parse_assert_if([[
      check_ ();]])[
      int yynrhs = yyrhsLength (this->yyrule);
      const glr_state* yystates[1 + YYMAXRHS];
      glr_state yyleftmost_state;

      {
                const glr_state *yys = this->state();
                for (int yyi = yynrhs; 0 < yyi; yyi -= 1)
                {
                    yystates[yyi] = yys;
                    yys           = yys->pred();
                }
                if (yys == YY_NULLPTR)
                {
                    yyleftmost_state.yyposn = 0;
                    yystates[0]             = &yyleftmost_state;
                }
                else
                    yystates[0] = yys;
      }

      std::string yylhs = ]b4_namespace_ref[::]b4_parser_class[::symbol_name (yylhsNonterm (this->yyrule));
      YYASSERT(this->state());
      if (this->state()->yyposn < yystates[0]->yyposn + 1)
        std::cerr << std::string(yyindent, ' ') << yylhs << " -> <Rule "
                  << this->yyrule - 1 << ", empty>\n";
      else
        std::cerr << std::string(yyindent, ' ') << yylhs << " -> <Rule "
                  << this->yyrule - 1 << ", tokens "
                  << yystates[0]->yyposn + 1 << " .. "
                  << this->state()->yyposn << ">\n";
      for (int yyi = 1; yyi <= yynrhs; yyi += 1)
        {
                if (yystates[yyi]->yyresolved)
                {
              std::string yysym = ]b4_namespace_ref[::]b4_parser_class[::symbol_name (yy_accessing_symbol (yystates[yyi]->yylrState));
              if (yystates[yyi-1]->yyposn+1 > yystates[yyi]->yyposn)
                std::cerr << std::string(yyindent + 2, ' ') << yysym
                          << " <empty>\n";
              else
                std::cerr << std::string(yyindent + 2, ' ') << yysym
                          << " <tokens " << yystates[yyi-1]->yyposn + 1
                          << " .. " << yystates[yyi]->yyposn << ">\n";
                }
                else
                    yystates[yyi]->firstVal()->yyreportTree(yyindent + 2);
        }
    }
#endif

    /** Rule number for this reduction */
    rule_num yyrule;

  private:
    template <typename T>
    static const glr_stack_item* asItem(const T* state)
    {
            return reinterpret_cast<const glr_stack_item *>(state);
    }
    template <typename T>
    static glr_stack_item* asItem(T* state)
    {
            return reinterpret_cast<glr_stack_item *>(state);
    }
    /** The last RHS state in the list of states to be reduced.  */
    std::ptrdiff_t yystate;
    /** Next sibling in chain of options.  To facilitate merging,
     *  options are chained in decreasing order by address.  */
    std::ptrdiff_t yynext;

  public:
    /** The lookahead for this reduction.  */
    symbol_type yyla;

]b4_parse_assert_if([[
  public:
    // Check invariants.
    void check_ () const
    {
            YY_IGNORE_NULL_DEREFERENCE_BEGIN
            YYASSERT(this->magic_ == MAGIC);
            YY_IGNORE_NULL_DEREFERENCE_END
    }

    // A magic number to check our pointer arithmetic is sane.
    enum { MAGIC = 0xeff1cace };
    unsigned int magic_;]])[
    };  // class semantic_option
} // namespace

namespace
{
    /** Type of the items in the GLR stack.
     *  It can be either a glr_state or a semantic_option. The is_state_ field
     *  indicates which item of the union is valid.  */
    class glr_stack_item
    {
      public:
    glr_stack_item (bool state = true)
      : is_state_ (state)]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {
            if (is_state_)
                new (&raw_) glr_state;
            else
                new (&raw_) semantic_option;
    }

    glr_stack_item (const glr_stack_item& other) YY_NOEXCEPT YY_NOTHROW
      : is_state_ (other.is_state_)]b4_parse_assert_if([[
      , magic_ (MAGIC)]])[
    {]b4_parse_assert_if([[
      other.check_ ();]])[
      std::memcpy (raw_, other.raw_, union_size);
    }

    glr_stack_item& operator= (glr_stack_item other)
    {]b4_parse_assert_if([[
      check_ ();
      other.check_ ();]])[
      std::swap (is_state_, other.is_state_);
      std::swap (raw_, other.raw_);
      return *this;
    }

    ~glr_stack_item ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      if (is_state ())
        getState ().~glr_state ();
      else
        getOption ().~semantic_option ();
    }

    void setState (const glr_state &state)
    {]b4_parse_assert_if([[
      check_ ();
      state.check_ ();]])[
      if (this != state.asItem ())
        {
                if (is_state_)
                    getState().~glr_state();
                else
                    getOption().~semantic_option();
                new (&raw_) glr_state(state);
                is_state_ = true;
        }
    }

    glr_state& getState ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      YYDASSERT (is_state ());
      void *yyp = raw_;
      glr_state& res = *static_cast<glr_state*> (yyp);]b4_parse_assert_if([[
      res.check_ ();]])[
      return res;
    }

    const glr_state& getState () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      YYDASSERT (is_state ());
      const void *yyp = raw_;
      const glr_state& res = *static_cast<const glr_state*> (yyp);]b4_parse_assert_if([[
      res.check_ ();]])[
      return res;
    }

    semantic_option& getOption ()
    {]b4_parse_assert_if([[
      check_ ();]])[
      YYDASSERT (!is_state ());
      void *yyp = raw_;
      return *static_cast<semantic_option*> (yyp);
    }
    const semantic_option& getOption () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      YYDASSERT (!is_state ());
      const void *yyp = raw_;
      return *static_cast<const semantic_option*> (yyp);
    }
    bool is_state () const
    {]b4_parse_assert_if([[
      check_ ();]])[
      return is_state_;
    }

  private:
    /// The possible contents of raw_. Since they have constructors, they cannot
    /// be directly included in the union.
    union contents
    {
            char yystate[sizeof(glr_state)];
            char yyoption[sizeof(semantic_option)];
    };
    enum { union_size = sizeof (contents) };
    union {
            /// Strongest alignment constraints.
            long double yyalign_me;
            /// A buffer large enough to store the contents.
            char raw_[union_size];
    };
    /** Type tag for the union. */
    bool is_state_;
]b4_parse_assert_if([[
  public:
    // Check invariants.
    void check_ () const
    {
            YYASSERT(this->magic_ == MAGIC);
            YYASSERT(this->is_state_ == false || this->is_state_ == true);
    }
    // A magic number to check our pointer arithmetic is sane.
    enum { MAGIC = 0xDEAD1ACC }; // 3735886540.
    const unsigned int magic_;]])[
    };  // class glr_stack_item
} // namespace

glr_state* glr_state::pred ()
{]b4_parse_assert_if([[
  check_ ();]])[
  YY_IGNORE_NULL_DEREFERENCE_BEGIN
  return yypred ? &asItem (as_pointer_ (this) - yypred)->getState () : YY_NULLPTR;
  YY_IGNORE_NULL_DEREFERENCE_END
}

const glr_state* glr_state::pred () const
{]b4_parse_assert_if([[
  check_ ();]])[
  YY_IGNORE_NULL_DEREFERENCE_BEGIN
  return yypred ? &asItem (as_pointer_ (this) - yypred)->getState () : YY_NULLPTR;
  YY_IGNORE_NULL_DEREFERENCE_END
}

void glr_state::setPred (const glr_state* state)
{]b4_parse_assert_if([[
  check_ ();
  if (state)
    state->check_ ();]])[
  yypred = state ? as_pointer_ (this) - as_pointer_ (state) : 0;
}

semantic_option* glr_state::firstVal ()
{]b4_parse_assert_if([[
  check_ ();]])[
  return yyfirstVal ? &(asItem(this) - yyfirstVal)->getOption() : YY_NULLPTR;
}

const semantic_option* glr_state::firstVal () const
{]b4_parse_assert_if([[
  check_ ();]])[
  return yyfirstVal ? &(asItem(this) - yyfirstVal)->getOption() : YY_NULLPTR;
}

void glr_state::setFirstVal (const semantic_option* option)
{]b4_parse_assert_if([[
  check_ ();]])[
  yyfirstVal = option ? asItem(this) - asItem(option) : 0;
}

std::ptrdiff_t glr_state::indexIn (const glr_stack_item* array) const
{]b4_parse_assert_if([[
  check_ ();]])[
  return asItem(this) - array;
}

std::ptrdiff_t semantic_option::indexIn (const glr_stack_item* array) const
{
    return asItem(this) - array;
}

glr_state* semantic_option::state ()
{
    YY_IGNORE_NULL_DEREFERENCE_BEGIN
    return yystate ? &(asItem(this) - yystate)->getState() : YY_NULLPTR;
    YY_IGNORE_NULL_DEREFERENCE_END
}

const glr_state* semantic_option::state () const
{
    return yystate ? &(asItem(this) - yystate)->getState() : YY_NULLPTR;
}

void semantic_option::setState (const glr_state* s)
{
    yystate = s ? asItem(this) - asItem(s) : 0;
}

const semantic_option* semantic_option::next () const
{
    return yynext ? &(asItem(this) - yynext)->getOption() : YY_NULLPTR;
}

semantic_option* semantic_option::next ()
{
    return yynext ? &(asItem(this) - yynext)->getOption() : YY_NULLPTR;
}

void semantic_option::setNext (const semantic_option* s)
{
    yynext = s ? asItem(this) - asItem(s) : 0;
}

void glr_state::destroy (char const* yymsg, ]b4_namespace_ref[::]b4_parser_class[& yyparser)
{]b4_parse_assert_if([[
  check_ ();]])[
  if (yyresolved)
    yyparser.yy_destroy_ (yymsg, yy_accessing_symbol(yylrState),
                          value ()]b4_locations_if([, yyloc])[);
  else
    {
#if] b4_api_PREFIX[DEBUG
      YYCDEBUG << yymsg
               << (firstVal() ? " unresolved " : " incomplete ")
               << (yy_accessing_symbol (yylrState) < YYNTOKENS ? "token" : "nterm")
               << ' ' << yyparser.symbol_name (yy_accessing_symbol (yylrState))
               << " ("]b4_locations_if([[
               << yyloc << ": "]])[
               << ")\n";
#endif
      if (firstVal() != YY_NULLPTR)
        {
            semantic_option &yyoption = *firstVal();
            glr_state *yyrh           = yyoption.state();
            for (int yyn = yyrhsLength(yyoption.yyrule); yyn > 0; yyn -= 1)
            {
                yyrh->destroy(yymsg, yyparser);
                yyrh = yyrh->pred();
            }
        }
    }
}

#undef YYFILL
#define YYFILL(N) yyfill(yyvsp, yylow, (N), yynormal)

namespace
{
    class state_stack
    {
      public:
    using parser_type = ]b4_namespace_ref[::]b4_parser_class[;
    using symbol_kind = parser_type::symbol_kind;
    using value_type = parser_type::value_type;]b4_locations_if([[
    using location_type = parser_type::location_type;]])[

    /** Initialize to a single empty stack, with total maximum
     *  capacity for all stacks of YYSIZE.  */
    state_stack (size_t yysize)
      : yysplitPoint (YY_NULLPTR)
    {
            yyitems.reserve(yysize);
    }

#if YYSTACKEXPANDABLE
    /** Returns false if it tried to expand but could not. */
    bool
    yyexpandGLRStackIfNeeded ()
    {
            return YYHEADROOM <= spaceLeft() || yyexpandGLRStack();
    }

  private:
    /** If *this is expandable, extend it.  WARNING: Pointers into the
        stack from outside should be considered invalid after this call.
        We always expand when there are 1 or fewer items left AFTER an
        allocation, so that we can avoid having external pointers exist
        across an allocation.  */
    bool
    yyexpandGLRStack ()
    {
            const size_t oldsize = yyitems.size();
            if (YYMAXDEPTH - YYHEADROOM < oldsize)
                return false;
            const size_t yynewSize        = YYMAXDEPTH < 2 * oldsize ? YYMAXDEPTH : 2 * oldsize;
            const glr_stack_item *oldbase = &yyitems[0];

            yyitems.reserve(yynewSize);
            const glr_stack_item *newbase = &yyitems[0];

            // Adjust the pointers.  Perform raw pointer arithmetic, as there
            // is no reason for objects to be aligned on their size.
            const ptrdiff_t disp =
                reinterpret_cast<const char *>(newbase) - reinterpret_cast<const char *>(oldbase);
            if (yysplitPoint)
                const_cast<glr_state *&>(yysplitPoint) = reinterpret_cast<glr_state *>(
                    reinterpret_cast<char *>(const_cast<glr_state *>(yysplitPoint)) + disp);

            for (std::vector<glr_state *>::iterator i = yytops.begin(), yyend = yytops.end();
                 i != yyend; ++i)
                if (glr_state_not_null(*i))
                    *i = reinterpret_cast<glr_state *>(reinterpret_cast<char *>(*i) + disp);

            return true;
    }

  public:
#else
    bool yyexpandGLRStackIfNeeded ()
    {
            return YYHEADROOM <= spaceLeft();
    }
#endif
#undef YYSTACKEXPANDABLE

    static bool glr_state_not_null (glr_state* s)
    {
            return s != YY_NULLPTR;
    }

    bool
    reduceToOneStack ()
    {
            using iterator         = std::vector<glr_state *>::iterator;
            const iterator yybegin = yytops.begin();
            const iterator yyend   = yytops.end();
            const iterator yyit    = std::find_if(yybegin, yyend, glr_state_not_null);
            if (yyit == yyend)
                return false;
            for (state_set_index yyk = create_state_set_index(yyit + 1 - yybegin);
                 yyk.uget() != numTops(); ++yyk)
                yytops.yymarkStackDeleted(yyk);
            yytops.yyremoveDeletes();
            yycompressStack();
            return true;
    }

    /** Called when returning to deterministic operation to clean up the extra
     * stacks. */
    void
    yycompressStack ()
    {
            if (yytops.size() != 1 || !isSplit())
                return;

            // yyr is the state after the split point.
            glr_state *yyr = YY_NULLPTR;
            for (glr_state *yyp = firstTop(), *yyq = yyp->pred(); yyp != yysplitPoint;
                 yyr = yyp, yyp = yyq, yyq = yyp->pred())
                yyp->setPred(yyr);

            // This const_cast is okay, since anyway we have access to the mutable
            // yyitems into which yysplitPoint points.
            glr_stack_item *nextFreeItem = const_cast<glr_state *>(yysplitPoint)->asItem() + 1;
            yysplitPoint                 = YY_NULLPTR;
            yytops.clearLastDeleted();

            while (yyr != YY_NULLPTR)
            {
                nextFreeItem->setState(*yyr);
                glr_state &nextFreeState = nextFreeItem->getState();
                yyr                      = yyr->pred();
                nextFreeState.setPred(&(nextFreeItem - 1)->getState());
                setFirstTop(&nextFreeState);
                ++nextFreeItem;
            }
            yyitems.resize(static_cast<size_t>(nextFreeItem - yyitems.data()));
    }

    bool isSplit() const {
            return yysplitPoint != YY_NULLPTR;
    }

    // Present the interface of a vector of glr_stack_item.
    std::vector<glr_stack_item>::const_iterator begin () const
    {
            return yyitems.begin();
    }

    std::vector<glr_stack_item>::const_iterator end () const
    {
            return yyitems.end();
    }

    size_t size() const
    {
            return yyitems.size();
    }

    glr_stack_item& operator[] (size_t i)
    {
            return yyitems[i];
    }

    glr_stack_item& stackItemAt (size_t index)
    {
            return yyitems[index];
    }

    size_t numTops () const
    {
            return yytops.size();
    }

    glr_state* firstTop () const
    {
            return yytops[create_state_set_index(0)];
    }

    glr_state* topAt (state_set_index i) const
    {
            return yytops[i];
    }

    void setFirstTop (glr_state* value)
    {
            yytops[create_state_set_index(0)] = value;
    }

    void setTopAt (state_set_index i, glr_state* value)
    {
            yytops[i] = value;
    }

    void pop_back ()
    {
            yyitems.pop_back();
    }

    void pop_back (size_t n)
    {
            yyitems.resize(yyitems.size() - n);
    }

    state_set_index
    yysplitStack (state_set_index yyk)
    {
            if (!isSplit())
            {
                YYASSERT(yyk.get() == 0);
                yysplitPoint = topAt(yyk);
            }
            return yytops.yysplitStack(yyk);
    }

    /** Assuming that YYS is a GLRState somewhere on *this, update the
     *  splitpoint of *this, if needed, so that it is at least as deep as
     *  YYS.  */
    void
    yyupdateSplit (glr_state& yys)
    {
            if (isSplit() && &yys < yysplitPoint)
                yysplitPoint = &yys;
    }

    /** Return a fresh GLRState.
     * Callers should call yyreserveStack afterwards to make sure there is
     * sufficient headroom.  */
    glr_state& yynewGLRState (const glr_state& newState)
    {
            glr_state &state = yyitems[yynewGLRStackItem(true)].getState();
#if false && 201103L <= YY_CPLUSPLUS
      state = std::move (newState);
#else
            state = newState;
#endif
            return state;
    }

    /** Return a fresh SemanticOption.
     * Callers should call yyreserveStack afterwards to make sure there is
     * sufficient headroom.  */
    semantic_option& yynewSemanticOption (semantic_option newOption)
    {
            semantic_option &option = yyitems[yynewGLRStackItem(false)].getOption();
            option                  = std::move(newOption);
            return option;
    }

    /* Do nothing if YYNORMAL or if *YYLOW <= YYLOW1.  Otherwise, fill in
     * YYVSP[YYLOW1 .. *YYLOW-1] as in yyfillin and set *YYLOW = YYLOW1.
     * For convenience, always return YYLOW1.  */
    int
    yyfill (glr_stack_item *yyvsp, int &yylow, int yylow1, bool yynormal)
    {
            if (!yynormal && yylow1 < yylow)
            {
                yyfillin(yyvsp, yylow, yylow1);
                yylow = yylow1;
            }
            return yylow1;
    }

    /** Fill in YYVSP[YYLOW1 .. YYLOW0-1] from the chain of states starting
     *  at YYVSP[YYLOW0].getState().pred().  Leaves YYVSP[YYLOW1].getState().pred()
     *  containing the pointer to the next state in the chain.  */
    void
    yyfillin (glr_stack_item *yyvsp, int yylow0, int yylow1)
    {
            glr_state *s = yyvsp[yylow0].getState().pred();
            YYASSERT(s != YY_NULLPTR);
            for (int i = yylow0 - 1; i >= yylow1; i -= 1, s = s->pred())
            {
                glr_state &yys = yyvsp[i].getState();
#if] b4_api_PREFIX[DEBUG
                yys.yylrState = s->yylrState;
#endif
                yys.yyresolved = s->yyresolved;
                if (s->yyresolved)
                {]b4_variant_if([[
              new (&yys.value ()) value_type ();
              ]b4_symbol_variant([yy_accessing_symbol (s->yylrState)],
                                 [yys.value ()], [copy], [s->value ()])], [[
              new (&yys.value ()) value_type (s->value ());]])[
                }
                else
                    /* The effect of using yyval or yyloc (in an immediate
                     * rule) is undefined.  */
                    yys.setFirstVal(YY_NULLPTR);]b4_locations_if([[
          yys.yyloc = s->yyloc;]])[
          yys.setPred(s->pred());
            }
    }

#if] b4_api_PREFIX[DEBUG

    /*----------------------------------------------------------------------.
    | Report that stack #YYK of *YYSTACKP is going to be reduced by YYRULE. |
    `----------------------------------------------------------------------*/

    void
    yy_reduce_print (bool yynormal, glr_stack_item* yyvsp, state_set_index yyk,
                     rule_num yyrule, parser_type& yyparser)
    {
            int yynrhs = yyrhsLength(yyrule);]b4_locations_if([
      int yylow = 1;])[
      int yyi;
      std::cerr << "Reducing stack " << yyk.get() << " by rule " << yyrule - 1
                << " (line " << int (yyrline[yyrule]) << "):\n";
      if (! yynormal)
        yyfillin (yyvsp, 1, -yynrhs);
      /* The symbols being reduced.  */
      for (yyi = 0; yyi < yynrhs; yyi++)
        {
                std::cerr << "   $" << yyi + 1 << " = ";
          yyparser.yy_symbol_print_
            (yy_accessing_symbol (yyvsp[yyi - yynrhs + 1].getState().yylrState),
             yyvsp[yyi - yynrhs + 1].getState().value ()]b4_locations_if([[,
             ]b4_rhs_location(yynrhs, yyi + 1)])[);
          if (!yyvsp[yyi - yynrhs + 1].getState().yyresolved)
              std::cerr << " (unresolved)";
          std::cerr << '\n';
        }
    }

#    define YYINDEX(YYX) ((YYX) == YY_NULLPTR ? -1 : (YYX)->indexIn(yyitems.data()))

    void
    dumpStack () const
    {
            for (size_t yyi = 0; yyi < size(); ++yyi)
            {
                const glr_stack_item &item = yyitems[yyi];
                std::cerr << std::setw(3) << yyi << ". ";
                if (item.is_state())
                {
                    std::cerr << "Res: " << item.getState().yyresolved
                              << ", LR State: " << item.getState().yylrState
                              << ", posn: " << item.getState().yyposn
                              << ", pred: " << YYINDEX(item.getState().pred());
                    if (!item.getState().yyresolved)
                        std::cerr << ", firstVal: " << YYINDEX(item.getState().firstVal());
                }
                else
                {
                    std::cerr << "Option. rule: " << item.getOption().yyrule - 1
                              << ", state: " << YYINDEX(item.getOption().state())
                              << ", next: " << YYINDEX(item.getOption().next());
                }
                std::cerr << '\n';
            }
            std::cerr << "Tops:";
            for (state_set_index yyi = create_state_set_index(0); yyi.uget() < numTops(); ++yyi)
            {
                std::cerr << yyi.get() << ": " << YYINDEX(topAt(yyi)) << "; ";
            }
            std::cerr << '\n';
    }

#    undef YYINDEX
#endif

    YYRESULTTAG
    yyreportAmbiguity (const semantic_option& yyx0,
                       const semantic_option& yyx1, parser_type& yyparser]b4_locations_if([, const location_type& yyloc])[)
    {
            YY_USE(yyx0);
            YY_USE(yyx1);

#if] b4_api_PREFIX[DEBUG
            std::cerr << "Ambiguity detected.\n"
                         "Option 1,\n";
            yyx0.yyreportTree();
            std::cerr << "\nOption 2,\n";
            yyx1.yyreportTree();
            std::cerr << '\n';
#endif

      yyparser.error (]b4_locations_if([yyloc, ])[YY_("syntax is ambiguous"));
      return yyabort;
    }

#if] b4_api_PREFIX[DEBUG
    /* Print YYS (possibly NULL) and its predecessors. */
    void
    yypstates (const glr_state* yys) const
    {
            if (yys != YY_NULLPTR)
                yys->yy_yypstack();
            else
                std::cerr << "<null>";
            std::cerr << '\n';
    }
#endif

  private:
    size_t spaceLeft() const
    {
            return yyitems.capacity() - yyitems.size();
    }

    /** Return a fresh GLRStackItem in this.  The item is an LR state
     *  if YYIS_STATE, and otherwise a semantic option.  Callers should call
     *  yyreserveStack afterwards to make sure there is sufficient
     *  headroom.  */
    size_t
    yynewGLRStackItem (bool yyis_state)
    {
            YYDASSERT(yyitems.size() < yyitems.capacity());
            yyitems.push_back(glr_stack_item(yyis_state));
            return yyitems.size() - 1;
    }


  public:
    std::vector<glr_stack_item> yyitems;
    // Where the stack splits. Anything below this address is deterministic.
    const glr_state* yysplitPoint;
    glr_state_set yytops;
    };  // class state_stack
} // namespace

#undef YYFILL
#define YYFILL(N) yystateStack.yyfill(yyvsp, yylow, (N), yynormal)

namespace ]b4_namespace_ref[
{
  class ]b4_parser_class[::glr_stack
  {
    public:
]b4_parse_error_bmatch([custom\|detailed\|verbose], [[
    // Needs access to yypact_value_is_default, etc.
    friend context;
]])[

    glr_stack (size_t yysize, parser_type& yyparser_yyarg]m4_ifset([b4_parse_param], [, b4_parse_param_decl])[)
      : yyerrState (0)
      , yystateStack (yysize)
      , yyerrcnt (0)
      , yyla ()
      , yyparser (yyparser_yyarg)]m4_ifset([b4_parse_param], [,b4_parse_param_cons])[
    {}

    ~glr_stack ()
    {
            if (!this->yyla.empty())
        yyparser.yy_destroy_ ("Cleanup: discarding lookahead",
                              this->yyla.kind (), this->yyla.value]b4_locations_if([, this->yyla.location])[);
            popall_();
    }

    int yyerrState;
]b4_locations_if([[  /* To compute the location of the error token.  */
    glr_stack_item yyerror_range[3];]])[
    state_stack yystateStack;
    int yyerrcnt;
    symbol_type yyla;
    YYJMP_BUF yyexception_buffer;
    parser_type& yyparser;

#define YYCHK1(YYE)                \
    do                             \
    {                              \
        switch (YYE)               \
        {                          \
            case yyok:             \
                break;             \
            case yyabort:          \
                goto yyabortlab;   \
            case yyaccept:         \
                goto yyacceptlab;  \
            case yyerr:            \
                goto yyuser_error; \
            default:               \
                goto yybuglab;     \
        }                          \
    } while (false)

    int
    parse ()
    {
            int yyresult;
            size_t yyposn;

            YYCDEBUG << "Starting parse\n";

            this->yyla.clear();
]m4_ifdef([b4_initial_action], [
b4_dollar_pushdef([yyla.value], [], [], [yyla.location])dnl
      b4_user_initial_action
b4_dollar_popdef])[]dnl
[
      switch (YYSETJMP (this->yyexception_buffer))
        {
                case 0:
                    break;
                case 1:
                    goto yyabortlab;
                case 2:
                    goto yyexhaustedlab;
                default:
                    goto yybuglab;
        }
      this->yyglrShift (create_state_set_index(0), 0, 0, this->yyla.value]b4_locations_if([, this->yyla.location])[);
      yyposn = 0;

      while (true)
        {
                /* For efficiency, we have two loops, the first of which is
                   specialized to deterministic operation (single stack, no
                   potential ambiguity).  */
                /* Standard mode */
                while (true)
                {
                    const state_num yystate = this->firstTopState()->yylrState;
                    YYCDEBUG << "Entering state " << yystate << '\n';
                    if (yystate == YYFINAL)
                        goto yyacceptlab;
                    if (yy_is_defaulted_state(yystate))
                    {
                        const rule_num yyrule = yy_default_action(yystate);
                        if (yyrule == 0)
                        {]b4_locations_if([[
                      this->yyerror_range[1].getState().yyloc = this->yyla.location;]])[
                      this->yyreportSyntaxError ();
                      goto yyuser_error;
                        }
                        YYCHK1(this->yyglrReduce(create_state_set_index(0), yyrule, true));
                    }
                    else
                    {
                        yyget_token();
                        const short *yyconflicts;
                        const int yyaction =
                            yygetLRActions(yystate, this->yyla.kind(), yyconflicts);
                        if (*yyconflicts != 0)
                            break;
                        if (yy_is_shift_action(yyaction))
                        {
                            YY_SYMBOL_PRINT("Shifting", this->yyla.kind(), this->yyla.value,
                                            this->yyla.location);
                            yyposn += 1;
                            // FIXME: we should move yylval.
                      this->yyglrShift (create_state_set_index(0), yyaction, yyposn, this->yyla.value]b4_locations_if([, this->yyla.location])[);
                      yyla.clear();
                      if (0 < this->yyerrState)
                          this->yyerrState -= 1;
                        }
                        else if (yy_is_error_action(yyaction))
                        {]b4_locations_if([[
                      this->yyerror_range[1].getState().yyloc = this->yyla.location;]])[
                      /* Don't issue an error message again for exceptions
                         thrown from the scanner.  */
                      if (this->yyla.kind () != ]b4_symbol(error, kind)[)
                        this->yyreportSyntaxError ();
                      goto yyuser_error;
                        }
                        else
                            YYCHK1(this->yyglrReduce(create_state_set_index(0), -yyaction, true));
                    }
                }

                while (true)
                {
                    for (state_set_index yys = create_state_set_index(0);
                         yys.uget() < this->yystateStack.numTops(); ++yys)
                        this->yystateStack.yytops.setLookaheadNeeds(yys, !this->yyla.empty());

                    /* yyprocessOneStack returns one of three things:

                        - An error flag.  If the caller is yyprocessOneStack, it
                          immediately returns as well.  When the caller is finally
                          yyparse, it jumps to an error label via YYCHK1.

                        - yyok, but yyprocessOneStack has invoked yymarkStackDeleted
                          (yys), which sets the top state of yys to NULL.  Thus,
                          yyparse's following invocation of yyremoveDeletes will remove
                          the stack.

                        - yyok, when ready to shift a token.

                       Except in the first case, yyparse will invoke yyremoveDeletes and
                       then shift the next token onto all remaining stacks.  This
                       synchronization of the shift (that is, after all preceding
                       reductions on all stacks) helps prevent double destructor calls
                       on yylval in the event of memory exhaustion.  */

                    for (state_set_index yys = create_state_set_index(0);
                         yys.uget() < this->yystateStack.numTops(); ++yys)
                YYCHK1 (this->yyprocessOneStack (yys, yyposn]b4_locations_if([, &this->yyla.location])[));
                    this->yystateStack.yytops.yyremoveDeletes();
                    if (this->yystateStack.yytops.size() == 0)
                    {
                        this->yystateStack.yytops.yyundeleteLastStack();
                        if (this->yystateStack.yytops.size() == 0)
                    this->yyFail (]b4_locations_if([&this->yyla.location, ])[YY_("syntax error"));
                        YYCHK1(this->yyresolveStack());
                        YYCDEBUG << "Returning to deterministic operation.\n";]b4_locations_if([[
                  this->yyerror_range[1].getState ().yyloc = this->yyla.location;]])[
                  this->yyreportSyntaxError ();
                  goto yyuser_error;
                    }

                    /* If any yyglrShift call fails, it will fail after shifting.  Thus,
                       a copy of yylval will already be on stack 0 in the event of a
                       failure in the following loop.  Thus, yyla is emptied
                       before the loop to make sure the user destructor for yylval isn't
                       called twice.  */
                    symbol_kind_type yytoken_to_shift = this->yyla.kind();
              this->yyla.kind_ = ]b4_symbol(empty, kind)[;
              yyposn += 1;
              for (state_set_index yys = create_state_set_index (0); yys.uget () < this->yystateStack.numTops (); ++yys)
                {
                        const state_num yystate = this->topState(yys)->yylrState;
                        const short *yyconflicts;
                        const int yyaction = yygetLRActions(yystate, yytoken_to_shift, yyconflicts);
                        /* Note that yyconflicts were handled by yyprocessOneStack.  */
                        YYCDEBUG << "On stack " << yys.get() << ", ";
                        YY_SYMBOL_PRINT("shifting", yytoken_to_shift, this->yyla.value,
                                        this->yyla.location);
                  this->yyglrShift (yys, yyaction, yyposn, this->yyla.value]b4_locations_if([, this->yyla.location])[);
                  YYCDEBUG << "Stack " << yys.get() << " now in state "
                           << this->topState(yys)->yylrState << '\n';
                }
]b4_variant_if([[
                // FIXME: User destructors.
                // Value type destructor.
                ]b4_symbol_variant([[yytoken_to_shift]], [[this->yyla.value]], [[template destroy]])])[

              if (this->yystateStack.yytops.size () == 1)
                {
                        YYCHK1(this->yyresolveStack());
                        YYCDEBUG << "Returning to deterministic operation.\n";
                        this->yystateStack.yycompressStack();
                        break;
                }
                }
                continue;
            yyuser_error:
          this->yyrecoverSyntaxError (]b4_locations_if([&this->yyla.location])[);
          yyposn = this->firstTopState()->yyposn;
        }

    yyacceptlab:
      yyresult = 0;
      goto yyreturn;

    yybuglab:
      YYASSERT (false);
      goto yyabortlab;

    yyabortlab:
      yyresult = 1;
      goto yyreturn;

    yyexhaustedlab:
      yyparser.error (]b4_locations_if([this->yyla.location, ])[YY_("memory exhausted"));
      yyresult = 2;
      goto yyreturn;

    yyreturn:
      return yyresult;
    }
#undef YYCHK1

    void yyreserveGlrStack ()
    {
            if (!yystateStack.yyexpandGLRStackIfNeeded())
                yyMemoryExhausted();
    }

    _Noreturn void
    yyMemoryExhausted ()
    {
            YYLONGJMP(yyexception_buffer, 2);
    }

    _Noreturn void
    yyFail (]b4_locations_if([location_type* yylocp, ])[const char* yymsg)
    {
            if (yymsg != YY_NULLPTR)
        yyparser.error (]b4_locations_if([*yylocp, ])[yymsg);
            YYLONGJMP(yyexception_buffer, 1);
    }

                                  /* GLRStates */


    /** Add a new semantic action that will execute the action for rule
     *  YYRULE on the semantic values in YYRHS to the list of
     *  alternative actions for YYSTATE.  Assumes that YYRHS comes from
     *  stack #YYK of *this. */
    void
    yyaddDeferredAction (state_set_index yyk, glr_state* yystate,
                         glr_state* yyrhs, rule_num yyrule)
    {
            semantic_option &yyopt = yystateStack.yynewSemanticOption(semantic_option(yyrule));
            yyopt.setState(yyrhs);
            yyopt.setNext(yystate->firstVal());
            if (yystateStack.yytops.lookaheadNeeds(yyk))
                yyopt.yyla = this->yyla;
            yystate->setFirstVal(&yyopt);

            yyreserveGlrStack();
    }

#if] b4_api_PREFIX[DEBUG
    void yypdumpstack () const
    {
            yystateStack.dumpStack();
    }
#endif

    void
    yyreportSyntaxError ()
    {
            if (yyerrState != 0)
                return;
]b4_parse_error_case(
[simple], [[
      std::string msg = YY_("syntax error");
      yyparser.error (]b4_join(b4_locations_if([yyla.location]), [[YY_MOVE (msg)]])[);]],
[custom], [[
      context yyctx (*this, yyla);
      yyparser.report_syntax_error (yyctx);]],
[[
      context yyctx (*this, yyla);
      std::string msg = yyparser.yysyntax_error_ (yyctx);
      yyparser.error (]b4_join(b4_locations_if([yyla.location]), [[YY_MOVE (msg)]])[);]])[
      yyerrcnt += 1;
    }

    /* Recover from a syntax error on this, assuming that yytoken,
       yylval, and yylloc are the syntactic category, semantic value, and location
       of the lookahead.  */
    void
    yyrecoverSyntaxError (]b4_locations_if([location_type* yylocp])[)
    {
            if (yyerrState == 3)
                /* We just shifted the error token and (perhaps) took some
                   reductions.  Skip tokens until we can proceed.  */
                while (true)
                {
            if (this->yyla.kind () == ]b4_symbol(eof, kind)[)
              yyFail (]b4_locations_if([yylocp, ])[YY_NULLPTR);
            if (this->yyla.kind () != ]b4_symbol(empty, kind)[)
            {]b4_locations_if([[
                /* We throw away the lookahead, but the error range
                   of the shifted error token must take it into account.  */
                glr_state *yys = firstTopState();
                yyerror_range[1].getState().yyloc = yys->yyloc;
                yyerror_range[2].getState().yyloc = this->yyla.location;
                YYLLOC_DEFAULT ((yys->yyloc), yyerror_range, 2);]])[
                yyparser.yy_destroy_ ("Error: discarding",
                                      this->yyla.kind (), this->yyla.value]b4_locations_if([, this->yyla.location])[);]b4_variant_if([[
                // Value type destructor.
                ]b4_symbol_variant([[this->yyla.kind ()]], [[this->yyla.value]], [[template destroy]])])[
                this->yyla.kind_ = ]b4_symbol(empty, kind)[;
            }
            yyget_token();
            int yyj = yypact[firstTopState()->yylrState];
            if (yypact_value_is_default(yyj))
                return;
            yyj += this->yyla.kind();
            if (yyj < 0 || YYLAST < yyj || yycheck[yyj] != this->yyla.kind())
            {
                if (yydefact[firstTopState()->yylrState] != 0)
                    return;
            }
            else if (!yytable_value_is_error(yytable[yyj]))
                return;
                }

            if (!yystateStack.reduceToOneStack())
        yyFail (]b4_locations_if([yylocp, ])[YY_NULLPTR);

            /* Now pop stack until we find a state that shifts the error token.  */
            yyerrState = 3;
            while (firstTopState() != YY_NULLPTR)
            {
                glr_state *yys = firstTopState();
                int yyj        = yypact[yys->yylrState];
                if (!yypact_value_is_default(yyj))
                {
                    yyj += YYTERROR;
                    if (0 <= yyj && yyj <= YYLAST && yycheck[yyj] == YYTERROR &&
                        yy_is_shift_action(yytable[yyj]))
                    {
                  /* Shift the error token.  */]b4_locations_if([[
                  /* First adjust its location.*/
                  location_type yyerrloc;
                  yyerror_range[2].getState().yyloc = this->yyla.location;
                  YYLLOC_DEFAULT (yyerrloc, (yyerror_range), 2);]])[
                  YY_SYMBOL_PRINT ("Shifting", yy_accessing_symbol (yytable[yyj]),
                                   this->yyla.value, yyerrloc);
                  yyglrShift (create_state_set_index(0), yytable[yyj],
                              yys->yyposn, yyla.value]b4_locations_if([, yyerrloc])[);
                  yys = firstTopState();
                  break;
                    }
                }]b4_locations_if([[
          yyerror_range[1].getState().yyloc = yys->yyloc;]])[
          if (yys->pred() != YY_NULLPTR)
            yys->destroy ("Error: popping", yyparser);
          yystateStack.setFirstTop(yys->pred());
          yystateStack.pop_back();
            }
            if (firstTopState() == YY_NULLPTR)
        yyFail (]b4_locations_if([yylocp, ])[YY_NULLPTR);
    }

    YYRESULTTAG
    yyprocessOneStack (state_set_index yyk,
                       size_t yyposn]b4_locations_if([, location_type* yylocp])[)
    {
            while (yystateStack.topAt(yyk) != YY_NULLPTR)
            {
                const state_num yystate = topState(yyk)->yylrState;
                YYCDEBUG << "Stack " << yyk.get() << " Entering state " << yystate << '\n';

                YYASSERT(yystate != YYFINAL);

                if (yy_is_defaulted_state(yystate))
                {
                    const rule_num yyrule = yy_default_action(yystate);
                    if (yyrule == 0)
                    {
                        YYCDEBUG << "Stack " << yyk.get() << " dies.\n";
                        yystateStack.yytops.yymarkStackDeleted(yyk);
                        return yyok;
                    }
                    const YYRESULTTAG yyflag = yyglrReduce(yyk, yyrule, yyimmediate[yyrule]);
                    if (yyflag == yyerr)
                    {
                        YYCDEBUG << "Stack " << yyk.get()
                                 << " dies"
                                    " (predicate failure or explicit user error).\n";
                        yystateStack.yytops.yymarkStackDeleted(yyk);
                        return yyok;
                    }
                    if (yyflag != yyok)
                        return yyflag;
                }
                else
                {
                    yystateStack.yytops.setLookaheadNeeds(yyk, true);
                    yyget_token();
                    const short *yyconflicts;
                    const int yyaction = yygetLRActions(yystate, this->yyla.kind(), yyconflicts);

                    for (; *yyconflicts != 0; ++yyconflicts)
                    {
                        state_set_index yynewStack = yystateStack.yysplitStack(yyk);
                        YYCDEBUG << "Splitting off stack " << yynewStack.get() << " from "
                                 << yyk.get() << ".\n";
                        YYRESULTTAG yyflag =
                            yyglrReduce(yynewStack, *yyconflicts, yyimmediate[*yyconflicts]);
                        if (yyflag == yyok)
                    YYCHK (yyprocessOneStack (yynewStack,
                                              yyposn]b4_locations_if([, yylocp])[));
                        else if (yyflag == yyerr)
                        {
                            YYCDEBUG << "Stack " << yynewStack.get() << " dies.\n";
                            yystateStack.yytops.yymarkStackDeleted(yynewStack);
                        }
                        else
                            return yyflag;
                    }

                    if (yy_is_shift_action(yyaction))
                        break;
                    else if (yy_is_error_action(yyaction))
                    {
                        YYCDEBUG << "Stack " << yyk.get() << " dies.\n";
                        yystateStack.yytops.yymarkStackDeleted(yyk);
                        break;
                    }
                    else
                    {
                        YYRESULTTAG yyflag = yyglrReduce(yyk, -yyaction, yyimmediate[-yyaction]);
                        if (yyflag == yyerr)
                        {
                            YYCDEBUG << "Stack " << yyk.get()
                                     << " dies"
                                        " (predicate failure or explicit user error).\n";
                            yystateStack.yytops.yymarkStackDeleted(yyk);
                            break;
                        }
                        else if (yyflag != yyok)
                            return yyflag;
                    }
                }
            }
            return yyok;
    }

    /** Perform user action for rule number YYN, with RHS length YYRHSLEN,
     *  and top stack item YYVSP.  YYVALP points to place to put semantic
     *  value ($$), and yylocp points to place for location information
     *  (@@$).  Returns yyok for normal return, yyaccept for YYACCEPT,
     *  yyerr for YYERROR, yyabort for YYABORT.  */
    YYRESULTTAG
    yyuserAction (rule_num yyrule, int yyrhslen, glr_stack_item* yyvsp, state_set_index yyk,
                  value_type* yyvalp]b4_locations_if([, location_type* yylocp])[)
    {
            bool yynormal YY_ATTRIBUTE_UNUSED = !yystateStack.isSplit();
            int yylow                         = 1;
]b4_parse_param_use([yyvalp], [yylocp])dnl
[      YY_USE (yyk);
      YY_USE (yyrhslen);
#undef yyerrok
#define yyerrok (yyerrState = 0)
#undef YYACCEPT
#define YYACCEPT return yyaccept
#undef YYABORT
#define YYABORT return yyabort
#undef YYERROR
#define YYERROR return yyerrok, yyerr
#undef YYRECOVERING
#define YYRECOVERING() (yyerrState != 0)
#undef yytoken
#define yytoken this->yyla.kind_
#undef yyclearin
#define yyclearin (yytoken = ]b4_symbol(empty, kind)[)
#undef YYBACKUP
#define YYBACKUP(Token, Value)                                                                   \
      return yyparser.error (]b4_locations_if([*yylocp, ])[YY_("syntax error: cannot back up")),     \
             yyerrok, yyerr

]b4_variant_if([[
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      // However we really need to prepare yyvsp now if we want to get
      // correct locations, so invoke YYFILL for $1 anyway.
      (void) YYFILL (1-yyrhslen);
      ]b4_symbol_variant([[yylhsNonterm (yyrule)]], [(*yyvalp)], [emplace])], [[
      if (yyrhslen == 0)
        *yyvalp = yyval_default;
      else
        *yyvalp = yyvsp[YYFILL (1-yyrhslen)].getState().value ();]])[]b4_locations_if([[
      /* Default location. */
      YYLLOC_DEFAULT ((*yylocp), (yyvsp - yyrhslen), yyrhslen);
      yyerror_range[1].getState().yyloc = *yylocp;
]])[
    /* If yyk == -1, we are running a deferred action on a temporary
       stack.  In that case, YY_REDUCE_PRINT must not play with YYFILL,
       so pretend the stack is "normal". */
    YY_REDUCE_PRINT ((yynormal || yyk == create_state_set_index (-1), yyvsp, yyk, yyrule, yyparser));
#if YY_EXCEPTIONS
      try
      {
#endif  // YY_EXCEPTIONS
                switch (yyrule)
                {
    ]b4_user_actions[
          default: break;
                }
#if YY_EXCEPTIONS
      }
      catch (const syntax_error& yyexc)
        {
                YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';]b4_locations_if([
          *yylocp = yyexc.location;])[
          yyparser.error (]b4_locations_if([*yylocp, ])[yyexc.what ());
          YYERROR;
        }
#endif  // YY_EXCEPTIONS
    YY_SYMBOL_PRINT ("-> $$ =", yylhsNonterm (yyrule), *yyvalp, *yylocp);

      return yyok;
#undef yyerrok
#undef YYABORT
#undef YYACCEPT
#undef YYERROR
#undef YYBACKUP
#undef yytoken
#undef yyclearin
#undef YYRECOVERING
    }

    YYRESULTTAG
    yyresolveStack ()
    {
            if (yystateStack.isSplit())
            {
                int yyn = 0;
                for (glr_state *yys = firstTopState(); yys != yystateStack.yysplitPoint;
                     yys            = yys->pred())
                    yyn += 1;
                YYCHK(yyresolveStates(*firstTopState(), yyn));
            }
            return yyok;
    }

    /** Pop the symbols consumed by reduction #YYRULE from the top of stack
     *  #YYK of *YYSTACKP, and perform the appropriate semantic action on their
     *  semantic values.  Assumes that all ambiguities in semantic values
     *  have been previously resolved.  Set *YYVALP to the resulting value,
     *  and *YYLOCP to the computed location (if any).  Return value is as
     *  for userAction.  */
    YYRESULTTAG
    yydoAction (state_set_index yyk, rule_num yyrule,
                value_type* yyvalp]b4_locations_if([, location_type* yylocp])[)
    {
            const int yynrhs = yyrhsLength(yyrule);

            if (!yystateStack.isSplit())
            {
                /* Standard special case: single stack.  */
                YYASSERT(yyk.get() == 0);
                glr_stack_item *yyrhs = yystateStack.firstTop()->asItem();
          const YYRESULTTAG res
            = yyuserAction (yyrule, yynrhs, yyrhs, yyk, yyvalp]b4_locations_if([, yylocp])[);
          yystateStack.pop_back(static_cast<size_t>(yynrhs));
          yystateStack.setFirstTop(&yystateStack[yystateStack.size() - 1].getState());
          return res;
            }
            else
            {
                glr_stack_item yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
                glr_state *yys = yystateStack.topAt(yyk);
                yyrhsVals[YYMAXRHS + YYMAXLEFT].getState().setPred(yys);]b4_locations_if([[
          if (yynrhs == 0)
            /* Set default location.  */
            yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].getState().yyloc = yys->yyloc;]])[
          for (int yyi = 0; yyi < yynrhs; yyi += 1)
            {
                    yys = yys->pred();
                    YYASSERT(yys != YY_NULLPTR);
            }
          yystateStack.yyupdateSplit (*yys);
          yystateStack.setTopAt(yyk, yys);
          return yyuserAction (yyrule, yynrhs, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                               yyk,
                               yyvalp]b4_locations_if([, yylocp])[);
            }
    }

    /** Pop items off stack #YYK of *YYSTACKP according to grammar rule YYRULE,
     *  and push back on the resulting nonterminal symbol.  Perform the
     *  semantic action associated with YYRULE and store its value with the
     *  newly pushed state, if YYFORCEEVAL or if *YYSTACKP is currently
     *  unambiguous.  Otherwise, store the deferred semantic action with
     *  the new state.  If the new state would have an identical input
     *  position, LR state, and predecessor to an existing state on the stack,
     *  it is identified with that existing state, eliminating stack #YYK from
     *  *YYSTACKP.  In this case, the semantic value is
     *  added to the options for the existing state's semantic value.
     */
    YYRESULTTAG
    yyglrReduce (state_set_index yyk, rule_num yyrule, bool yyforceEval)
    {
            size_t yyposn = topState(yyk)->yyposn;

            if (yyforceEval || !yystateStack.isSplit())
            {
                value_type val;]b4_locations_if([[
          location_type loc;]])[

          YYRESULTTAG yyflag = yydoAction (yyk, yyrule, &val]b4_locations_if([, &loc])[);
          if (yyflag == yyerr && yystateStack.isSplit())
            {]b4_parse_trace_if([[
              YYCDEBUG << "Parse on stack " << yyk.get ()
                       << " rejected by rule " << yyrule - 1
                       << " (line " << int (yyrline[yyrule]) << ").\n";
            ]])[}
          if (yyflag != yyok)
            return yyflag;
          yyglrShift (yyk,
                      yyLRgotoState (topState(yyk)->yylrState,
                                     yylhsNonterm (yyrule)),
                      yyposn, val]b4_locations_if([, loc])[);]b4_variant_if([[
          // FIXME: User destructors.
          // Value type destructor.
          ]b4_symbol_variant([[yylhsNonterm (yyrule)]], [[val]], [[template destroy]])])[
            }
            else
            {
                glr_state *yys  = yystateStack.topAt(yyk);
                glr_state *yys0 = yys;
                for (int yyn = yyrhsLength(yyrule); 0 < yyn; yyn -= 1)
                {
                    yys = yys->pred();
                    YYASSERT(yys != YY_NULLPTR);
                }
                yystateStack.yyupdateSplit(*yys);
                state_num yynewLRState = yyLRgotoState(yys->yylrState, yylhsNonterm(yyrule));]b4_parse_trace_if([[
          YYCDEBUG << "Reduced stack " << yyk.get ()
                   << " by rule " << yyrule - 1 << " (line " << int (yyrline[yyrule])
                   << "); action deferred.  Now in state " << yynewLRState
                   << ".\n";]])[
          for (state_set_index yyi = create_state_set_index(0); yyi.uget() < yystateStack.numTops(); ++yyi)
            if (yyi != yyk && yystateStack.topAt(yyi) != YY_NULLPTR)
              {
                    const glr_state *yysplit = yystateStack.yysplitPoint;
                    glr_state *yyp           = yystateStack.topAt(yyi);
                    while (yyp != yys && yyp != yysplit && yyp->yyposn >= yyposn)
                    {
                        if (yyp->yylrState == yynewLRState && yyp->pred() == yys)
                        {
                            yyaddDeferredAction(yyk, yyp, yys0, yyrule);
                            yystateStack.yytops.yymarkStackDeleted(yyk);
                            YYCDEBUG << "Merging stack " << yyk.get() << " into stack " << yyi.get()
                                     << ".\n";
                            return yyok;
                        }
                        yyp = yyp->pred();
                    }
              }
          yystateStack.setTopAt(yyk, yys);
          yyglrShiftDefer (yyk, yynewLRState, yyposn, yys0, yyrule);
            }
            return yyok;
    }

    /** Shift stack #YYK of *YYSTACKP, to a new state corresponding to LR
     *  state YYLRSTATE, at input position YYPOSN, with the (unresolved)
     *  semantic value of YYRHS under the action for YYRULE.  */
    void
    yyglrShiftDefer (state_set_index yyk, state_num yylrState,
                     size_t yyposn, glr_state* yyrhs, rule_num yyrule)
    {
            glr_state &yynewState = yystateStack.yynewGLRState(glr_state(yylrState, yyposn));
            yynewState.setPred(yystateStack.topAt(yyk));
            yystateStack.setTopAt(yyk, &yynewState);

            /* Invokes yyreserveStack.  */
            yyaddDeferredAction(yyk, &yynewState, yyrhs, yyrule);
    }

    /** Shift to a new state on stack #YYK of *YYSTACKP, corresponding to LR
     * state YYLRSTATE, at input position YYPOSN, with (resolved) semantic
     * value YYVAL_ARG and source location YYLOC_ARG.  */
    void
    yyglrShift (state_set_index yyk, state_num yylrState,
                size_t yyposn,
                const value_type& yyval_arg]b4_locations_if([, const location_type& yyloc_arg])[)
    {
      glr_state& yynewState = yystateStack.yynewGLRState (
        glr_state (yylrState, yyposn, yyval_arg]b4_locations_if([, yyloc_arg])[));
      yynewState.setPred(yystateStack.topAt(yyk));
      yystateStack.setTopAt(yyk, &yynewState);
      yyreserveGlrStack();
    }

#if] b4_api_PREFIX[DEBUG
    void
    yypstack (state_set_index yyk) const
    {
            yystateStack.yypstates(yystateStack.topAt(yyk));
    }
#endif

    glr_state* topState(state_set_index i) {
            return yystateStack.topAt(i);
    }

    glr_state* firstTopState() {
            return yystateStack.firstTop();
    }

  private:

    void popall_ ()
    {
            /* If the stack is well-formed, pop the stack until it is empty,
               destroying its entries as we go.  But free the stack regardless
               of whether it is well-formed.  */
            for (state_set_index k = create_state_set_index(0); k.uget() < yystateStack.numTops();
                 k += 1)
                if (yystateStack.topAt(k) != YY_NULLPTR)
                {
                    while (yystateStack.topAt(k) != YY_NULLPTR)
                    {
                        glr_state *state          = topState(k);]b4_locations_if([[
                yyerror_range[1].getState().yyloc = state->yyloc;]])[
                if (state->pred() != YY_NULLPTR)
                  state->destroy ("Cleanup: popping", yyparser);
                yystateStack.setTopAt(k, state->pred());
                yystateStack.pop_back();
                    }
                    break;
                }
    }

    /** Resolve the previous YYN states starting at and including state YYS
     *  on *YYSTACKP. If result != yyok, some states may have been left
     *  unresolved possibly with empty semantic option chains.  Regardless
     *  of whether result = yyok, each state has been left with consistent
     *  data so that destroy can be invoked if necessary.  */
    YYRESULTTAG
    yyresolveStates (glr_state& yys, int yyn)
    {
            if (0 < yyn)
            {
                YYASSERT(yys.pred() != YY_NULLPTR);
                YYCHK(yyresolveStates(*yys.pred(), yyn - 1));
                if (!yys.yyresolved)
                    YYCHK(yyresolveValue(yys));
            }
            return yyok;
    }

    static void
    yyuserMerge (int yyn, value_type& yy0, value_type& yy1)
    {
            YY_USE(yy0);
            YY_USE(yy1);

            switch (yyn)
            {
]b4_mergers[
          default: break;
            }
    }

    /** Resolve the ambiguity represented in state YYS in *YYSTACKP,
     *  perform the indicated actions, and set the semantic value of YYS.
     *  If result != yyok, the chain of semantic options in YYS has been
     *  cleared instead or it has been left unmodified except that
     *  redundant options may have been removed.  Regardless of whether
     *  result = yyok, YYS has been left with consistent data so that
     *  destroy can be invoked if necessary.  */
    YYRESULTTAG
    yyresolveValue (glr_state& yys)
    {
            semantic_option *yybest = yys.firstVal();
            YYASSERT(yybest != YY_NULLPTR);
            bool yymerge = false;
            YYRESULTTAG yyflag;]b4_locations_if([
      location_type *yylocp = &yys.yyloc;])[

      semantic_option* yypPrev = yybest;
      for (semantic_option* yyp = yybest->next();
           yyp != YY_NULLPTR; )
        {
                if (yybest->isIdenticalTo(*yyp))
                {
                    yybest->mergeWith(*yyp);
                    yypPrev->setNext(yyp->next());
                    yyp = yypPrev->next();
                }
                else
                {
                    switch (yypreference(*yybest, *yyp))
                    {
                        case 0:]b4_locations_if([[
                  yyresolveLocations (yys, 1);]])[
                  return yystateStack.yyreportAmbiguity (*yybest, *yyp, yyparser]b4_locations_if([, *yylocp])[);
                  break;
                case 1:
                  yymerge = true;
                  break;
                case 2:
                  break;
                case 3:
                  yybest = yyp;
                  yymerge = false;
                  break;
                default:
                  /* This cannot happen so it is not worth a YYASSERT (false),
                     but some compilers complain if the default case is
                     omitted.  */
                  break;
                    }
                    yypPrev = yyp;
                    yyp     = yyp->next();
                }
        }

      value_type val;
      if (yymerge)
        {
                int yyprec = yydprec[yybest->yyrule];
          yyflag = yyresolveAction (*yybest, &val]b4_locations_if([, yylocp])[);
          if (yyflag == yyok)
              for (semantic_option *yyp = yybest->next(); yyp != YY_NULLPTR; yyp = yyp->next())
              {
                  if (yyprec == yydprec[yyp->yyrule])
                  {
                      value_type yyval_other;]b4_locations_if([
                    location_type yydummy;])[
                    yyflag = yyresolveAction (*yyp, &yyval_other]b4_locations_if([, &yydummy])[);
                    if (yyflag != yyok)
                      {
                        yyparser.yy_destroy_ ("Cleanup: discarding incompletely merged value for",
                                              yy_accessing_symbol (yys.yylrState),
                                              this->yyla.value]b4_locations_if([, *yylocp])[);
                        break;
                      }
                    yyuserMerge (yymerger[yyp->yyrule], val, yyval_other);]b4_variant_if([[
                    // FIXME: User destructors.
                    // Value type destructor.
                    ]b4_symbol_variant([[yy_accessing_symbol (yys.yylrState)]], [[yyval_other]], [[template destroy]])])[
                  }
              }
        }
      else
        yyflag = yyresolveAction (*yybest, &val]b4_locations_if([, yylocp])[);

      if (yyflag == yyok)
        {
                yys.yyresolved = true;
          YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN]b4_variant_if([[
          new (&yys.value ()) value_type ();
          ]b4_symbol_variant([yy_accessing_symbol (yys.yylrState)],
                             [yys.value ()], [copy], [val])], [[
          new (&yys.value ()) value_type (val);]])[

          YY_IGNORE_MAYBE_UNINITIALIZED_END
        }
      else
        yys.setFirstVal(YY_NULLPTR);
]b4_variant_if([[
      // FIXME: User destructors.
      // Value type destructor.
      ]b4_symbol_variant([[yy_accessing_symbol (yys.yylrState)]], [[val]], [[template destroy]])])[
      return yyflag;
    }

    /** Resolve the states for the RHS of YYOPT on *YYSTACKP, perform its
     *  user action, and return the semantic value and location in *YYVALP
     *  and *YYLOCP.  Regardless of whether result = yyok, all RHS states
     *  have been destroyed (assuming the user action destroys all RHS
     *  semantic values if invoked).  */
    YYRESULTTAG
    yyresolveAction (semantic_option& yyopt, value_type* yyvalp]b4_locations_if([, location_type* yylocp])[)
    {
            glr_state *yyoptState = yyopt.state();
            YYASSERT(yyoptState != YY_NULLPTR);
            int yynrhs         = yyrhsLength(yyopt.yyrule);
            YYRESULTTAG yyflag = yyresolveStates(*yyoptState, yynrhs);
            if (yyflag != yyok)
            {
                for (glr_state *yys = yyoptState; yynrhs > 0; yys = yys->pred(), yynrhs -= 1)
                    yys->destroy("Cleanup: popping", yyparser);
                return yyflag;
            }

            glr_stack_item yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
            yyrhsVals[YYMAXRHS + YYMAXLEFT].getState().setPred(yyopt.state());]b4_locations_if([[
      if (yynrhs == 0)
        /* Set default location.  */
        yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].getState().yyloc = yyoptState->yyloc;]])[
      {
                symbol_type yyla_current = std::move(this->yyla);
                this->yyla               = std::move(yyopt.yyla);
        yyflag = yyuserAction (yyopt.yyrule, yynrhs,
                               yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                               create_state_set_index (-1),
                               yyvalp]b4_locations_if([, yylocp])[);
        this->yyla                       = std::move(yyla_current);
      }
      return yyflag;
    }]b4_locations_if([[

    /** Resolve the locations for each of the YYN1 states in *YYSTACKP,
     *  ending at YYS1.  Has no effect on previously resolved states.
     *  The first semantic option of a state is always chosen.  */
    void
    yyresolveLocations (glr_state &yys1, int yyn1)
    {
            if (0 < yyn1)
            {
                yyresolveLocations(*yys1.pred(), yyn1 - 1);
                if (!yys1.yyresolved)
                {
                    glr_stack_item yyrhsloc[1 + YYMAXRHS];
                    YYASSERT(yys1.firstVal() != YY_NULLPTR);
                    semantic_option &yyoption = *yys1.firstVal();
                    const int yynrhs          = yyrhsLength(yyoption.yyrule);
                    if (0 < yynrhs)
                    {
                        yyresolveLocations(*yyoption.state(), yynrhs);
                        const glr_state *yys = yyoption.state();
                        for (int yyn = yynrhs; yyn > 0; yyn -= 1)
                        {
                            yyrhsloc[yyn].getState().yyloc = yys->yyloc;
                            yys                            = yys->pred();
                        }
                    }
                    else
                    {
                        /* Both yyresolveAction and yyresolveLocations traverse the GSS
                           in reverse rightmost order.  It is only necessary to invoke
                           yyresolveLocations on a subforest for which yyresolveAction
                           would have been invoked next had an ambiguity not been
                           detected.  Thus the location of the previous state (but not
                           necessarily the previous state itself) is guaranteed to be
                           resolved already.  */
                        YY_IGNORE_NULL_DEREFERENCE_BEGIN
                        yyrhsloc[0].getState().yyloc = yyoption.state()->yyloc;
                        YY_IGNORE_NULL_DEREFERENCE_END
                    }
                    YYLLOC_DEFAULT((yys1.yyloc), yyrhsloc, yynrhs);
                }
            }
    }]])[

    /** If yytoken is empty, fetch the next token.  */
    void
    yyget_token ()
    {
]b4_parse_param_use()dnl
[      if (this->yyla.empty ())
        {
                YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
                try
#endif  // YY_EXCEPTIONS
                {]b4_token_ctor_if([[
              symbol_type yylookahead (]b4_yylex[);
              yyla.move (yylookahead);]], [[
              yyla.kind_ = yyparser.yytranslate_ (]b4_yylex[);]])[
                }
#if YY_EXCEPTIONS
                catch (const parser_type::syntax_error &yyexc)
                {
                    YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';]b4_locations_if([
              this->yyla.location = yyexc.location;])[
              yyparser.error (]b4_locations_if([this->yyla.location, ])[yyexc.what ());
              // Map errors caught in the scanner to the error token, so that error
              // handling is started.
              this->yyla.kind_ = ]b4_symbol(error, kind)[;
                }
        }
#endif  // YY_EXCEPTIONS
      if (this->yyla.kind () == ]b4_symbol(eof, kind)[)
        YYCDEBUG << "Now at end of input.\n";
      else
        YY_SYMBOL_PRINT ("Next token is", this->yyla.kind (), this->yyla.value, this->yyla.location);
    }


                                /* Bison grammar-table manipulation.  */

    /** The action to take in YYSTATE on seeing YYTOKEN.
     *  Result R means
     *    R < 0:  Reduce on rule -R.
     *    R = 0:  Error.
     *    R > 0:  Shift to state R.
     *  Set *YYCONFLICTS to a pointer into yyconfl to a 0-terminated list
     *  of conflicting reductions.
     */
    static int
    yygetLRActions (state_num yystate, symbol_kind_type yytoken, const short*& yyconflicts)
    {
            int yyindex = yypact[yystate] + yytoken;
      if (yytoken == ]b4_symbol(error, kind)[)
      {
          // This is the error token.
          yyconflicts = yyconfl;
          return 0;
      }
      else if (yy_is_defaulted_state(yystate) || yyindex < 0 || YYLAST < yyindex ||
               yycheck[yyindex] != yytoken)
      {
          yyconflicts = yyconfl;
          return -yydefact[yystate];
      }
      else if (!yytable_value_is_error(yytable[yyindex]))
      {
          yyconflicts = yyconfl + yyconflp[yyindex];
          return yytable[yyindex];
      }
      else
      {
          yyconflicts = yyconfl + yyconflp[yyindex];
          return 0;
      }
    }

    /** Compute post-reduction state.
     * \param yystate   the current state
     * \param yysym     the nonterminal to push on the stack
     */
    static state_num
    yyLRgotoState (state_num yystate, symbol_kind_type yysym)
    {
            const int yyr = yypgoto[yysym - YYNTOKENS] + yystate;
            if (0 <= yyr && yyr <= YYLAST && yycheck[yyr] == yystate)
                return yytable[yyr];
            else
                return yydefgoto[yysym - YYNTOKENS];
    }

    static bool
    yypact_value_is_default (state_num yystate)
    {
      return ]b4_table_value_equals([[pact]], [[yystate]], [b4_pact_ninf], [YYPACT_NINF])[;
    }

    static bool
    yytable_value_is_error (int yytable_value YY_ATTRIBUTE_UNUSED)
    {
      return ]b4_table_value_equals([[table]], [[yytable_value]], [b4_table_ninf], [YYTABLE_NINF])[;
    }

    static bool
    yy_is_shift_action (int yyaction) YY_NOEXCEPT
    {
            return 0 < yyaction;
    }

    static bool
    yy_is_error_action (int yyaction) YY_NOEXCEPT
    {
            return yyaction == 0;
    }

    /** Whether LR state YYSTATE has only a default reduction
     *  (regardless of token).  */
    static bool
    yy_is_defaulted_state (state_num yystate)
    {
            return yypact_value_is_default(yypact[yystate]);
    }

    /** The default reduction for YYSTATE, assuming it has one.  */
    static rule_num
    yy_default_action (state_num yystate)
    {
            return yydefact[yystate];
    }

                                    /* GLRStacks */

    /** Y0 and Y1 represent two possible actions to take in a given
     *  parsing state; return 0 if no combination is possible,
     *  1 if user-mergeable, 2 if Y0 is preferred, 3 if Y1 is preferred.  */
    static int
    yypreference (const semantic_option& y0, const semantic_option& y1)
    {
            const rule_num r0 = y0.yyrule, r1 = y1.yyrule;
            const int p0 = yydprec[r0], p1 = yydprec[r1];

            if (p0 == p1)
            {
                if (yymerger[r0] == 0 || yymerger[r0] != yymerger[r1])
                    return 0;
                else
                    return 1;
            }
            else if (p0 == 0 || p1 == 0)
                return 0;
            else if (p0 < p1)
                return 3;
            else if (p1 < p0)
                return 2;
            else
                return 0;
    }

]b4_parse_param_vars[
  };  // class ]b4_parser_class[::glr_stack
} // namespace ]b4_namespace_ref[

#if] b4_api_PREFIX[DEBUG
namespace
{
  void
  yypstack (const glr_stack& yystack, size_t yyk)
  {
    yystack.yypstack (create_state_set_index (static_cast<std::ptrdiff_t> (yyk)));
}

void yypdumpstack(const glr_stack &yystack)
{
    yystack.yypdumpstack();
}
}
#endif

]b4_namespace_open[
  /// Build a parser object.
  ]b4_parser_class::b4_parser_class[ (]b4_parse_param_decl[)]m4_ifset([b4_parse_param], [
    :])[
#if] b4_api_PREFIX[DEBUG
    ]m4_ifset([b4_parse_param], [  ], [ :])[yycdebug_ (&std::cerr)]m4_ifset([b4_parse_param], [,])[
#endif] b4_parse_param_cons[
  {}

  ]b4_parser_class::~b4_parser_class[ ()
  {}

  ]b4_parser_class[::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  int
  ]b4_parser_class[::operator() ()
  {
    return parse();
  }

  int
  ]b4_parser_class[::parse ()
  {
    glr_stack yystack(YYINITDEPTH, *this]b4_user_args[);
    return yystack.parse();
  }

]b4_parse_error_bmatch([custom\|detailed],
[[  const char *
  ]b4_parser_class[::symbol_name (symbol_kind_type yysymbol)
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
  }
]],
[simple],
[[#if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
  const char *
  ]b4_parser_class[::symbol_name (symbol_kind_type yysymbol)
  {
    return yytname_[yysymbol];
  }
#endif  // #if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
]],
[verbose],
[[  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  ]b4_parser_class[::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
    {
        std::string yyr;
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
                    yyr += *yyp;
                    break;

                case '"':
                    return yyr;
            }
    do_not_strip_quotes:;
    }

    return yystr;
  }

  std::string
  ]b4_parser_class[::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_(yytname_[yysymbol]);
  }
]])[

]b4_parse_error_bmatch([simple\|verbose],
[[#if ]b4_api_PREFIX[DEBUG]b4_tname_if([[ || 1]])[
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const ]b4_parser_class[::yytname_[] =
  {
  ]b4_tname[
  };
#endif
]])[

]b4_parse_error_bmatch([custom\|detailed\|verbose], [[
  // ]b4_parser_class[::context.
  ]b4_parser_class[::context::context (glr_stack& yystack, const symbol_type& yyla)
    : yystack_ (yystack)
    , yyla_ (yyla)
  {}

  int
  ]b4_parser_class[::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount   = 0;
    const int yyn = yypact[yystack_.firstTopState()->yylrState];
    if (!yystack_.yypact_value_is_default(yyn))
    {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for this
           state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        /* Stay within bounds of both yycheck and yytname.  */
        const int yychecklim = YYLAST - yyn + 1;
        const int yyxend     = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck[yyx + yyn] == yyx && yyx != ]b4_symbol(error, kind)[
              && !yystack_.yytable_value_is_error (yytable[yyx + yyn]))
          {
              if (!yyarg)
                  ++yycount;
              else if (yycount == yyargn)
                  return 0;
              else
                  yyarg[yycount++] = YY_CAST(symbol_kind_type, yyx);
          }
    }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = ]b4_symbol(empty, kind)[;
    return yycount;
  }

]])[

]b4_parse_error_bmatch([detailed\|verbose], [[
  int
  ]b4_parser_class[::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
    */

    if (!yyctx.lookahead().empty())
    {
        if (yyarg)
            yyarg[0] = yyctx.token();
        int yyn = yyctx.expected_tokens(yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
    }
    return 0;
  }

  // Generate an error message.
  std::string
  ]b4_parser_class[::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum
    {
        YYARGS_MAX = 5
    };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_(yyctx, yyarg, YYARGS_MAX);

    char const *yyformat = YY_NULLPTR;
    switch (yycount)
    {
#define YYCASE_(N, S) \
    case N:           \
        yyformat = S; \
        break
        default:  // Avoid compiler warnings.
            YYCASE_(0, YY_("syntax error"));
            YYCASE_(1, YY_("syntax error, unexpected %s"));
            YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
            YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
            YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
            YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const *yyp = yyformat; *yyp; ++yyp)
        if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
            yyres += symbol_name(yyarg[yyi++]);
            ++yyp;
        }
        else
            yyres += *yyp;
    return yyres;
  }]])[

  void
  ]b4_parser_class[::yy_destroy_ (const char* yymsg, symbol_kind_type yykind,
                           value_type& yyval]b4_locations_if([[,
                           location_type& yyloc]])[)
  {
    YY_USE(yyval);]b4_locations_if([[
    YY_USE (yyloc);]])[
    if (!yymsg)
      yymsg = "Deleting";
    ]b4_parser_class[& yyparser = *this;
    YY_USE (yyparser);
    YY_SYMBOL_PRINT (yymsg, yykind, yyval, yyloc);

    YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
    ]m4_do([m4_pushdef([b4_symbol_action], m4_defn([b4_symbol_action_for_yyval]))],
           [b4_symbol_actions([destructor])],
           [m4_popdef([b4_symbol_action])])[
    YY_IGNORE_MAYBE_UNINITIALIZED_END
  }

#if] b4_api_PREFIX[DEBUG
  /*--------------------.
  | Print this symbol.  |
  `--------------------*/

  void
  ]b4_parser_class[::yy_symbol_value_print_ (symbol_kind_type yykind,
                           const value_type& yyval]b4_locations_if([[,
                           const location_type& yyloc]])[) const
  {]b4_locations_if([[
    YY_USE (yyloc);]])[
    YY_USE (yyval);
    std::ostream& yyo = debug_stream ();
    YY_USE (yyo);
    ]m4_do([m4_pushdef([b4_symbol_action], m4_defn([b4_symbol_action_for_yyval]))],
           [b4_symbol_actions([printer])],
           [m4_popdef([b4_symbol_action])])[
  }

  void
  ]b4_parser_class[::yy_symbol_print_ (symbol_kind_type yykind,
                           const value_type& yyval]b4_locations_if([[,
                           const location_type& yyloc]])[) const
  {
    *yycdebug_ << (yykind < YYNTOKENS ? "token" : "nterm")
               << ' ' << symbol_name (yykind) << " ("]b4_locations_if([[
               << yyloc << ": "]])[;
    yy_symbol_value_print_ (yykind, yyval]b4_locations_if([[, yyloc]])[);
    *yycdebug_ << ')';
  }

  std::ostream&
  ]b4_parser_class[::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  ]b4_parser_class[::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  ]b4_parser_class[::debug_level_type
  ]b4_parser_class[::debug_level () const
  {
    return yydebug;
  }

  void
  ]b4_parser_class[::set_debug_level (debug_level_type l)
  {
    // Actually, it is yydebug which is really used.
    yydebug = l;
  }
#endif  // ]b4_api_PREFIX[DEBUG

]b4_token_ctor_if([], [b4_yytranslate_define([cc])])[

]b4_token_ctor_if([], [[
  /*---------.
  | symbol.  |
  `---------*/
]b4_public_types_define([cc])])[
]b4_namespace_close[]dnl
b4_epilogue[]dnl
b4_output_end
