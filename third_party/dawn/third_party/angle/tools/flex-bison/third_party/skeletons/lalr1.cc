#C++ skeleton for Bison

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

#parse.lac
b4_percent_define_default([[parse.lac]], [[none]])
b4_percent_define_check_values([[[[parse.lac]], [[full]], [[none]]]])
b4_define_flag_if([lac])
m4_define([b4_lac_flag],
          [m4_if(b4_percent_define_get([[parse.lac]]),
                 [none], [[0]], [[1]])])

#b4_tname_if(TNAME - NEEDED, TNAME - NOT - NEEDED)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
m4_define([b4_tname_if],
[m4_case(b4_percent_define_get([[parse.error]]),
         [verbose],         [$1],
         [b4_token_table_if([$1],
                            [$2])])])

#b4_integral_parser_table_declare(TABLE - NAME, CONTENT, COMMENT)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Declare "parser::yy<TABLE-NAME>_" whose contents is CONTENT.
m4_define([b4_integral_parser_table_declare],
[m4_ifval([$3], [b4_comment([$3], [    ])
])dnl
    static const b4_int_type_for([$2]) yy$1_[[]];dnl
])

#b4_integral_parser_table_define(TABLE - NAME, CONTENT, COMMENT)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Define "parser::yy<TABLE-NAME>_" whose contents is CONTENT.
m4_define([b4_integral_parser_table_define],
[  const b4_int_type_for([$2])
  b4_parser_class::yy$1_[[]] =
  {
  $2
  };dnl
])

#b4_symbol_kind(NUM)
#-- -- -- -- -- -- -- -- -- -
m4_define([b4_symbol_kind],
[symbol_kind::b4_symbol_kind_base($@)])

#b4_symbol_value_template(VAL, SYMBOL - NUM, [TYPE])
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Same as b4_symbol_value, but used in a template method.It makes
#a difference when using variants.Note that b4_value_type_setup_union
#overrides b4_symbol_value, so we must override it again.
m4_copy([b4_symbol_value], [b4_symbol_value_template])
m4_append([b4_value_type_setup_union],
[m4_copy_force([b4_symbol_value_union], [b4_symbol_value_template])])

#b4_lhs_value(SYMBOL - NUM, [TYPE])
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#See README.
m4_define([b4_lhs_value],
[b4_symbol_value([yylhs.value], [$1], [$2])])

#b4_lhs_location()
#-- -- -- -- -- -- -- -- -
#Expansion of @$.
m4_define([b4_lhs_location],
[yylhs.location])

#b4_rhs_data(RULE - LENGTH, POS)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#See README.
m4_define([b4_rhs_data],
[yystack_@{b4_subtract($@)@}])

#b4_rhs_state(RULE - LENGTH, POS)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#The state corresponding to the symbol #POS, where the current
#rule has RULE - LENGTH symbols on RHS.
m4_define([b4_rhs_state],
[b4_rhs_data([$1], [$2]).state])

#b4_rhs_value(RULE - LENGTH, POS, SYMBOL - NUM, [TYPE])
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#See README.
m4_define([_b4_rhs_value],
[b4_symbol_value([b4_rhs_data([$1], [$2]).value], [$3], [$4])])

m4_define([b4_rhs_value],
[b4_percent_define_ifdef([api.value.automove],
                         [YY_MOVE (_b4_rhs_value($@))],
                         [_b4_rhs_value($@)])])

#b4_rhs_location(RULE - LENGTH, POS)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Expansion of @POS, where the current rule has RULE - LENGTH symbols
#on RHS.
m4_define([b4_rhs_location],
[b4_rhs_data([$1], [$2]).location])

#b4_symbol_action(SYMBOL - NUM, KIND)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#Run the action KIND(destructor or printer) for SYMBOL - NUM.
#Same as in C, but using references instead of pointers.
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

#b4_yylex
#-- -- -- --
#Call yylex.
m4_define([b4_yylex],
[b4_token_ctor_if(
[b4_function_call([yylex],
                  [symbol_type], m4_ifdef([b4_lex_param], b4_lex_param))],
[b4_function_call([yylex], [int],
                  [[value_type *], [&yyla.value]][]dnl
b4_locations_if([, [[location_type *], [&yyla.location]]])dnl
m4_ifdef([b4_lex_param], [, ]b4_lex_param))])])


m4_pushdef([b4_copyright_years],
           [2002-2015, 2018-2021])

m4_define([b4_parser_class],
          [b4_percent_define_get([[api.parser.class]])])

