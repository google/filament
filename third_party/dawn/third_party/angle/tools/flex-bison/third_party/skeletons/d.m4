                                                            -*- Autoconf -*-

# D language support for Bison

# Copyright (C) 2018-2021 Free Software Foundation, Inc.

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


# b4_symbol_action(SYMBOL-NUM, ACTION)
# ------------------------------------
# Run the action ACTION ("destructor" or "printer") for SYMBOL-NUM.
m4_define([b4_symbol_action],
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


# b4_use(EXPR)
# ------------
# Pacify the compiler about some maybe unused value.
m4_define([b4_use],
[])


# b4_sync_start(LINE, FILE)
# -------------------------
m4_define([b4_sync_start], [[#]line $1 $2])


# b4_list2(LIST1, LIST2)
# ----------------------
# Join two lists with a comma if necessary.
m4_define([b4_list2],
          [$1[]m4_ifval(m4_quote($1), [m4_ifval(m4_quote($2), [[, ]])])[]$2])


# b4_percent_define_get3(DEF, PRE, POST, NOT)
# -------------------------------------------
# Expand to the value of DEF surrounded by PRE and POST if it's %define'ed,
# otherwise NOT.
m4_define([b4_percent_define_get3],
          [m4_ifval(m4_quote(b4_percent_define_get([$1])),
                [$2[]b4_percent_define_get([$1])[]$3], [$4])])

# b4_percent_define_if_get2(ARG1, ARG2, DEF, NOT)
# -----------------------------------------------
# Expand to the value of DEF if ARG1 or ARG2 are %define'ed,
# otherwise NOT.
m4_define([b4_percent_define_if_get2],
          [m4_ifval(m4_quote(b4_percent_define_get([$1])),
                [$3], [m4_ifval(m4_quote(b4_percent_define_get([$2])),
                      [$3], [$4])])])

# b4_percent_define_class_before_interface(CLASS, INTERFACE)
# ----------------------------------------------------------
# Expand to a ', ' if both a class and an interface have been %define'ed
m4_define([b4_percent_define_class_before_interface],
          [m4_ifval(m4_quote(b4_percent_define_get([$1])),
                [m4_ifval(m4_quote(b4_percent_define_get([$2])),
                      [, ])])])


# b4_flag_value(BOOLEAN-FLAG)
# ---------------------------
m4_define([b4_flag_value], [b4_flag_if([$1], [true], [false])])


# b4_parser_class_declaration
# ---------------------------
# The declaration of the parser class ("class YYParser"), with all its
# qualifiers/annotations.
b4_percent_define_default([[api.parser.abstract]], [[false]])
b4_percent_define_default([[api.parser.final]],    [[false]])
b4_percent_define_default([[api.parser.public]],   [[false]])

m4_define([b4_parser_class_declaration],
[b4_percent_define_get3([api.parser.annotations], [], [ ])dnl
b4_percent_define_flag_if([api.parser.public],   [public ])dnl
b4_percent_define_flag_if([api.parser.abstract], [abstract ])dnl
b4_percent_define_flag_if([api.parser.final],    [final ])dnl
[class ]b4_parser_class[]dnl
b4_percent_define_if_get2([api.parser.extends], [api.parser.implements], [ : ])dnl
b4_percent_define_get([api.parser.extends])dnl
b4_percent_define_class_before_interface([api.parser.extends], [api.parser.implements])dnl
b4_percent_define_get([api.parser.implements])])


# b4_lexer_if(TRUE, FALSE)
# ------------------------
m4_define([b4_lexer_if],
[b4_percent_code_ifdef([[lexer]], [$1], [$2])])


# b4_position_type_if(TRUE, FALSE)
# --------------------------------
m4_define([b4_position_type_if],
[b4_percent_define_ifdef([[position_type]], [$1], [$2])])


# b4_location_type_if(TRUE, FALSE)
# --------------------------------
m4_define([b4_location_type_if],
[b4_percent_define_ifdef([[location_type]], [$1], [$2])])


# b4_identification
# -----------------
m4_define([b4_identification],
[[/** Version number for the Bison executable that generated this parser.  */
  public static immutable string yy_bison_version = "]b4_version_string[";

  /** Name of the skeleton that generated this parser.  */
  public static immutable string yy_bison_skeleton = ]b4_skeleton[;
]])


## ------------ ##
## Data types.  ##
## ------------ ##

# b4_int_type(MIN, MAX)
# ---------------------
# Return the smallest int type able to handle numbers ranging from
# MIN to MAX (included).
m4_define([b4_int_type],
[m4_if(b4_ints_in($@,   [-128],   [127]), [1], [byte],
       b4_ints_in($@, [-32768], [32767]), [1], [short],
                                               [int])])

# b4_int_type_for(NAME)
# ---------------------
# Return the smallest int type able to handle numbers ranging from
# `NAME_min' to `NAME_max' (included).
m4_define([b4_int_type_for],
[b4_int_type($1_min, $1_max)])

# b4_null
# -------
m4_define([b4_null], [null])


# b4_integral_parser_table_define(NAME, DATA, COMMENT)
#-----------------------------------------------------
# Define "yy<TABLE-NAME>" whose contents is CONTENT.
m4_define([b4_integral_parser_table_define],
[m4_ifvaln([$3], [b4_comment([$3], [  ])])dnl
private static immutable b4_int_type_for([$2])[[]] yy$1_ =
@{
  $2
@};dnl
])


## ------------- ##
## Token kinds.  ##
## ------------- ##

m4_define([b4_symbol(-2, id)],  [[YYEMPTY]])
b4_percent_define_default([[api.token.raw]], [[true]])

# b4_token_enum(TOKEN-NAME, TOKEN-NUMBER)
# ---------------------------------------
# Output the definition of this token as an enum.
m4_define([b4_token_enum],
[b4_token_format([  %s = %s,
], [$1])])

# b4_token_enums
# --------------
# Output the definition of the tokens as enums.
m4_define([b4_token_enums],
[/* Token kinds.  */
public enum TokenKind {
  ]b4_symbol(empty, id)[ = -2,
b4_symbol_foreach([b4_token_enum])dnl
}
])

# b4_symbol_translate(STRING)
# ---------------------------
# Used by "bison" in the array of symbol names to mark those that
# require translation.
m4_define([b4_symbol_translate],
[[_($1)]])


# _b4_token_constructor_define(SYMBOL-NUM)
# ----------------------------------------
# Define Symbol.FOO for SYMBOL-NUM.
m4_define([_b4_token_constructor_define],
[b4_token_visible_if([$1],
[[
    static auto ]b4_symbol([$1], [id])[(]b4_symbol_if([$1], [has_type],
[b4_union_if([b4_symbol([$1], [type]],
[[typeof(YYSemanticType.]b4_symbol([$1], [type])[]])) [val]])dnl
[]b4_locations_if([b4_symbol_if([$1], [has_type], [[, ]])[Location l]])[)
    {
      return Symbol(TokenKind.]b4_symbol([$1], [id])[]b4_symbol_if([$1], [has_type],
      [[, val]])[]b4_locations_if([[, l]])[);
    }]])])

# b4_token_constructor_define
# ---------------------------
# Define Symbol.FOO for each token kind FOO.
m4_define([b4_token_constructor_define],
[[
    /* Implementation of token constructors for each symbol type visible to
     * the user. The code generates static methods that have the same names
     * as the TokenKinds.
     */]b4_symbol_foreach([_b4_token_constructor_define])dnl
])

## -------------- ##
## Symbol kinds.  ##
## -------------- ##

# b4_symbol_kind(NUM)
# -------------------
m4_define([b4_symbol_kind],
[SymbolKind.b4_symbol_kind_base($@)])


# b4_symbol_enum(SYMBOL-NUM)
# --------------------------
# Output the definition of this symbol as an enum.
m4_define([b4_symbol_enum],
[m4_format([    %-30s %s],
           m4_format([[%s = %s,]],
                     b4_symbol([$1], [kind_base]),
                     [$1]),
           [b4_symbol_tag_comment([$1])])])


# b4_declare_symbol_enum
# ----------------------
# The definition of the symbol internal numbers as an enum.
# Defining YYEMPTY here is important: it forces the compiler
# to use a signed type, which matters for yytoken.
m4_define([b4_declare_symbol_enum],
[[  /* Symbol kinds.  */
  struct SymbolKind
  {
    enum
    {
    ]b4_symbol(empty, kind_base)[ = -2,  /* No symbol.  */
]b4_symbol_foreach([b4_symbol_enum])dnl
[    }

    private int yycode_;
    alias yycode_ this;

    this(int code)
    {
      yycode_ = code;
    }

    /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
    final void toString(W)(W sink) const
    if (isOutputRange!(W, char))
    {
      immutable string[] yy_sname = @{
  ]b4_symbol_names[
      @};]b4_has_translations_if([[
      /* YYTRANSLATABLE[SYMBOL-NUM] -- Whether YY_SNAME[SYMBOL-NUM] is
        internationalizable.  */
      immutable ]b4_int_type_for([b4_translatable])[[] yytranslatable = @{
  ]b4_translatable[
      @};]])[

      put(sink, yy_sname[yycode_]);
    }
  }
]])


# b4_case(ID, CODE, [COMMENTS])
# -----------------------------
m4_define([b4_case], [    case $1:m4_ifval([$3], [ b4_comment([$3])])
$2
      break;])


## ---------------- ##
## Default values.  ##
## ---------------- ##

m4_define([b4_yystype], [b4_percent_define_get([[stype]])])
b4_percent_define_default([[stype]], [[YYSemanticType]])])

# %name-prefix
m4_define_default([b4_prefix], [[YY]])

b4_percent_define_default([[api.parser.class]], [b4_prefix[]Parser])])
m4_define([b4_parser_class], [b4_percent_define_get([[api.parser.class]])])

#b4_percent_define_default([[location_type]], [Location])])
m4_define([b4_location_type], b4_percent_define_ifdef([[location_type]],[b4_percent_define_get([[location_type]])],[YYLocation]))

#b4_percent_define_default([[position_type]], [Position])])
m4_define([b4_position_type], b4_percent_define_ifdef([[position_type]],[b4_percent_define_get([[position_type]])],[YYPosition]))


## ---------------- ##
## api.value.type.  ##
## ---------------- ##


# ---------------------- #
# api.value.type=union.  #
# ---------------------- #

# b4_symbol_type_register(SYMBOL-NUM)
# -----------------------------------
# Symbol SYMBOL-NUM has a type (for union) instead of a type-tag.
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


# _b4_value_type_setup_keyword
# ----------------------------
# api.value.type is defined with a keyword/string syntax.  Check if
# that is properly defined, and prepare its use.
m4_define([_b4_value_type_setup_keyword],
[b4_percent_define_check_values([[[[api.value.type]],
                                  [[none]],
                                  [[union]],
                                  [[union-directive]],
                                  [[yystype]]]])dnl
m4_case(b4_percent_define_get([[api.value.type]]),
        [union],   [b4_value_type_setup_union])])


# b4_value_type_setup
# -------------------
# Check if api.value.type is properly defined, and possibly prepare
# its use.
b4_define_silent([b4_value_type_setup],
[
# Define default value.
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
   [keyword], [_b4_value_type_setup_keyword])
])


