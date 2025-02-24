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

#This skeleton produces a C++ class that encapsulates a C glr parser.
#This is in order to reduce the maintenance burden.The glr.c
#skeleton is clean and pure enough so that there are no real
#problems.The C++ interface is the same as that of lalr1.cc.In
#fact, glr.c can replace yacc.c without the user noticing any
#difference, and similarly for glr.cc replacing lalr1.cc.
#
#The passing of parse - params
#
#The additional arguments are stored as members of the parser
#object, yyparser.The C routines need to carry yyparser
#throughout the C parser; that's easy: make yyparser an
#additional parse - param.But because the C++ skeleton needs to
#know the "real" original parse - param, we save them
#(b4_parse_param_orig).Note that b4_parse_param is overquoted
#(and c.m4 strips one level of quotes).This is a PITA, and
#explains why there are so many levels of quotes.
#
#The locations
#
#We use location.cc just like lalr1.cc, but because glr.c stores
#the locations in a union, the position and location classes
#must not have a constructor.Therefore, contrary to lalr1.cc, we
#must not define "b4_location_constructors".As a consequence the
#user must initialize the first positions(in particular the
#filename member).

#We require a pure interface.
m4_define([b4_pure_flag], [1])

m4_include(b4_skeletonsdir/[c++.m4])
b4_bison_locations_if([m4_include(b4_skeletonsdir/[location.cc])])

m4_define([b4_parser_class],
          [b4_percent_define_get([[api.parser.class]])])

#Save the parse parameters.
m4_define([b4_parse_param_orig], m4_defn([b4_parse_param]))

#b4_parse_param_wrap
#-- -- -- -- -- -- -- -- -- -
#New ones.
m4_ifset([b4_parse_param],
[m4_define([b4_parse_param_wrap],
           [[b4_namespace_ref::b4_parser_class[& yyparser], [[yyparser]]],]
m4_defn([b4_parse_param]))],
[m4_define([b4_parse_param_wrap],
           [[b4_namespace_ref::b4_parser_class[& yyparser], [[yyparser]]]])
])

#b4_yy_symbol_print_define
#-- -- -- -- -- -- -- -- -- -- -- -- -
#Bypass the default implementation to generate the "yy_symbol_print"
# and "yy_symbol_value_print" functions.
m4_define([b4_yy_symbol_print_define],
[[/*--------------------.
| Print this symbol.  |
`--------------------*/

static void
yy_symbol_print (FILE *, ]b4_namespace_ref::b4_parser_class[::symbol_kind_type yytoken,
                 const ]b4_namespace_ref::b4_parser_class[::value_type *yyvaluep]b4_locations_if([[,
                 const ]b4_namespace_ref::b4_parser_class[::location_type *yylocationp]])[]b4_user_formals[)
{
]b4_parse_param_use[]dnl
[  yyparser.yy_symbol_print_ (yytoken, yyvaluep]b4_locations_if([, yylocationp])[);
}
]])[

#Hijack the initial action to initialize the locations.
]b4_bison_locations_if([m4_define([b4_initial_action],
[yylloc.initialize ();]m4_ifdef([b4_initial_action], [
m4_defn([b4_initial_action])]))])[

#Hijack the post prologue to declare yyerror.
]m4_append([b4_post_prologue],
[b4_syncline([@oline@], [@ofile@])dnl
[static void
yyerror (]b4_locations_if([[const ]b4_namespace_ref::b4_parser_class[::location_type *yylocationp,
         ]])[]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param),
         ])[const char* msg);]])[

#Inserted before the epilogue to define implementations(yyerror, parser member
#functions etc.).
]m4_define([b4_glr_cc_pre_epilogue],
[b4_syncline([@oline@], [@ofile@])dnl
[
/*------------------.
| Report an error.  |
`------------------*/

static void
yyerror (]b4_locations_if([[const ]b4_namespace_ref::b4_parser_class[::location_type *yylocationp,
         ]])[]m4_ifset([b4_parse_param], [b4_formals(b4_parse_param),
         ])[const char* msg)
{
]b4_parse_param_use[]dnl
[  yyparser.error (]b4_locations_if([[*yylocationp, ]])[msg);
}


]b4_namespace_open[
]dnl In this section, the parse params are the original parse_params.
m4_pushdef([b4_parse_param], m4_defn([b4_parse_param_orig]))dnl
[  /// Build a parser object.
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
    return ::yy_parse_impl (*this]b4_user_args[);
  }

#if] b4_api_PREFIX[DEBUG
  /*--------------------.
  | Print this symbol.  |
  `--------------------*/

  void
  ]b4_parser_class[::yy_symbol_value_print_ (symbol_kind_type yykind,
                           const value_type* yyvaluep]b4_locations_if([[,
                           const location_type* yylocationp]])[) const
  {]b4_locations_if([[
    YY_USE (yylocationp);]])[
    YY_USE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    ]b4_symbol_actions([printer])[
  }


  void
  ]b4_parser_class[::yy_symbol_print_ (symbol_kind_type yykind,
                           const value_type* yyvaluep]b4_locations_if([[,
                           const location_type* yylocationp]])[) const
  {
    *yycdebug_ << (yykind < YYNTOKENS ? "token" : "nterm")
               << ' ' << yysymbol_name (yykind) << " ("]b4_locations_if([[
               << *yylocationp << ": "]])[;
    yy_symbol_value_print_ (yykind, yyvaluep]b4_locations_if([[, yylocationp]])[);
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

#endif
]m4_popdef([b4_parse_param])dnl
b4_namespace_close[]dnl
])


