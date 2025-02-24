                                                            -*- Autoconf -*-

# C++ skeleton for Bison

# Copyright (C) 2002-2021 Free Software Foundation, Inc.

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

# Sanity checks, before defaults installed by c.m4.
b4_percent_define_ifdef([[api.value.union.name]],
  [b4_complain_at(b4_percent_define_get_loc([[api.value.union.name]]),
                  [named %union is invalid in C++])])

b4_percent_define_default([[api.symbol.prefix]], [[S_]])

m4_include(b4_skeletonsdir/[c.m4])

b4_percent_define_check_kind([api.namespace], [code], [deprecated])
b4_percent_define_check_kind([api.parser.class], [code], [deprecated])


## ----- ##
## C++.  ##
## ----- ##

# b4_comment(TEXT, [PREFIX])
# --------------------------
# Put TEXT in comment. Prefix all the output lines with PREFIX.
m4_define([b4_comment],
[_b4_comment([$1], [$2// ], [$2// ])])


# b4_inline(hh|cc)
# ----------------
# Expand to `inline\n  ` if $1 is hh.
m4_define([b4_inline],
[m4_case([$1],
  [cc], [],
  [hh], [[inline
  ]],
  [m4_fatal([$0: invalid argument: $1])])])


# b4_cxx_portability
# ------------------
m4_define([b4_cxx_portability],
[#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif[]dnl
])


## ---------------- ##
## Default values.  ##
## ---------------- ##

b4_percent_define_default([[api.parser.class]], [[parser]])

# Don't do that so that we remember whether we're using a user
# request, or the default value.
#
# b4_percent_define_default([[api.location.type]], [[location]])

b4_percent_define_default([[api.filename.type]], [[const std::string]])
# Make it a warning for those who used betas of Bison 3.0.
b4_percent_define_default([[api.namespace]], m4_defn([b4_prefix]))

b4_percent_define_default([[define_location_comparison]],
                          [m4_if(b4_percent_define_get([[filename_type]]),
                                 [std::string], [[true]], [[false]])])



## ----------- ##
## Namespace.  ##
## ----------- ##

m4_define([b4_namespace_ref], [b4_percent_define_get([[api.namespace]])])


# Don't permit an empty b4_namespace_ref.  Any '::parser::foo' appended to it
# would compile as an absolute reference with 'parser' in the global namespace.
# b4_namespace_open would open an anonymous namespace and thus establish
# internal linkage.  This would compile.  However, it's cryptic, and internal
# linkage for the parser would be specified in all translation units that
# include the header, which is always generated.  If we ever need to permit
# internal linkage somehow, surely we can find a cleaner approach.
m4_if(m4_bregexp(b4_namespace_ref, [^[	 ]*$]), [-1], [],
[b4_complain_at(b4_percent_define_get_loc([[api.namespace]]),
                [[namespace reference is empty]])])

# Instead of assuming the C++ compiler will do it, Bison should reject any
# invalid b4_namespace_ref that would be converted to a valid
# b4_namespace_open.  The problem is that Bison doesn't always output
# b4_namespace_ref to uncommented code but should reserve the ability to do so
# in future releases without risking breaking any existing user grammars.
# Specifically, don't allow empty names as b4_namespace_open would just convert
# those into anonymous namespaces, and that might tempt some users.
m4_if(m4_bregexp(b4_namespace_ref, [::[	 ]*::]), [-1], [],
[b4_complain_at(b4_percent_define_get_loc([[api.namespace]]),
                [[namespace reference has consecutive "::"]])])
m4_if(m4_bregexp(b4_namespace_ref, [::[	 ]*$]), [-1], [],
[b4_complain_at(b4_percent_define_get_loc([[api.namespace]]),
                [[namespace reference has a trailing "::"]])])

m4_define([b4_namespace_open],
[b4_user_code([b4_percent_define_get_syncline([[api.namespace]])dnl
[namespace ]m4_bpatsubst(m4_dquote(m4_bpatsubst(m4_dquote(b4_namespace_ref),
                                                [^\(.\)[	 ]*::], [\1])),
                         [::], [ { namespace ])[ {]])])

m4_define([b4_namespace_close],
[b4_user_code([b4_percent_define_get_syncline([[api.namespace]])dnl
m4_bpatsubst(m4_dquote(m4_bpatsubst(m4_dquote(b4_namespace_ref[ ]),
                                    [^\(.\)[	 ]*\(::\)?\([^][:]\|:[^:]\)*],
                                    [\1])),
             [::\([^][:]\|:[^:]\)*], [} ])[} // ]b4_namespace_ref])])


## ------------- ##
## Token kinds.  ##
## ------------- ##


# b4_token_enums
# --------------
# Output the definition of the token kinds.
m4_define([b4_token_enums],
[[enum token_kind_type
      {
        ]b4_symbol([-2], [id])[ = -2,
]b4_symbol_foreach([b4_token_enum])dnl
[      };]dnl
])



## -------------- ##
## Symbol kinds.  ##
## -------------- ##

# b4_declare_symbol_enum
# ----------------------
# The definition of the symbol internal numbers as an enum.
# Defining YYEMPTY here is important: it forces the compiler
# to use a signed type, which matters for yytoken.
m4_define([b4_declare_symbol_enum],
[[enum symbol_kind_type
      {
        YYNTOKENS = ]b4_tokens_number[, ///< Number of tokens.
        ]b4_symbol(empty, kind_base)[ = -2,
]b4_symbol_foreach([      b4_symbol_enum])dnl
[      };]])



## ----------------- ##
## Semantic Values.  ##
## ----------------- ##



# b4_value_type_declare
# ---------------------
# Declare value_type.
m4_define([b4_value_type_declare],
[b4_value_type_setup[]dnl
[    /// Symbol semantic values.
]m4_bmatch(b4_percent_define_get_kind([[api.value.type]]),
[code],
[[    typedef ]b4_percent_define_get([[api.value.type]])[ value_type;]],
[m4_bmatch(b4_percent_define_get([[api.value.type]]),
[union\|union-directive],
[[    union value_type
    {
]b4_user_union_members[
    };]])])dnl
])


# b4_public_types_declare
# -----------------------
# Define the public types: token, semantic value, location, and so forth.
# Depending on %define token_lex, may be output in the header or source file.
m4_define([b4_public_types_declare],
[b4_glr2_cc_if(
[b4_value_type_declare],
[[#ifdef ]b4_api_PREFIX[STYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define ]b4_api_PREFIX[STYPE in C++, use %define api.value.type"
# endif
    typedef ]b4_api_PREFIX[STYPE value_type;
#else
]b4_value_type_declare[
#endif
    /// Backward compatibility (Bison 3.8).
    typedef value_type semantic_type;
]])[]b4_locations_if([
    /// Symbol locations.
    typedef b4_percent_define_get([[api.location.type]],
                                  [[location]]) location_type;])[

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (]b4_locations_if([const location_type& l, ])[const std::string& m)
        : std::runtime_error (m)]b4_locations_if([
        , location (l)])[
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())]b4_locations_if([
        , location (s.location)])[
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;]b4_locations_if([

      location_type location;])[
    };

    /// Token kinds.
    struct token
    {
      ]b4_token_enums[]b4_glr2_cc_if([], [[
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;]])[
    };

    /// Token kind, as returned by yylex.
    typedef token::token_kind_type token_kind_type;]b4_glr2_cc_if([], [[

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;]])[

    /// Symbol kinds.
    struct symbol_kind
    {
      ]b4_declare_symbol_enum[
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;
]])


# b4_symbol_type_define
# ---------------------
# Define symbol_type, the external type for symbols used for symbol
# constructors.
m4_define([b4_symbol_type_define],
[[    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value]b4_locations_if([ and location])[.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()]b4_locations_if([
        , location ()])[
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value (]b4_variant_if([], [std::move (that.value)]))b4_locations_if([
        , location (std::move (that.location))])[
      {]b4_variant_if([
        b4_symbol_variant([this->kind ()], [value], [move],
                          [std::move (that.value)])
      ])[}
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);]b4_variant_if([[

      /// Constructors for typed symbols.
]b4_type_foreach([b4_basic_symbol_constructor_define], [
])], [[
      /// Constructor for valueless symbols.
      basic_symbol (typename Base::kind_type t]b4_locations_if([,
                    YY_MOVE_REF (location_type) l])[);

      /// Constructor for symbols with semantic value.
      basic_symbol (typename Base::kind_type t,
                    YY_RVREF (value_type) v]b4_locations_if([,
                    YY_RVREF (location_type) l])[);
]])[
      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }

]b4_glr2_cc_if([[
      /// Copy assignment.
      basic_symbol& operator= (const basic_symbol& that)
      {
        Base::operator= (that);]b4_variant_if([[
        ]b4_symbol_variant([this->kind ()], [value], [copy],
                           [that.value])], [[
        value = that.value]])[;]b4_locations_if([[
        location = that.location;]])[
        return *this;
      }

      /// Move assignment.
      basic_symbol& operator= (basic_symbol&& that)
      {
        Base::operator= (std::move (that));]b4_variant_if([[
        ]b4_symbol_variant([this->kind ()], [value], [move],
                           [std::move (that.value)])], [[
        value = std::move (that.value)]])[;]b4_locations_if([[
        location = std::move (that.location);]])[
        return *this;
      }
]])[

      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {]b4_variant_if([[
        // User destructor.
        symbol_kind_type yykind = this->kind ();
        basic_symbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
]b4_symbol_foreach([b4_symbol_destructor])dnl
[       default:
          break;
        }

        // Value type destructor.
]b4_symbol_variant([[yykind]], [[value]], [[template destroy]])])[
        Base::clear ();
      }

]b4_parse_error_bmatch(
[custom\|detailed],
[[      /// The user-facing name of this symbol.
      const char *name () const YY_NOEXCEPT
      {
        return ]b4_parser_class[::symbol_name (this->kind ());
      }]],
[simple],
[[#if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
      /// The user-facing name of this symbol.
      const char *name () const YY_NOEXCEPT
      {
        return ]b4_parser_class[::symbol_name (this->kind ());
      }
#endif // #if ]b4_api_PREFIX[DEBUG || ]b4_token_table_flag[
]],
[verbose],
[[      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
      {
        return ]b4_parser_class[::symbol_name (this->kind ());
      }]])[]b4_glr2_cc_if([], [[

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;]])[

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      value_type value;]b4_locations_if([

      /// The location.
      location_type location;])[

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Default constructor.
      by_kind () YY_NOEXCEPT;

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that) YY_NOEXCEPT;
#endif

      /// Copy constructor.
      by_kind (const by_kind& that) YY_NOEXCEPT;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t) YY_NOEXCEPT;

]b4_glr2_cc_if([[
      /// Copy assignment.
      by_kind& operator= (const by_kind& that);

      /// Move assignment.
      by_kind& operator= (by_kind&& that);
]])[

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;]b4_glr2_cc_if([], [[

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;]])[

      /// The symbol kind.
      /// \a ]b4_symbol_prefix[YYEMPTY when empty.
      symbol_kind_type kind_;
    };]b4_glr2_cc_if([], [[

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;]])[

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {]b4_variant_if([[
      /// Superclass.
      typedef basic_symbol<by_kind> super_type;

      /// Empty symbol.
      symbol_type () YY_NOEXCEPT {}

      /// Constructor for valueless symbols, and symbols from each type.
]b4_type_foreach([_b4_symbol_constructor_define])dnl
    ])[};
]])


# b4_public_types_define(hh|cc)
# -----------------------------
# Provide the implementation needed by the public types.
m4_define([b4_public_types_define],
[[  // basic_symbol.
  template <typename Base>
  ]b4_parser_class[::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (]b4_variant_if([], [that.value]))b4_locations_if([
    , location (that.location)])[
  {]b4_variant_if([
    b4_symbol_variant([this->kind ()], [value], [copy],
                      [YY_MOVE (that.value)])
  ])[}

]b4_variant_if([], [[
  /// Constructor for valueless symbols.
  template <typename Base>
  ]b4_parser_class[::basic_symbol<Base>::basic_symbol (]b4_join(
          [typename Base::kind_type t],
          b4_locations_if([YY_MOVE_REF (location_type) l]))[)
    : Base (t)
    , value ()]b4_locations_if([
    , location (l)])[
  {}

  template <typename Base>
  ]b4_parser_class[::basic_symbol<Base>::basic_symbol (]b4_join(
          [typename Base::kind_type t],
          [YY_RVREF (value_type) v],
          b4_locations_if([YY_RVREF (location_type) l]))[)
    : Base (t)
    , value (]b4_variant_if([], [YY_MOVE (v)])[)]b4_locations_if([
    , location (YY_MOVE (l))])[
  {]b4_variant_if([[
    (void) v;
    ]b4_symbol_variant([this->kind ()], [value], [YY_MOVE_OR_COPY], [YY_MOVE (v)])])[}]])[

]b4_glr2_cc_if([], [[
  template <typename Base>
  ]b4_parser_class[::symbol_kind_type
  ]b4_parser_class[::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }
]])[

  template <typename Base>
  bool
  ]b4_parser_class[::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == ]b4_symbol(empty, kind)[;
  }

  template <typename Base>
  void
  ]b4_parser_class[::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    ]b4_variant_if([b4_symbol_variant([this->kind ()], [value], [move],
                                      [YY_MOVE (s.value)])],
                   [value = YY_MOVE (s.value);])[]b4_locations_if([
    location = YY_MOVE (s.location);])[
  }

  // by_kind.
  ]b4_inline([$1])b4_parser_class[::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (]b4_symbol(empty, kind)[)
  {}

#if 201103L <= YY_CPLUSPLUS
  ]b4_inline([$1])b4_parser_class[::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  ]b4_inline([$1])b4_parser_class[::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  ]b4_inline([$1])b4_parser_class[::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}

]b4_glr2_cc_if([[
  ]b4_inline([$1])]b4_parser_class[::by_kind&
  b4_parser_class[::by_kind::by_kind::operator= (const by_kind& that)
  {
    kind_ = that.kind_;
    return *this;
  }

  ]b4_inline([$1])]b4_parser_class[::by_kind&
  b4_parser_class[::by_kind::by_kind::operator= (by_kind&& that)
  {
    kind_ = that.kind_;
    that.clear ();
    return *this;
  }
]])[

  ]b4_inline([$1])[void
  ]b4_parser_class[::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = ]b4_symbol(empty, kind)[;
  }

  ]b4_inline([$1])[void
  ]b4_parser_class[::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  ]b4_inline([$1])[]b4_parser_class[::symbol_kind_type
  ]b4_parser_class[::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

]b4_glr2_cc_if([], [[
  ]b4_inline([$1])[]b4_parser_class[::symbol_kind_type
  ]b4_parser_class[::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }
]])[
]])