## ----------------- ##
## Semantic Values.  ##
## ----------------- ##


# b4_symbol_value(VAL, [SYMBOL-NUM], [TYPE-TAG])
# ----------------------------------------------
# See README.  FIXME: factor in c-like?
m4_define([b4_symbol_value],
[m4_ifval([$3],
          [($1.$3)],
          [m4_ifval([$2],
                    [b4_symbol_if([$2], [has_type],
                                  [($1.b4_symbol([$2], [type]))],
                                  [$1])],
                    [$1])])])

# b4_lhs_value(SYMBOL-NUM, [TYPE])
# --------------------------------
# See README.
m4_define([b4_lhs_value],
[b4_symbol_value([yyval], [$1], [$2])])


# b4_rhs_value(RULE-LENGTH, POS, SYMBOL-NUM, [TYPE])
# --------------------------------------------------
# See README.
#
# In this simple implementation, %token and %type have class names
# between the angle brackets.
m4_define([b4_rhs_value],
[b4_symbol_value([(yystack.valueAt (b4_subtract([$1], [$2])))], [$3], [$4])])


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
[yystack.locationAt (b4_subtract($@))])


# b4_lex_param
# b4_parse_param
# --------------
# If defined, b4_lex_param arrives double quoted, but below we prefer
# it to be single quoted.  Same for b4_parse_param.