m4_define([b4_define_symbol_kind],
[m4_format([#define %-15s %s],
           b4_symbol($][1, kind_base),
           b4_namespace_ref[::]b4_parser_class[::symbol_kind::]b4_symbol($1, kind_base))
])

#b4_glr_cc_setup
#-- -- -- -- -- -- -- -
#Setup redirections for glr.c : Map the names used in c.m4 to the ones used
#in c++.m4.
m4_define([b4_glr_cc_setup],
[[]b4_attribute_define[
]b4_null_define[

// This skeleton is based on C, yet compiles it as C++.
// So expect warnings about C style casts.
#if defined __clang__ && 306 <= __clang_major__ * 100 + __clang_minor__
#    pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined __GNUC__ && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

// On MacOS, PTRDIFF_MAX is defined as long long, which Clang's
// -pedantic reports as being a C++11 extension.
#if defined __APPLE__ && YY_CPLUSPLUS < 201103L && defined __clang__ && 4 <= __clang_major__
#    pragma clang diagnostic ignored "-Wc++11-long-long"
#endif

#undef] b4_symbol(empty, [id])[
#define] b4_symbol(empty, [id])[] b4_namespace_ref[::] b4_parser_class[::token::] b4_symbol( \
    empty, [id])[
#undef] b4_symbol(eof, [id])[
#define] b4_symbol(eof, [id])[] b4_namespace_ref[::] b4_parser_class[::token::] b4_symbol( \
    eof, [id])[
#undef] b4_symbol(error, [id])[
#define] b4_symbol(error, [id])[] b4_namespace_ref[::] b4_parser_class[::token::] b4_symbol( \
    error, [id])[

#ifndef] b4_api_PREFIX[STYPE
#    define] b4_api_PREFIX[STYPE] b4_namespace_ref[::] b4_parser_class[::value_type
#endif
#ifndef] b4_api_PREFIX[LTYPE
#    define] b4_api_PREFIX[LTYPE] b4_namespace_ref[::] b4_parser_class[::location_type
#endif

typedef ]b4_namespace_ref[::]b4_parser_class[::symbol_kind_type yysymbol_kind_t;

// Expose C++ symbol kinds to C.
]b4_define_symbol_kind(-2)dnl
b4_symbol_foreach([b4_define_symbol_kind])])[
]])