# b4_token_constructor_define
# ----------------------------
# Define make_FOO for all the token kinds.
# Use at class-level.  Redefined in variant.hh.
m4_define([b4_token_constructor_define], [])


# b4_yytranslate_define(cc|hh)
# ----------------------------
# Define yytranslate_.  Sometimes used in the header file ($1=hh),
# sometimes in the cc file.
m4_define([b4_yytranslate_define],
[  b4_inline([$1])b4_parser_class[::symbol_kind_type
  ]b4_parser_class[::yytranslate_ (int t) YY_NOEXCEPT
  {
]b4_api_token_raw_if(
[[    return static_cast<symbol_kind_type> (t);]],
[[    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const ]b4_int_type_for([b4_translate])[
    translate_table[] =
    {
  ]b4_translate[
    };
    // Last valid token kind.
    const int code_max = ]b4_code_max[;

    if (t <= 0)
      return symbol_kind::]b4_symbol_prefix[YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::]b4_symbol_prefix[YYUNDEF;]])[
  }
]])


# b4_lhs_value([TYPE])
# --------------------
m4_define([b4_lhs_value],
[b4_symbol_value([yyval], [$1])])


# b4_rhs_value(RULE-LENGTH, POS, [TYPE])
# --------------------------------------
# FIXME: Dead code.
m4_define([b4_rhs_value],
[b4_symbol_value([yysemantic_stack_@{($1) - ($2)@}], [$3])])