# TODO: should be in bison.m4
m4_define_default([b4_lex_param], [[]]))
m4_define([b4_lex_param], b4_lex_param))
m4_define([b4_parse_param], b4_parse_param))

# b4_lex_param_decl
# -------------------
# Extra formal arguments of the constructor.
m4_define([b4_lex_param_decl],
[m4_ifset([b4_lex_param],
          [b4_remove_comma([$1],
                           b4_param_decls(b4_lex_param))],
          [$1])])

m4_define([b4_param_decls],
          [m4_map([b4_param_decl], [$@])])
m4_define([b4_param_decl], [, $1])

m4_define([b4_remove_comma], [m4_ifval(m4_quote($1), [$1, ], [])m4_shift2($@)])



# b4_parse_param_decl
# -------------------
# Extra formal arguments of the constructor.
m4_define([b4_parse_param_decl],
[m4_ifset([b4_parse_param],
          [b4_remove_comma([$1],
                           b4_param_decls(b4_parse_param))],
          [$1])])



# b4_lex_param_call
# -------------------
# Delegating the lexer parameters to the lexer constructor.
m4_define([b4_lex_param_call],
          [m4_ifset([b4_lex_param],
                    [b4_remove_comma([$1],
                                     b4_param_calls(b4_lex_param))],
                    [$1])])