m4_define([b4_undef_symbol_kind],
[[#undef ]b4_symbol($1, kind_base)[
]])

#b4_glr_cc_cleanup
#-- -- -- -- -- -- -- -- -
#Remove redirections for glr.c.
m4_define([b4_glr_cc_cleanup],
[[#undef ]b4_symbol(empty, [id])[
#undef] b4_symbol(eof, [id])[
#undef] b4_symbol(error, [id])[

]b4_undef_symbol_kind(-2)dnl
b4_symbol_foreach([b4_undef_symbol_kind])dnl
])

#b4_shared_declarations(hh | cc)
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Declaration that might either go into the header(if --header, $1 = hh)
# or in the implementation file.
m4_define([b4_shared_declarations],
[m4_pushdef([b4_parse_param], m4_defn([b4_parse_param_orig]))dnl
b4_percent_code_get([[requires]])[
#include <iostream>
#include <stdexcept>
#include <string>

]b4_cxx_portability[
]m4_ifdef([b4_location_include],
          [[# include ]b4_location_include])[
]b4_variant_if([b4_variant_includes])[

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
#    if defined __GNUC__ && !defined __EXCEPTIONS
#        define YY_EXCEPTIONS 0
#    else
#        define YY_EXCEPTIONS 1
#    endif
#endif

]b4_YYDEBUG_define[

]b4_namespace_open[

]b4_bison_locations_if([m4_ifndef([b4_location_file],
                                  [b4_location_define])])[

  /// A Bison parser.
  class ]b4_parser_class[
  {
  public:
]b4_public_types_declare[

    /// Build a parser object.
    ]b4_parser_class[ (]b4_parse_param_decl[);
    virtual ~]b4_parser_class[ ();

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if] b4_api_PREFIX[DEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.]b4_locations_if([[
    /// \param loc    where the syntax error is found.]])[
    /// \param msg    a description of the syntax error.
    virtual void error (]b4_locations_if([[const location_type& loc, ]])[const std::string& msg);

#if] b4_api_PREFIX[DEBUG
  public:
    /// \brief Report a symbol value on the debug stream.
    /// \param yykind       The symbol kind.
    /// \param yyvaluep     Its semantic value.]b4_locations_if([[
    /// \param yylocationp  Its location.]])[
    virtual void yy_symbol_value_print_ (symbol_kind_type yykind,
                                         const value_type* yyvaluep]b4_locations_if([[,
                                         const location_type* yylocationp]])[) const;
    /// \brief Report a symbol on the debug stream.
    /// \param yykind       The symbol kind.
    /// \param yyvaluep     Its semantic value.]b4_locations_if([[
    /// \param yylocationp  Its location.]])[
    virtual void yy_symbol_print_ (symbol_kind_type yykind,
                                   const value_type* yyvaluep]b4_locations_if([[,
                                   const location_type* yylocationp]])[) const;
  private:
    /// Debug stream.
    std::ostream* yycdebug_;
#endif

]b4_parse_param_vars[
  };

]b4_namespace_close[

]b4_percent_code_get([[provides]])[
]m4_popdef([b4_parse_param])dnl
])[

]b4_header_if(
[b4_output_begin([b4_spec_header_file])
b4_copyright([Skeleton interface for Bison GLR parsers in C++],
             [2002-2015, 2018-2021])[
// C++ GLR parser skeleton written by Akim Demaille.

]b4_disclaimer[
]b4_cpp_guard_open([b4_spec_mapped_header_file])[
]b4_shared_declarations[
]b4_cpp_guard_close([b4_spec_mapped_header_file])[
]b4_output_end])

#Let glr.c(and b4_shared_declarations) believe that the user
#arguments include the parser itself.
m4_pushdef([b4_parse_param], m4_defn([b4_parse_param_wrap]))
m4_include(b4_skeletonsdir/[glr.c])
m4_popdef([b4_parse_param])