# b4_lhs_location()
# -----------------
# Expansion of @$.
m4_define([b4_lhs_location],
[(yyloc)])


# b4_rhs_location(RULE-LENGTH, POS)
# ---------------------------------
# Expansion of @POS, where the current rule has RULE-LENGTH symbols
# on RHS.
m4_define([b4_rhs_location],
[(yylocation_stack_@{($1) - ($2)@})])


# b4_parse_param_decl
# -------------------
# Extra formal arguments of the constructor.
# Change the parameter names from "foo" into "foo_yyarg", so that
# there is no collision bw the user chosen attribute name, and the
# argument name in the constructor.
m4_define([b4_parse_param_decl],
[m4_ifset([b4_parse_param],
          [m4_map_sep([b4_parse_param_decl_1], [, ], [b4_parse_param])])])

m4_define([b4_parse_param_decl_1],
[$1_yyarg])



# b4_parse_param_cons
# -------------------
# Extra initialisations of the constructor.
m4_define([b4_parse_param_cons],
          [m4_ifset([b4_parse_param],
                    [
      b4_cc_constructor_calls(b4_parse_param)])])
m4_define([b4_cc_constructor_calls],
          [m4_map_sep([b4_cc_constructor_call], [,
      ], [$@])])
m4_define([b4_cc_constructor_call],
          [$2 ($2_yyarg)])

# b4_parse_param_vars
# -------------------
# Extra instance variables.
m4_define([b4_parse_param_vars],
          [m4_ifset([b4_parse_param],
                    [
    // User arguments.
b4_cc_var_decls(b4_parse_param)])])
m4_define([b4_cc_var_decls],
          [m4_map_sep([b4_cc_var_decl], [
], [$@])])
m4_define([b4_cc_var_decl],
          [    $1;])


## ---------##
## Values.  ##
## ---------##

# b4_yylloc_default_define
# ------------------------
# Define YYLLOC_DEFAULT.
m4_define([b4_yylloc_default_define],
[[/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif
]])

## -------- ##
## Checks.  ##
## -------- ##

b4_token_ctor_if([b4_variant_if([],
  [b4_fatal_at(b4_percent_define_get_loc(api.token.constructor),
               [cannot use '%s' without '%s'],
               [%define api.token.constructor],
               [%define api.value.type variant]))])])