b4_bison_locations_if([# Backward compatibility.
   m4_define([b4_location_constructors])
   m4_include(b4_skeletonsdir/[location.cc])])
m4_include(b4_skeletonsdir/[stack.hh])
b4_variant_if([m4_include(b4_skeletonsdir/[variant.hh])])

#b4_shared_declarations(hh | cc)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Declaration that might either go into the header(if --header, $1 = hh)
# or in the implementation file.
m4_define([b4_shared_declarations],
[b4_percent_code_get([[requires]])[
]b4_parse_assert_if([# include <cassert>])[
#include <cstdlib>  // std::abort
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

]b4_cxx_portability[
]m4_ifdef([b4_location_include],
          [[# include ]b4_location_include])[
]b4_variant_if([b4_variant_includes])[

]b4_attribute_define[
]b4_cast_define[
]b4_null_define[

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
    /// Build a parser object.
    ]b4_parser_class[ (]b4_parse_param_decl[);
    virtual ~]b4_parser_class[ ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    ]b4_parser_class[ (const ]b4_parser_class[&) = delete;
    /// Non copyable.
    ]b4_parser_class[& operator= (const ]b4_parser_class[&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if] b4_api_PREFIX[DEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.]b4_locations_if([[
    /// \param loc    where the syntax error is found.]])[
    /// \param msg    a description of the syntax error.
    virtual void error (]b4_locations_if([[const location_type& loc, ]])[const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

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
      context (const ]b4_parser_class[& yyparser, const symbol_type& yyla);
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
      const ]b4_parser_class[& yyparser_;
      const symbol_type& yyla_;
    };
]])[
  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    ]b4_parser_class[ (const ]b4_parser_class[&);
    /// Non copyable.
    ]b4_parser_class[& operator= (const ]b4_parser_class[&);
#endif
]b4_lac_if([[
    /// Check the lookahead yytoken.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_check_ (symbol_kind_type yytoken) const;
    /// Establish the initial context if no initial context currently exists.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_establish_ (symbol_kind_type yytoken);
    /// Discard any previous initial lookahead context because of event.
    /// \param event  the event which caused the lookahead to be discarded.
    ///               Only used for debbuging output.
    void yy_lac_discard_ (const char* event);]])[

    /// Stored state numbers (used for stacks).
    typedef ]b4_int_type(0, m4_eval(b4_states_number - 1))[ state_type;
]b4_parse_error_bmatch(
[custom], [[
    /// Report a syntax error
    /// \param yyctx     the context in which the error occurred.
    void report_syntax_error (const context& yyctx) const;]],
[detailed\|verbose], [[
    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;]])[
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT;

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT;

    static const ]b4_int_type(b4_pact_ninf, b4_pact_ninf)[ yypact_ninf_;
    static const ]b4_int_type(b4_table_ninf, b4_table_ninf)[ yytable_ninf_;

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

    // Tables.
]b4_parser_tables_declare[

#if] b4_api_PREFIX[DEBUG
]b4_integral_parser_table_declare([rline], [b4_rline],
     [[YYRLINE[YYN] -- Source line where rule number YYN was defined.]])[
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
        /// Default constructor.
        by_state() YY_NOEXCEPT;

        /// The symbol kind as needed by the constructor.
        typedef state_type kind_type;

        /// Constructor.
        by_state(kind_type s) YY_NOEXCEPT;

        /// Copy constructor.
        by_state(const by_state &that) YY_NOEXCEPT;

        /// Record that this symbol is empty.
        void clear() YY_NOEXCEPT;

        /// Steal the symbol kind from \a that.
        void move(by_state & that);

        /// The symbol kind (corresponding to \a state).
        /// \a ]b4_symbol(empty, kind)[ when empty.
        symbol_kind_type kind() const YY_NOEXCEPT;

        /// The state number used to denote an empty symbol.
        /// We use the initial state, as it does not have a value.
        enum
        {
            empty_state = 0
        };

        /// The state.
        /// \a empty when empty.
        state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
        /// Superclass.
        typedef basic_symbol<by_state> super_type;
        /// Construct an empty symbol.
        stack_symbol_type();
        /// Move or copy construction.
        stack_symbol_type(YY_RVREF(stack_symbol_type) that);
        /// Steal the contents from \a sym to build this.
        stack_symbol_type(state_type s, YY_MOVE_REF(symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
        /// Assignment, needed by push_back by some old implementations.
        /// Moves the contents of that.
        stack_symbol_type &operator=(stack_symbol_type &that);

        /// Assignment, needed by push_back by other implementations.
        /// Needed by some other old implementations.
        stack_symbol_type &operator=(const stack_symbol_type &that);
#endif
    };

]b4_stack_define[

    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;]b4_lac_if([[
    /// The stack for LAC.
    /// Logically, the yy_lac_stack's lifetime is confined to the function
    /// yy_lac_check_. We just store it as a member of this class to hold
    /// on to the memory and to avoid frequent reallocations.
    /// Since yy_lac_check_ is const, this member must be mutable.
    mutable std::vector<state_type> yylac_stack_;
    /// Whether an initial LAC context was established.
    bool yy_lac_established_;
]])[

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1) YY_NOEXCEPT;

    /// Constants.
    enum
    {
      yylast_ = ]b4_last[,     ///< Last index in yytable_.
      yynnts_ = ]b4_nterms_number[,  ///< Number of nonterminal symbols.
      yyfinal_ = ]b4_final_state_number[ ///< Termination state number.
    };

]b4_parse_param_vars[
]b4_percent_code_get([[yy_bison_internal_hook]])[
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
]b4_copyright([Skeleton interface for Bison LALR(1) parsers in C++])[

/**
 ** \file ]b4_spec_mapped_header_file[
 ** Define the ]b4_namespace_ref[::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

]b4_disclaimer[
]b4_cpp_guard_open([b4_spec_mapped_header_file])[
]b4_shared_declarations(hh)[
]b4_cpp_guard_close([b4_spec_mapped_header_file])[
]b4_output_end[
]])[

#-- -- -- -- -- -- -- -- -- -- - #
#Implementation file.#
#-- -- -- -- -- -- -- -- -- -- - #

]b4_output_begin([b4_parser_file_name])[
]b4_copyright([Skeleton implementation for Bison LALR(1) parsers in C++])[
]b4_disclaimer[
]b4_percent_code_get([[top]])[]dnl
m4_if(b4_prefix, [yy], [],
[
// Take the name prefix into account.
[#]define yylex   b4_prefix[]lex])[

]b4_user_pre_prologue[

]b4_header_if([[#include "@basename(]b4_spec_header_file[@)"]],
               [b4_shared_declarations([cc])])[

]b4_user_post_prologue[
]b4_percent_code_get[

#ifndef YY_
#    if defined YYENABLE_NLS && YYENABLE_NLS
#        if ENABLE_NLS
#            include <libintl.h>  // FIXME: INFRINGES ON USER NAME SPACE.
#            define YY_(msgid) dgettext("bison-runtime", msgid)
#        endif
#    endif
#    ifndef YY_
#        define YY_(msgid) msgid
#    endif
#endif
]b4_has_translations_if([
#ifndef N_
#    define N_(Msgid) Msgid
#endif
])[

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
#    if defined __GNUC__ && !defined __EXCEPTIONS
#        define YY_EXCEPTIONS 0
#    else
#        define YY_EXCEPTIONS 1
#    endif
#endif

]b4_locations_if([dnl
[#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
]b4_yylloc_default_define])[

// Enable debugging if requested.
#if] b4_api_PREFIX[DEBUG

// A pseudo ostream that takes yydebug_ into account.
#    define YYCDEBUG  \
        if (yydebug_) \
        (*yycdebug_)

#    define YY_SYMBOL_PRINT(Title, Symbol)     \
        do                                     \
        {                                      \
            if (yydebug_)                      \
            {                                  \
                *yycdebug_ << Title << ' ';    \
                yy_print_(*yycdebug_, Symbol); \
                *yycdebug_ << '\n';            \
            }                                  \
        } while (false)

#    define YY_REDUCE_PRINT(Rule)       \
        do                              \
        {                               \
            if (yydebug_)               \
                yy_reduce_print_(Rule); \
        } while (false)

#    define YY_STACK_PRINT()       \
        do                         \
        {                          \
            if (yydebug_)          \
                yy_stack_print_(); \
        } while (false)

#else  // !]b4_api_PREFIX[DEBUG

#    define YYCDEBUG \
        if (false)   \
        std::cerr
#    define YY_SYMBOL_PRINT(Title, Symbol) YY_USE(Symbol)
#    define YY_REDUCE_PRINT(Rule) static_cast<void>(0)
#    define YY_STACK_PRINT() static_cast<void>(0)

#endif  // !]b4_api_PREFIX[DEBUG

#define yyerrok (yyerrstatus_ = 0)
#define yyclearin (yyla.clear())

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYRECOVERING() (!!yyerrstatus_)

]b4_namespace_open[
  /// Build a parser object.
  ]b4_parser_class::b4_parser_class[ (]b4_parse_param_decl[)
#if] b4_api_PREFIX[DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr)]b4_lac_if([,], [m4_ifset([b4_parse_param], [,])])[
#else
]b4_lac_if([    :], [m4_ifset([b4_parse_param], [    :])])[
#endif] b4_lac_if([[
      yy_lac_established_ (false)]m4_ifset([b4_parse_param], [,])])[]b4_parse_param_cons[
  {}

  ]b4_parser_class::~b4_parser_class[ ()
  {}

  ]b4_parser_class[::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

]b4_token_ctor_if([], [b4_public_types_define([cc])])[

  // by_state.
  ]b4_parser_class[::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  ]b4_parser_class[::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  ]b4_parser_class[::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  ]b4_parser_class[::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear();
  }

  ]b4_parser_class[::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  ]b4_parser_class[::symbol_kind_type
  ]b4_parser_class[::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return ]b4_symbol(empty, kind)[;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  ]b4_parser_class[::stack_symbol_type::stack_symbol_type ()
  {}

  ]b4_parser_class[::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state)]b4_variant_if([], [, YY_MOVE (that.value)])b4_locations_if([, YY_MOVE (that.location)])[)
  {]b4_variant_if([
    b4_symbol_variant([that.kind ()],
                      [value], [YY_MOVE_OR_COPY], [YY_MOVE (that.value)])])[
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  ]b4_parser_class[::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s]b4_variant_if([], [, YY_MOVE (that.value)])[]b4_locations_if([, YY_MOVE (that.location)])[)
  {]b4_variant_if([
    b4_symbol_variant([that.kind ()],
                      [value], [move], [YY_MOVE (that.value)])])[
    // that is emptied.
    that.kind_ = ]b4_symbol(empty, kind)[;
  }

#if YY_CPLUSPLUS < 201103L
  ]b4_parser_class[::stack_symbol_type&
  ]b4_parser_class[::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    ]b4_variant_if([b4_symbol_variant([that.kind ()],
                                      [value], [copy], [that.value])],
                   [[value = that.value;]])[]b4_locations_if([
    location = that.location;])[
    return *this;
  }

  ]b4_parser_class[::stack_symbol_type&
  ]b4_parser_class[::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    ]b4_variant_if([b4_symbol_variant([that.kind ()],
                                      [value], [move], [that.value])],
                   [[value = that.value;]])[]b4_locations_if([
    location = that.location;])[
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  ]b4_parser_class[::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
        YY_SYMBOL_PRINT(yymsg, yysym);]b4_variant_if([], [

    // User destructor.
    b4_symbol_actions([destructor], [yysym.kind ()])])[
  }

#if] b4_api_PREFIX[DEBUG
  template <typename Base>
  void
  ]b4_parser_class[::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream &yyoutput = yyo;
    YY_USE(yyoutput);
    if (yysym.empty())
        yyo << "empty symbol";
    else
    {
        symbol_kind_type yykind = yysym.kind();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("]b4_locations_if([
            << yysym.location << ": "])[;
        ]b4_symbol_actions([printer])[
        yyo << ')';
    }
  }
#endif

  void
  ]b4_parser_class[::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
        YY_SYMBOL_PRINT(m, sym);
    yystack_.push(YY_MOVE(sym));
  }

  void
  ]b4_parser_class[::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_(m, stack_symbol_type(s, std::move(sym)));
#else
    stack_symbol_type ss(s, sym);
    yypush_(m, ss);
#endif
  }

  void
  ]b4_parser_class[::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop(n);
  }

#if] b4_api_PREFIX[DEBUG
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
    return yydebug_;
  }

  void
  ]b4_parser_class[::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif  // ]b4_api_PREFIX[DEBUG

  ]b4_parser_class[::state_type
  ]b4_parser_class[::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
        return yytable_[yyr];
    else
        return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  ]b4_parser_class[::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  ]b4_parser_class[::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  ]b4_parser_class[::operator() ()
  {
    return parse();
  }

  int
  ]b4_parser_class[::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_     = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;]b4_locations_if([[

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];]])[

    /// The return value of parse ().
    int yyresult;]b4_lac_if([[

    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yy_lac_discard_ ("init");]])[

#if YY_EXCEPTIONS
    try
#endif  // YY_EXCEPTIONS
      {
        YYCDEBUG << "Starting parse\n";

]m4_ifdef([b4_initial_action], [
b4_dollar_pushdef([yyla.value], [], [], [yyla.location])dnl
    b4_user_initial_action
b4_dollar_popdef])[]dnl

  [  /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
            YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
            try
#endif  // YY_EXCEPTIONS
            {]b4_token_ctor_if([[
            symbol_type yylookahead (]b4_yylex[);
            yyla.move (yylookahead);]], [[
            yyla.kind_ = yytranslate_ (]b4_yylex[);]])[
            }
#if YY_EXCEPTIONS
            catch (const syntax_error &yyexc)
            {
                YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
                error(yyexc);
                goto yyerrlab1;
            }
#endif  // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == ]b4_symbol(error, kind)[)
    {
            // The scanner already issued an error message, process directly
            // to error recovery.  But do not keep the error token as
            // lookahead, it is too special and may lead us to an endless
            // loop in error recovery. */
      yyla.kind_ = ]b4_symbol(undef, kind)[;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {]b4_lac_if([[
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;]])[
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
            if (yy_table_value_is_error_(yyn))
                goto yyerrlab;]b4_lac_if([[
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;
]])[
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));]b4_lac_if([[
    yy_lac_discard_ ("shift");]])[
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
            stack_symbol_type yylhs;
            yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);]b4_variant_if([[
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      ]b4_symbol_variant([[yyr1_@{yyn@}]], [yylhs.value], [emplace])], [[
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_@{yylen - 1@}.value;
      else
        yylhs.value = yystack_@{0@}.value;]])[
]b4_locations_if([dnl
[
      // Default location.
      {
                stack_type::slice range(yystack_, yylen);
                YYLLOC_DEFAULT(yylhs.location, range, yylen);
                yyerror_range[1].location = yylhs.location;
      }]])[

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif  // YY_EXCEPTIONS
        {
                switch (yyn)
                {
]b4_user_actions[
            default:
              break;
                }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
                YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
                error(yyexc);
                YYERROR;
        }
#endif  // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
            ++yynerrs_;]b4_parse_error_case(
                  [simple], [[
        std::string msg = YY_("syntax error");
        error (]b4_join(b4_locations_if([yyla.location]), [[YY_MOVE (msg)]])[);]],
                  [custom], [[
        context yyctx (*this, yyla);
        report_syntax_error (yyctx);]],
                  [[
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (]b4_join(b4_locations_if([yyla.location]), [[YY_MOVE (msg)]])[);]])[
      }

]b4_locations_if([[
    yyerror_range[1].location = yyla.location;]])[
    if (yyerrstatus_ == 3)
      {
            /* If just tried and failed to reuse lookahead token after an
               error, discard it.  */

            // Return failure if at end of input.
        if (yyla.kind () == ]b4_symbol(eof, kind)[)
            YYABORT;
        else if (!yyla.empty())
        {
            yy_destroy_("Error: discarding", yyla);
            yyla.clear();
        }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
            yyn = yypact_[+yystack_[0].state];
            if (!yy_pact_value_is_default_(yyn))
            {
            yyn += ]b4_symbol(error, kind)[;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == ]b4_symbol(error, kind)[)
              {
                    yyn = yytable_[yyn];
                    if (0 < yyn)
                        break;
              }
            }

            // Pop the current state because it cannot handle the error token.
            if (yystack_.size() == 1)
                YYABORT;
]b4_locations_if([[
        yyerror_range[1].location = yystack_[0].location;]])[
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
            stack_symbol_type error_token;
]b4_locations_if([[
      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);]])[

      // Shift the error token.]b4_lac_if([[
      yy_lac_discard_ ("error recovery");]])[
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
            yy_destroy_("Cleanup: popping", yystack_[0]);
            yypop_();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty())
            yy_destroy_(YY_NULLPTR, yyla);

        while (1 < yystack_.size())
        {
            yy_destroy_(YY_NULLPTR, yystack_[0]);
            yypop_();
        }
        throw;
      }