m4_define([b4_param_calls],
          [m4_map([b4_param_call], [$@])])
m4_define([b4_param_call], [, $2])



# b4_parse_param_cons
# -------------------
# Extra initialisations of the constructor.
m4_define([b4_parse_param_cons],
          [m4_ifset([b4_parse_param],
                    [b4_constructor_calls(b4_parse_param)])])

m4_define([b4_constructor_calls],
          [m4_map([b4_constructor_call], [$@])])
m4_define([b4_constructor_call],
          [this.$2 = $2;
          ])



# b4_parse_param_vars
# -------------------
# Extra instance variables.
m4_define([b4_parse_param_vars],
          [m4_ifset([b4_parse_param],
                    [
    /* User arguments.  */
b4_var_decls(b4_parse_param)])])

m4_define([b4_var_decls],
          [m4_map_sep([b4_var_decl], [
], [$@])])
m4_define([b4_var_decl],
          [    protected $1;])


# b4_public_types_declare
# -----------------------
# Define the public types: token, semantic value, location, and so forth.
# Depending on %define token_lex, may be output in the header or source file.
m4_define([b4_public_types_declare],
[[
alias Symbol = ]b4_parser_class[.Symbol;
alias Value = ]b4_yystype[;]b4_locations_if([[
alias Location = ]b4_location_type[;
alias Position = ]b4_position_type[;]b4_push_if([[
alias PUSH_MORE = ]b4_parser_class[.YYPUSH_MORE;
alias ABORT = ]b4_parser_class[.YYABORT;
alias ACCEPT = ]b4_parser_class[.YYACCEPT;]])[]])[
]])


# b4_basic_symbol_constructor_define
# ----------------------------------
# Create Symbol struct constructors for all the visible types.
m4_define([b4_basic_symbol_constructor_define],
[b4_token_visible_if([$1],
[[    this(TokenKind token]b4_symbol_if([$1], [has_type],
[[, ]b4_union_if([], [[typeof(YYSemanticType.]])b4_symbol([$1], [type])dnl
[]b4_union_if([], [[) ]])[ val]])[]b4_locations_if([[, Location loc]])[)
    {
      kind = yytranslate_(token);]b4_union_if([b4_symbol_if([$1], [has_type], [[
      static foreach (member; __traits(allMembers, YYSemanticType))
      {
        static if (is(typeof(mixin("value_." ~ member)) == ]b4_symbol([$1], [type])[))
        {
          mixin("value_." ~ member ~ " = val;");
        }
      }]])], [b4_symbol_if([$1], [has_type], [[
      value_.]b4_symbol([$1], [type])[ = val;]])])[]b4_locations_if([
      location_ = loc;])[
    }
]])])


# b4_symbol_type_define
# ---------------------
# Define symbol_type, the external type for symbols used for symbol
# constructors.
m4_define([b4_symbol_type_define],
[[
  /**
    * A complete symbol
    */
  struct Symbol
  {
    private SymbolKind kind;
    private Value value_;]b4_locations_if([[
    private Location location_;]])[

]b4_type_foreach([b4_basic_symbol_constructor_define])[
    SymbolKind token() { return kind; }
    Value value() { return value_; }]b4_locations_if([[
    Location location() { return location_; }]])[
]b4_token_ctor_if([b4_token_constructor_define])[
  }
]])