#endif  // YY_EXCEPTIONS
  }

  void
  ]b4_parser_class[::error (const syntax_error& yyexc)
  {
    error (]b4_join(b4_locations_if([yyexc.location]),
                    [[yyexc.what ()]])[);
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

]b4_parse_error_bmatch([custom\|detailed\|verbose], [[
  // ]b4_parser_class[::context.
  ]b4_parser_class[::context::context (const ]b4_parser_class[& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  ]b4_parser_class[::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;
]b4_lac_if([[
#if] b4_api_PREFIX[DEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST(symbol_kind_type, yyx);
        if (yysym != ]b4_symbol(error, kind)[
            && yysym != ]b4_symbol(undef, kind)[
            && yyparser_.yy_lac_check_ (yysym))
        {
            if (!yyarg)
                ++yycount;
            else if (yycount == yyargn)
                return 0;
            else
                yyarg[yycount++] = yysym;
        }
      }]], [[
    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend     = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != ]b4_symbol(error, kind)[
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
          {
              if (!yyarg)
                  ++yycount;
              else if (yycount == yyargn)
                  return 0;
              else
                  yyarg[yycount++] = YY_CAST(symbol_kind_type, yyx);
          }
      }
]])[
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = ]b4_symbol(empty, kind)[;
    return yycount;
  }

]])[

]b4_lac_if([[
  bool
  ]b4_parser_class[::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear();
    // Reduce until we encounter a shift and thereby accept the token.
#if] b4_api_PREFIX[DEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name(yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
    {
        state_type top_state =
            (yylac_stack_.empty() ? yystack_[lac_top].state : yylac_stack_.back());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_(yyrule) || (yyrule += yytoken) < 0 || yylast_ < yyrule ||
            yycheck_[yyrule] != yytoken)
        {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
            {
                YYCDEBUG << " Err\n";
                return false;
            }
        }
        else
        {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_(yyrule))
            {
                YYCDEBUG << " Err\n";
                return false;
            }
            if (0 < yyrule)
            {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
            }
            yyrule = -yyrule;
        }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
            std::ptrdiff_t yylen = yyr2_[yyrule];
            // First pop from the LAC stack as many tokens as possible.
            std::ptrdiff_t lac_size = std::ptrdiff_t(yylac_stack_.size());
            if (yylen < lac_size)
            {
                yylac_stack_.resize(std::size_t(lac_size - yylen));
                yylen = 0;
            }
            else if (lac_size)
            {
                yylac_stack_.clear();
                yylen -= lac_size;
            }
            // Only afterwards look at the main stack.
            // We simulate popping elements by incrementing lac_top.
            lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty() ? yystack_[lac_top].state : yylac_stack_.back());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_(top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int(state);
        yylac_stack_.push_back(state);
    }
  }

  // Establish the initial context if no initial context currently exists.
  bool
  ]b4_parser_class[::yy_lac_establish_ (symbol_kind_type yytoken)
  {
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

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (yy_lac_established_)
        return true;
    else
    {
#if] b4_api_PREFIX[DEBUG
        YYCDEBUG << "LAC: initial context established for " << symbol_name(yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_(yytoken);
    }
  }

  // Discard any previous initial lookahead context.
  void
  ]b4_parser_class[::yy_lac_discard_ (const char* event)
  {
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
    if (yy_lac_established_)
    {
        YYCDEBUG << "LAC: initial context discarded due to " << event << '\n';
        yy_lac_established_ = false;
    }
  }]])[

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
         yyla.  (However, yyla is currently not documented for users.)]b4_lac_if([[
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.]], [[
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.]])[
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


  const ]b4_int_type(b4_pact_ninf, b4_pact_ninf) b4_parser_class::yypact_ninf_ = b4_pact_ninf[;

  const ]b4_int_type(b4_table_ninf, b4_table_ninf) b4_parser_class::yytable_ninf_ = b4_table_ninf[;

]b4_parser_tables_define[

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

#if] b4_api_PREFIX[DEBUG][
]b4_integral_parser_table_define([rline], [b4_rline])[

  void
  ]b4_parser_class[::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator i = yystack_.begin(), i_end = yystack_.end(); i != i_end; ++i)
        *yycdebug_ << ' ' << int(i->state);
    *yycdebug_ << '\n';
  }

  void
  ]b4_parser_class[::yy_reduce_print_ (int yyrule) const
  {
    int yylno  = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1 << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       ]b4_rhs_data(yynrhs, yyi + 1)[);
  }
#endif  // ]b4_api_PREFIX[DEBUG

]b4_token_ctor_if([], [b4_yytranslate_define([cc])])[
]b4_namespace_close[
]b4_epilogue[]dnl
b4_output_end


m4_popdef([b4_copyright_years])dnl
