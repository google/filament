# D skeleton for Bison -*- autoconf -*-

# Copyright (C) 2007-2012, 2019-2021 Free Software Foundation, Inc.

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

m4_include(b4_skeletonsdir/[d.m4])

b4_header_if([b4_complain([%header/%defines does not make sense in D])])

# parse.lac
b4_percent_define_default([[parse.lac]], [[none]])
b4_percent_define_check_values([[[[parse.lac]], [[full]], [[none]]]])
b4_define_flag_if([lac])
m4_define([b4_lac_flag],
          [m4_if(b4_percent_define_get([[parse.lac]]),
                 [none], [[0]], [[1]])])


## --------------- ##
## api.push-pull.  ##
## --------------- ##

b4_percent_define_default([[api.push-pull]], [[pull]])
b4_percent_define_check_values([[[[api.push-pull]],
                                 [[pull]], [[push]], [[both]]]])

# Define m4 conditional macros that encode the value
# of the api.push-pull flag.
b4_define_flag_if([pull]) m4_define([b4_pull_flag], [[1]])
b4_define_flag_if([push]) m4_define([b4_push_flag], [[1]])
m4_case(b4_percent_define_get([[api.push-pull]]),
        [pull], [m4_define([b4_push_flag], [[0]])],
        [push], [m4_define([b4_pull_flag], [[0]])])

# Define a macro to be true when api.push-pull has the value "both".
m4_define([b4_both_if],[b4_push_if([b4_pull_if([$1],[$2])],[$2])])

# Handle BISON_USE_PUSH_FOR_PULL for the test suite.  So that push parsing
# tests function as written, do not let BISON_USE_PUSH_FOR_PULL modify the
# behavior of Bison at all when push parsing is already requested.
b4_define_flag_if([use_push_for_pull])
b4_use_push_for_pull_if([
  b4_push_if([m4_define([b4_use_push_for_pull_flag], [[0]])],
             [m4_define([b4_push_flag], [[1]])])])


# Define a macro to encapsulate the parse state variables.  This
# allows them to be defined either in parse() when doing pull parsing,
# or as class instance variable when doing push parsing.
b4_output_begin([b4_parser_file_name])
b4_copyright([Skeleton implementation for Bison LALR(1) parsers in D],
             [2007-2012, 2019-2021])[
]b4_disclaimer[
]b4_percent_define_ifdef([package], [module b4_percent_define_get([package]);
])[
version(D_Version2) {
} else {
  static assert(false, "need compiler for D Version 2");
}

]b4_user_pre_prologue[
]b4_user_post_prologue[
]b4_percent_code_get([[imports]])[
import std.format;
import std.conv;

/**
 * Handle error message internationalisation.
 */
static if (!is(typeof(YY_))) {
  version(YYENABLE_NLS)
  {
    version(ENABLE_NLS)
    {
      extern(C) char* dgettext(const char*, const char*);
      string YY_(const char* s)
      {
        return to!string(dgettext("bison-runtime", s));
      }
    }
  }
  static if (!is(typeof(YY_)))
  {
    pragma(inline, true)
    string YY_(string msg) { return msg; }
  }
}

/**
 * A Bison parser, automatically generated from <tt>]m4_bpatsubst(b4_file_name, [^"\(.*\)"$], [\1])[</tt>.
 *
 * @@author LALR (1) parser skeleton written by Paolo Bonzini.
 * Port to D language was done by Oliver Mangold.
 */

/**
 * Communication interface between the scanner and the Bison-generated
 * parser <tt>]b4_parser_class[</tt>.
 */
public interface Lexer
{
  /**
   * Entry point for the scanner.  Returns the token identifier corresponding
   * to the next token and prepares to return the semantic value
   * ]b4_locations_if([and beginning/ending positions ])[of the token.
   * @@return the token identifier corresponding to the next token. */
  Symbol yylex ();

  /**
   * Entry point for error reporting.  Emits an error
   * ]b4_locations_if([referring to the given location ])[in a user-defined way.
   *]b4_locations_if([[
   * @@param loc The location of the element to which the
   *                error message is related]])[
   * @@param s The string for the error message.  */
   void yyerror (]b4_locations_if([[const Location loc, ]])[string s);
]b4_parse_error_bmatch([custom], [[
  /**
   * Build and emit a "syntax error" message in a user-defined way.
   *
   * @@param ctx  The context of the error.
   */
  void reportSyntaxError(]b4_parser_class[.Context ctx);
]])[
}

]b4_public_types_declare[

]b4_locations_if([b4_position_type_if([[
static assert(__traits(compiles,
              (new Position[1])[0]=(new Position[1])[0]),
              "struct/class Position must be default-constructible "
              "and assignable");
static assert(__traits(compiles, (new string[1])[0]=(new Position).toString()),
              "error: struct/class Position must have toString method");
]], [[
  /**
   * A struct denoting a point in the input.*/
public struct ]b4_position_type[ {

  /** The column index within the line of input.  */
  public int column = 1;
  /** The line number within an input file.  */
  public int line = 1;
  /** The name of the input file.  */
  public string filename = null;

  /**
   * A string representation of the position. */
  public string toString() const {
    if (filename)
      return format("%s:%d.%d", filename, line, column);
    else
      return format("%d.%d", line, column);
  }
}
]])b4_location_type_if([[
static assert(__traits(compiles, (new Location((new Position[1])[0]))) &&
              __traits(compiles, (new Location((new Position[1])[0], (new Position[1])[0]))),
              "error: struct/class Location must have "
              "default constructor and constructors this(Position) and this(Position, Position).");
static assert(__traits(compiles, (new Location[1])[0].begin=(new Location[1])[0].begin) &&
              __traits(compiles, (new Location[1])[0].begin=(new Location[1])[0].end) &&
              __traits(compiles, (new Location[1])[0].end=(new Location[1])[0].begin) &&
              __traits(compiles, (new Location[1])[0].end=(new Location[1])[0].end),
              "error: struct/class Location must have assignment-compatible "
              "members/properties 'begin' and 'end'.");
static assert(__traits(compiles, (new string[1])[0]=(new Location[1])[0].toString()),
              "error: struct/class Location must have toString method.");

private immutable bool yy_location_is_class = !__traits(compiles, *(new Location((new Position[1])[0])));]], [[
/**
 * A struct defining a pair of positions.  Positions, defined by the
 * <code>Position</code> struct, denote a point in the input.
 * Locations represent a part of the input through the beginning
 * and ending positions.  */
public struct ]b4_location_type[
{
  /** The first, inclusive, position in the range.  */
  public Position begin;

  /** The first position beyond the range.  */
  public Position end;

  /**
   * Create a <code>Location</code> denoting an empty range located at
   * a given point.
   * @@param loc The position at which the range is anchored.  */
  public this(Position loc)
  {
    this.begin = this.end = loc;
  }

  /**
   * Create a <code>Location</code> from the endpoints of the range.
   * @@param begin The first position included in the range.
   * @@param end   The first position beyond the range.  */
  public this(Position begin, Position end)
  {
    this.begin = begin;
    this.end = end;
  }

  /**
   * Reset initial location to final location.
   */
  public void step()
  {
    this.begin = this.end;
  }

  /**
   * A representation of the location.
   */
  public string toString() const
  {
    auto end_col = 0 < end.column ? end.column - 1 : 0;
    auto res = begin.toString ();
    if (end.filename && begin.filename != end.filename)
      res ~= "-" ~ format("%s:%d.%d", end.filename, end.line, end_col);
    else if (begin.line < end.line)
      res ~= "-" ~ format("%d.%d", end.line, end_col);
    else if (begin.column < end_col)
      res ~= "-" ~ format("%d", end_col);
    return res;
  }
}

private immutable bool yy_location_is_class = false;

]])])[]b4_value_type_setup[]m4_ifdef([b4_user_union_members], [private union YYSemanticType
{
b4_user_union_members
};],
[m4_if(b4_tag_seen_flag, 0,
[[private alias int YYSemanticType;]])])[
]b4_token_enums[
]b4_parser_class_declaration[
{
  ]b4_identification[

]b4_declare_symbol_enum[

]b4_locations_if([[
  private final Location yylloc_from_stack (ref YYStack rhs, int n)
  {
    static if (yy_location_is_class) {
      if (n > 0)
        return new Location (rhs.locationAt (n-1).begin, rhs.locationAt (0).end);
      else
        return new Location (rhs.locationAt (0).end);
    } else {
      if (n > 0)
        return Location (rhs.locationAt (n-1).begin, rhs.locationAt (0).end);
      else
        return Location (rhs.locationAt (0).end);
    }
  }]])[

]b4_lexer_if([[  private class YYLexer implements Lexer {
]b4_percent_code_get([[lexer]])[
  }
]])[
  /** The object doing lexical analysis for us.  */
  private Lexer yylexer;

]b4_parse_param_vars[

]b4_lexer_if([[
  /**
   * Instantiate the Bison-generated parser.
   */
  public this] (b4_parse_param_decl([b4_lex_param_decl])[) {
]b4_percent_code_get([[init]])[]b4_lac_if([[
    this.yylacStack = new int[];
    this.yylacEstablished = false;]])[
    this (new YYLexer(]b4_lex_param_call[));
  }
]])[

  /**
   * Instantiate the Bison-generated parser.
   * @@param yylexer The scanner that will supply tokens to the parser.
   */
  ]b4_lexer_if([[protected]], [[public]]) [this (]b4_parse_param_decl([[Lexer yylexer]])[) {
    this.yylexer = yylexer;]b4_parse_trace_if([[
    this.yyDebugStream = stderr;]])[
]b4_parse_param_cons[
  }
]b4_parse_trace_if([[
  import std.stdio;
  private File yyDebugStream;

  /**
   * The <tt>File</tt> on which the debugging output is
   * printed.
   */
  public File getDebugStream () { return yyDebugStream; }

  /**
   * Set the <tt>std.File</tt> on which the debug output is printed.
   * @@param s The stream that is used for debugging output.
   */
  public final void setDebugStream(File s) { yyDebugStream = s; }

  private int yydebug = 0;

  /**
   * Answer the verbosity of the debugging output; 0 means that all kinds of
   * output from the parser are suppressed.
   */
  public final int getDebugLevel() { return yydebug; }

  /**
   * Set the verbosity of the debugging output; 0 means that all kinds of
   * output from the parser are suppressed.
   * @@param level The verbosity level for debugging output.
   */
  public final void setDebugLevel(int level) { yydebug = level; }

  protected final void yycdebug (string s) {
    if (0 < yydebug)
      yyDebugStream.write (s);
  }

  protected final void yycdebugln (string s) {
    if (0 < yydebug)
      yyDebugStream.writeln (s);
  }
]])[
  private final ]b4_parser_class[.Symbol yylex () {
    return yylexer.yylex ();
  }

  protected final void yyerror (]b4_locations_if([[const Location loc, ]])[string s) {
    yylexer.yyerror (]b4_locations_if([loc, ])[s);
  }

  /**
   * The number of syntax errors so far.
   */
  public int numberOfErrors() const { return yynerrs_; }
  private int yynerrs_ = 0;

  /**
   * Returned by a Bison action in order to stop the parsing process and
   * return success (<tt>true</tt>).  */
  public static immutable int YYACCEPT = 0;

  /**
   * Returned by a Bison action in order to stop the parsing process and
   * return failure (<tt>false</tt>).  */
  public static immutable int YYABORT = 1;
]b4_push_if([
  /**
   * Returned by a Bison action in order to request a new token.
   */
  public static immutable int YYPUSH_MORE = 4;])[

  /**
   * Returned by a Bison action in order to start error recovery without
   * printing an error message.  */
  public static immutable int YYERROR = 2;

  // Internal return codes that are not supported for user semantic
  // actions.
  private static immutable int YYERRLAB = 3;
  private static immutable int YYNEWSTATE = 4;
  private static immutable int YYDEFAULT = 5;
  private static immutable int YYREDUCE = 6;
  private static immutable int YYERRLAB1 = 7;
  private static immutable int YYRETURN = 8;
]b4_push_if([[  private static immutable int YYGETTOKEN = 9; /* Signify that a new token is expected when doing push-parsing.  */]])[

]b4_locations_if([
  private static immutable YYSemanticType yy_semantic_null;])[
  private int yyerrstatus_ = 0;

  private void yyerrok()
  {
    yyerrstatus_ = 0;
  }

  // Lookahead symbol kind.
  SymbolKind yytoken = ]b4_symbol(empty, kind)[;

  /* State.  */
  int yyn = 0;
  int yylen = 0;
  int yystate = 0;

  YYStack yystack;

  int label = YYNEWSTATE;

  /* Error handling.  */
]b4_locations_if([[
  /// The location where the error started.
  Location yyerrloc;

  /// Location of the lookahead.
  Location yylloc;

  /// @@$.
  Location yyloc;]])[

  /// Semantic value of the lookahead.
  Value yylval;

  /**
   * Whether error recovery is being done.  In this state, the parser
   * reads token until it reaches a known state, and then restarts normal
   * operation.  */
  public final bool recovering ()
  {
    return yyerrstatus_ == 0;
  }

  /** Compute post-reduction state.
   * @@param yystate   the current state
   * @@param yysym     the nonterminal to push on the stack
   */
  private int yyLRGotoState(int yystate, int yysym) {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  private int yyaction (int yyn, ref YYStack yystack, int yylen)
  {
    Value yyval;]b4_locations_if([[
    Location yyloc = yylloc_from_stack (yystack, yylen);]])[

    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen > 0)
      yyval = yystack.valueAt (yylen - 1);
    else
      yyval = yystack.valueAt (0);

]b4_parse_trace_if([[
    yy_reduce_print (yyn, yystack);]])[

    switch (yyn)
    {
]b4_user_actions[
      default: break;
    }

]b4_parse_trace_if([[
    yy_symbol_print ("-> $$ =", to!SymbolKind (yyr1_[yyn]), yyval]b4_locations_if([, yyloc])[);]])[

    yystack.pop (yylen);
    yylen = 0;

    /* Shift the result of the reduction.  */
    int yystate = yyLRGotoState(yystack.stateAt(0), yyr1_[yyn]);
    yystack.push (yystate, yyval]b4_locations_if([, yyloc])[);
    return YYNEWSTATE;
  }

]b4_parse_trace_if([[
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  private final void yy_symbol_print (string s, SymbolKind yykind,
    ref Value yyval]b4_locations_if([, ref Location yyloc])[)
  {
    if (0 < yydebug)
    {
      File yyo = yyDebugStream;
      yyo.write(s);
      yyo.write(yykind < yyntokens_ ? " token " : " nterm ");
      yyo.write(format("%s", yykind));
      yyo.write(" ("]b4_locations_if([ ~ yyloc.toString() ~ ": "])[);
      ]b4_symbol_actions([printer])[
      yyo.write(")\n");
    }
  }
]])[
]b4_symbol_type_define[
]b4_push_if([[
  /**
   * Push Parse input from external lexer
   *
   * @@param yyla current Symbol
   *
   * @@return <tt>YYACCEPT, YYABORT, YYPUSH_MORE</tt>
   */
  public int pushParse(Symbol yyla)]], [[
  /**
   * Parse input from the scanner that was specified at object construction
   * time.  Return whether the end of the input was reached successfully.
   *
   * @@return <tt>true</tt> if the parsing succeeds.  Note that this does not
   *          imply that there were no syntax errors.
   */
  public bool parse()]])[
  {]b4_push_if([[
    if (!this.pushParseInitialized)
    {
      pushParseInitialize();
      yyerrstatus_ = 0;
    }
    else
      label = YYGETTOKEN;

    bool push_token_consumed = true;
]], [[  bool yyresult;]b4_lac_if([[
    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yylacDiscard("init");]])[]b4_parse_trace_if([[

    yycdebugln ("Starting parse");]])[
    yyerrstatus_ = 0;

]m4_ifdef([b4_initial_action], [
m4_pushdef([b4_at_dollar],     [yylloc])dnl
m4_pushdef([b4_dollar_dollar], [yylval])dnl
    /* User initialization code.  */
    b4_user_initial_action
m4_popdef([b4_dollar_dollar])dnl
m4_popdef([b4_at_dollar])])dnl

  [  /* Initialize the stack.  */
    yystack.push (yystate, yylval]b4_locations_if([, yylloc])[);

    label = YYNEWSTATE;]])[
    for (;;)
      final switch (label)
      {
        /* New state.  Unlike in the C/C++ skeletons, the state is already
           pushed when we come here.  */
      case YYNEWSTATE:]b4_parse_trace_if([[
        yycdebugln (format("Entering state %d", yystate));
        if (0 < yydebug)
          yystack.print (yyDebugStream);]])[

        /* Accept?  */
        if (yystate == yyfinal_)]b4_push_if([[
        {
          label = YYACCEPT;
          break;
        }]], [[
          return true;]])[

        /* Take a decision.  First try without lookahead.  */
        yyn = yypact_[yystate];
        if (yyPactValueIsDefault(yyn))
        {
          label = YYDEFAULT;
          break;
        }]b4_push_if([[
        goto case;

        case YYGETTOKEN:]])[

        /* Read a lookahead token.  */
        if (yytoken == ]b4_symbol(empty, kind)[)
        {]b4_push_if([[
          if (!push_token_consumed)
            return YYPUSH_MORE;]])[]b4_parse_trace_if([[
          yycdebugln ("Reading a token");]])[]b4_push_if([[
          yytoken = yyla.token;
          yylval = yyla.value;]b4_locations_if([[
          yylloc = yyla.location;]])[
          push_token_consumed = false;]], [[
          Symbol yysymbol = yylex();
          yytoken = yysymbol.token();
          yylval = yysymbol.value();]b4_locations_if([[
          yylloc = yysymbol.location();]])[]])[
        }

        /* Token already converted to internal form.  */]b4_parse_trace_if([[
        yy_symbol_print ("Next token is", yytoken, yylval]b4_locations_if([, yylloc])[);]])[

        if (yytoken == ]b4_symbol(error, kind)[)
        {
          // The scanner already issued an error message, process directly
          // to error recovery.  But do not keep the error token as
          // lookahead, it is too special and may lead us to an endless
          // loop in error recovery. */
          yytoken = ]b4_symbol(undef, kind)[;]b4_locations_if([[
          yyerrloc = yylloc;]])[
          label = YYERRLAB1;
        }
        else
        {
          /* If the proper action on seeing token YYTOKEN is to reduce or to
             detect an error, take that action.  */
          yyn += yytoken;
          if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken) {]b4_lac_if([[
            if (!yylacEstablish(yystack, yytoken))
              label = YYERRLAB;
            else]])[
              label = YYDEFAULT;
          }
          /* <= 0 means reduce or error.  */
          else if ((yyn = yytable_[yyn]) <= 0)
          {
            if (yyTableValueIsError(yyn))
              label = YYERRLAB;]b4_lac_if([[
            else if (!yylacEstablish(yystack, yytoken))
              label = YYERRLAB;]])[
            else
            {
              yyn = -yyn;
              label = YYREDUCE;
            }
          }
          else
          {
            /* Shift the lookahead token.  */]b4_parse_trace_if([[
            yy_symbol_print ("Shifting", yytoken, yylval]b4_locations_if([, yylloc])[);]])[

            /* Discard the token being shifted.  */
            yytoken = ]b4_symbol(empty, kind)[;

            /* Count tokens shifted since error; after three, turn off error
             * status.  */
            if (yyerrstatus_ > 0)
              --yyerrstatus_;

            yystate = yyn;
            yystack.push (yystate, yylval]b4_locations_if([, yylloc])[);]b4_lac_if([[
            yylacDiscard("shift");]])[
            label = YYNEWSTATE;
          }
        }
        break;

      /*-----------------------------------------------------------.
      | yydefault -- do the default action for the current state.  |
      `-----------------------------------------------------------*/
      case YYDEFAULT:
        yyn = yydefact_[yystate];
        if (yyn == 0)
          label = YYERRLAB;
        else
          label = YYREDUCE;
        break;

      /*-----------------------------.
      | yyreduce -- Do a reduction.  |
      `-----------------------------*/
      case YYREDUCE:
        yylen = yyr2_[yyn];
        label = yyaction (yyn, yystack, yylen);
        yystate = yystack.stateAt (0);
        break;

      /*--------------------------------------.
      | yyerrlab -- here on detecting error.  |
      `--------------------------------------*/
      case YYERRLAB:
        /* If not already recovering from an error, report this error.  */
        if (yyerrstatus_ == 0)
        {
          ++yynerrs_;
          yyreportSyntaxError(new Context(]b4_lac_if([[this, ]])[yystack, yytoken]b4_locations_if([[, yylloc]])[));
        }
]b4_locations_if([
        yyerrloc = yylloc;])[
        if (yyerrstatus_ == 3)
        {
          /* If just tried and failed to reuse lookahead token after an
           * error, discard it.  */

          /* Return failure if at end of input.  */
          if (yytoken == ]b4_symbol(eof, [kind])[)]b4_push_if([[
          {
            label = YYABORT;
            break;
          }]], [[
          return false;]])[
          else
            yytoken = ]b4_symbol(empty, kind)[;
        }

        /* Else will try to reuse lookahead token after shifting the error
         * token.  */
        label = YYERRLAB1;
        break;

      /*-------------------------------------------------.
      | errorlab -- error raised explicitly by YYERROR.  |
      `-------------------------------------------------*/
      case YYERROR:]b4_locations_if([
        yyerrloc = yystack.locationAt (yylen - 1);])[
        /* Do not reclaim the symbols of the rule which action triggered
           this YYERROR.  */
        yystack.pop (yylen);
        yylen = 0;
        yystate = yystack.stateAt (0);
        label = YYERRLAB1;
        break;

      /*-------------------------------------------------------------.
      | yyerrlab1 -- common code for both syntax error and YYERROR.  |
      `-------------------------------------------------------------*/
      case YYERRLAB1:
        yyerrstatus_ = 3;       /* Each real token shifted decrements this.  */

        // Pop stack until we find a state that shifts the error token.
        for (;;)
        {
          yyn = yypact_[yystate];
          if (!yyPactValueIsDefault(yyn))
          {
            yyn += ]b4_symbol(error, kind)[;
            if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == ]b4_symbol(error, kind)[)
            {
              yyn = yytable_[yyn];
              if (0 < yyn)
                break;
                  }
          }

          /* Pop the current state because it cannot handle the error token.  */
          if (yystack.height == 1)]b4_push_if([[
          {
            label = YYABORT;
            break;
          }]],[[
            return false;]])[

]b4_locations_if([          yyerrloc = yystack.locationAt (0);])[
          yystack.pop ();
          yystate = yystack.stateAt (0);]b4_parse_trace_if([[
          if (0 < yydebug)
            yystack.print (yyDebugStream);]])[
        }]b4_push_if([[
        if (label == YYABORT)
          /* Leave the switch.  */
          break;
]])[
]b4_locations_if([
        /* Muck with the stack to setup for yylloc.  */
        yystack.push (0, yy_semantic_null, yylloc);
        yystack.push (0, yy_semantic_null, yyerrloc);
        yyloc = yylloc_from_stack (yystack, 2);
        yystack.pop (2);])[

        /* Shift the error token.  */]b4_lac_if([[
        yylacDiscard("error recovery");]])[]b4_parse_trace_if([[
        yy_symbol_print ("Shifting", to!SymbolKind (yystos_[yyn]), yylval]b4_locations_if([, yyloc])[);]])[
        yystate = yyn;
        yystack.push (yyn, yylval]b4_locations_if([, yyloc])[);
        label = YYNEWSTATE;
        break;

      /* Accept.  */
      case YYACCEPT:]b4_push_if([[
        this.pushParseInitialized = false;]b4_parse_trace_if([[
        if (0 < yydebug)
          yystack.print (yyDebugStream);]])[
        return YYACCEPT;]], [[
        yyresult = true;
        label = YYRETURN;
        break;]])[

      /* Abort.  */
      case YYABORT:]b4_push_if([[
        this.pushParseInitialized = false;]b4_parse_trace_if([[
        if (0 < yydebug)
          yystack.print (yyDebugStream);]])[
        return YYABORT;]], [[
        yyresult = false;
        label = YYRETURN;
        break;]])[
]b4_push_if([[]], [[      ][case YYRETURN:]b4_parse_trace_if([[
        if (0 < yydebug)
          yystack.print (yyDebugStream);]])[
        return yyresult;]])[
    }
    assert(0);
  }

]b4_push_if([[
  bool pushParseInitialized = false;

  /**
   * (Re-)Initialize the state of the push parser.
   */
  public void pushParseInitialize()
  {

    /* Lookahead and lookahead in internal form.  */
    this.yytoken = ]b4_symbol(empty, kind)[;

    /* State.  */
    this.yyn = 0;
    this.yylen = 0;
    this.yystate = 0;
    destroy(this.yystack);
    this.label = YYNEWSTATE;
]b4_lac_if([[
    destroy(this.yylacStack);
    this.yylacEstablished = false;]])[

    /* Error handling.  */
    this.yynerrs_ = 0;
]b4_locations_if([
    /* The location where the error started.  */
    this.yyerrloc = Location(Position(), Position());
    this.yylloc = Location(Position(), Position());])[

    /* Semantic value of the lookahead.  */
    //destroy(this.yylval);

    /* Initialize the stack.  */
    yystack.push(this.yystate, this.yylval]b4_locations_if([, this.yylloc])[);

    this.pushParseInitialized = true;
  }]])[]b4_both_if([[
  /**
   * Parse input from the scanner that was specified at object construction
   * time.  Return whether the end of the input was reached successfully.
   * This version of parse() is defined only when api.push-push=both.
   *
   * @@return <tt>true</tt> if the parsing succeeds.  Note that this does not
   *          imply that there were no syntax errors.
   */
  bool parse()
  {
    int status = 0;
    do {
      status = this.pushParse(yylex());
    } while (status == YYPUSH_MORE);
    return status == YYACCEPT;
  }]])[

  // Generate an error message.
  private final void yyreportSyntaxError(Context yyctx)
  {]b4_parse_error_bmatch(
[custom], [[
    yylexer.reportSyntaxError(yyctx);]],
[detailed], [[
    if (yyctx.getToken() != ]b4_symbol(empty, kind)[)
    {
      // FIXME: This method of building the message is not compatible
      // with internationalization.
      immutable int argmax = 5;
      SymbolKind[] yyarg = new SymbolKind[argmax];
      int yycount = yysyntaxErrorArguments(yyctx, yyarg, argmax);
      string res, yyformat;
      switch (yycount)
      {
        case  1:
          yyformat = YY_("syntax error, unexpected %s");
          res = format(yyformat, yyarg[0]);
         break;
        case  2:
          yyformat = YY_("syntax error, unexpected %s, expecting %s");
          res = format(yyformat, yyarg[0], yyarg[1]);
          break;
        case  3:
          yyformat = YY_("syntax error, unexpected %s, expecting %s or %s");
          res = format(yyformat, yyarg[0], yyarg[1], yyarg[2]);
          break;
        case  4:
          yyformat = YY_("syntax error, unexpected %s, expecting %s or %s or %s");
          res = format(yyformat, yyarg[0], yyarg[1], yyarg[2], yyarg[3]);
          break;
        case  5:
          yyformat = YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
          res = format(yyformat, yyarg[0], yyarg[1], yyarg[2], yyarg[3], yyarg[4]);
          break;
        default:
          res = YY_("syntax error");
          break;
      }
      yyerror(]b4_locations_if([yyctx.getLocation(), ])[res);
    }]],
[[simple]], [[
    yyerror(]b4_locations_if([yyctx.getLocation(), ])[YY_("syntax error"));]])[
  }

]b4_parse_error_bmatch(
[detailed], [[
  private int yysyntaxErrorArguments(Context yyctx, SymbolKind[] yyarg, int yyargn) {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action,
         then the only way this function was invoked is if the
         default action is an error action.  In that case, don't
         check for expected tokens because there are none.
       - The only way there can be no lookahead present (in tok) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this
         state is a consistent state with a default action.  There
         might have been a previous inconsistent state, consistent
         state with a non-default action, or user semantic action
         that manipulated yychar.  (However, yychar is currently out
         of scope during semantic actions.)
       - Of course, the expected token list depends on states to
         have correct lookahead information, and it depends on the
         parser not to perform extra reductions after fetching a
         lookahead from the scanner and before detecting a syntax
         error.  Thus, state merging (from LALR or IELR) and default
         reductions corrupt the expected token list.  However, the
         list is correct for canonical LR with one exception: it
         will still contain any token that will not be accepted due
         to an error action in a later state.
    */
    int yycount = 0;
    if (yyctx.getToken() != ]b4_symbol(empty, kind)[)
      {
        if (yyarg !is null)
          yyarg[yycount] = yyctx.getToken();
        yycount += 1;
        yycount += yyctx.getExpectedTokens(yyarg, 1, yyargn);
      }
    return yycount;
  }
]])[


  /**
   * Information needed to get the list of expected tokens and to forge
   * a syntax error diagnostic.
   */
  public static final class Context
  {]b4_lac_if([[
    private ]b4_parser_class[ yyparser;]])[
    private const(YYStack) yystack;
    private SymbolKind yytoken;]b4_locations_if([[
    private const(Location) yylocation;]])[

    this(]b4_lac_if([[]b4_parser_class[ parser, ]])[YYStack stack, SymbolKind kind]b4_locations_if([[, Location loc]])[)
    {]b4_lac_if([[
        yyparser = parser;]])[
      yystack = stack;
      yytoken = kind;]b4_locations_if([[
      yylocation = loc;]])[
    }

    final SymbolKind getToken() const
    {
      return yytoken;
    }]b4_locations_if([[

    final const(Location) getLocation() const
    {
      return yylocation;
    }]])[
    /**
     * Put in YYARG at most YYARGN of the expected tokens given the
     * current YYCTX, and return the number of tokens stored in YYARG.  If
     * YYARG is null, return the number of expected tokens (guaranteed to
     * be less than YYNTOKENS).
     */
    int getExpectedTokens(SymbolKind[] yyarg, int yyargn)]b4_lac_if([[]], [[ const]])[
    {
      return getExpectedTokens(yyarg, 0, yyargn);
    }

    int getExpectedTokens(SymbolKind[] yyarg, int yyoffset, int yyargn)]b4_lac_if([[]], [[ const]])[
    {
      int yycount = yyoffset;]b4_lac_if([b4_parse_trace_if([[
      // Execute LAC once. We don't care if it is successful, we
      // only do it for the sake of debugging output.

      if (!yyparser.yylacEstablished)
        yyparser.yylacCheck(yystack, yytoken);
]])[
      for (int yyx = 0; yyx < yyntokens_; ++yyx)
        {
          SymbolKind yysym = SymbolKind(yyx);
          if (yysym != ]b4_symbol(error, kind)[
              && yysym != ]b4_symbol(undef, kind)[
              && yyparser.yylacCheck(yystack, yysym))
            {
              if (yyarg == null)
                yycount += 1;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = yysym;
            }
        }]], [[
      int yyn = yypact_[this.yystack.stateAt(0)];
      if (!yyPactValueIsDefault(yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative
           indexes in YYCHECK.  In other words, skip the first
           -YYN actions for this state because they are default
           actions.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;
        /* Stay within bounds of both yycheck and yytname.  */
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != ]b4_symbol(error, kind)[
              && !yyTableValueIsError(yytable_[yyx + yyn]))
          {
            if (yyarg is null)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = SymbolKind(yyx);
          }
      }]])[
      if (yyarg !is null && yycount == yyoffset && yyoffset < yyargn)
        yyarg[yyoffset] = ]b4_symbol(empty, kind)[;
      return yycount - yyoffset;
    }
  }

]b4_lac_if([[
  /** Check the lookahead yytoken.
   * \returns  true iff the token will be eventually shifted.
   */
  bool yylacCheck(const YYStack yystack, SymbolKind yytoken)
  {
    // Logically, the yylacStack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    destroy(yylacStack);
    // Reduce until we encounter a shift and thereby accept the token.
]b4_parse_trace_if([[
    yycdebug("LAC: checking lookahead " ~ format("%s", yytoken) ~ ":");]])[
    int lacTop = 0;
    while (true)
    {
      int topState = (yylacStack.length == 0
                      ? yystack.stateAt(lacTop)
                      : yylacStack[$ - 1]);
      int yyrule = yypact_[topState];
      if (yyPactValueIsDefault(yyrule)
          || (yyrule += yytoken) < 0 || yylast_ < yyrule
          || yycheck_[yyrule] != yytoken)
      {
        // Use the default action.
        yyrule = yydefact_[+topState];
        if (yyrule == 0)
        {]b4_parse_trace_if([[
          yycdebugln(" Err");]])[
          return false;
        }
      }
      else
      {
        // Use the action from yytable.
        yyrule = yytable_[yyrule];
        if (yyTableValueIsError(yyrule))
        {]b4_parse_trace_if([[
          yycdebugln(" Err");]])[
          return false;
        }
        if (0 < yyrule)
        {]b4_parse_trace_if([[
          yycdebugln(" S" ~ to!string(yyrule));]])[
          return true;
        }
        yyrule = -yyrule;
      }
      // By now we know we have to simulate a reduce.
]b4_parse_trace_if([[
      yycdebug(" R" ~ to!string(yyrule - 1));]])[
      // Pop the corresponding number of values from the stack.
      {
        int yylen = yyr2_[yyrule];
        // First pop from the LAC stack as many tokens as possible.
        int lacSize = cast (int) yylacStack.length;
        if (yylen < lacSize)
        {
          yylacStack.length -= yylen;
          yylen = 0;
        }
        else if (lacSize != 0)
        {
          destroy(yylacStack);
          yylen -= lacSize;
        }
        // Only afterwards look at the main stack.
        // We simulate popping elements by incrementing lacTop.
        lacTop += yylen;
      }
      // Keep topState in sync with the updated stack.
      topState = (yylacStack.length == 0
                  ? yystack.stateAt(lacTop)
                  : yylacStack[$ - 1]);
      // Push the resulting state of the reduction.
      int state = yyLRGotoState(topState, yyr1_[yyrule]);]b4_parse_trace_if([[
      yycdebug(" G" ~ to!string(state));]])[
      yylacStack.length++;
      yylacStack[$ - 1] = state;
    }
  }

  /** Establish the initial context if no initial context currently exists.
   * \returns  true iff the token will be eventually shifted.
   */
  bool yylacEstablish(YYStack yystack, SymbolKind yytoken)
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

     yylacEstablish should be invoked when a reduction is about to be
     performed in an inconsistent state (which, for the purposes of LAC,
     includes consistent states that don't know they're consistent because
     their default reductions have been disabled).

     For parse.lac=full, the implementation of yylacEstablish is as
     follows.  If no initial context is currently established for the
     current lookahead, then check if that lookahead can eventually be
     shifted if syntactic actions continue from the current context.  */
    if (yylacEstablished)
      return true;
    else
    {]b4_parse_trace_if([[
        yycdebugln("LAC: initial context established for " ~ format("%s", yytoken));]])[
        yylacEstablished = true;
        return yylacCheck(yystack, yytoken);
    }
  }

  /** Discard any previous initial lookahead context because of event.
   * \param event  the event which caused the lookahead to be discarded.
   *               Only used for debbuging output.  */
  void yylacDiscard(string event)
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
    if (yylacEstablished)
    {]b4_parse_trace_if([[
      yycdebugln("LAC: initial context discarded due to " ~ event);]])[
      yylacEstablished = false;
    }
  }

  /** The stack for LAC.
   * Logically, the yylacStack's lifetime is confined to the function
   * yylacCheck. We just store it as a member of this class to hold
   * on to the memory and to avoid frequent reallocations.
   */
  int[] yylacStack;
  /**  Whether an initial LAC context was established. */
  bool yylacEstablished;
]])[

  /**
   * Whether the given <code>yypact_</code> value indicates a defaulted state.
   * @@param yyvalue   the value to check
   */
  private static bool yyPactValueIsDefault(int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  /**
   * Whether the given <code>yytable_</code> value indicates a syntax error.
   * @@param yyvalue   the value to check
   */
  private static bool yyTableValueIsError(int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  private static immutable ]b4_int_type_for([b4_pact])[ yypact_ninf_ = ]b4_pact_ninf[;

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  private static immutable ]b4_int_type_for([b4_table])[ yytable_ninf_ = ]b4_table_ninf[;

  ]b4_parser_tables_define[

]b4_parse_trace_if([[
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  private static immutable ]b4_int_type_for([b4_rline])[[] yyrline_ =
  @{
  ]b4_rline[
  @};

  // Report on the debug stream that the rule yyrule is going to be reduced.
  private final void yy_reduce_print (int yyrule, ref YYStack yystack)
  {
    if (yydebug == 0)
      return;

    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    yycdebugln (format("Reducing stack by rule %d (line %d):",
                yyrule - 1, yylno));

    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      yy_symbol_print (format("   $%d =", yyi + 1),
                       to!SymbolKind (yystos_[yystack.stateAt(yynrhs - (yyi + 1))]),
                       ]b4_rhs_value(yynrhs, yyi + 1)b4_locations_if([,
                       b4_rhs_location(yynrhs, yyi + 1)])[);
  }
]])[

  private static auto yytranslate_ (int t)
  {
]b4_api_token_raw_if(
[[    return SymbolKind(t);]],
[[    /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
    immutable ]b4_int_type_for([b4_translate])[[] translate_table =
    @{
  ]b4_translate[
    @};

    // Last valid token kind.
    immutable int code_max = ]b4_code_max[;

    if (t <= 0)
      return ]b4_symbol(eof, kind)[;
    else if (t <= code_max)
      return SymbolKind(translate_table[t]);
    else
      return ]b4_symbol(undef, kind)[;]])[
  }

  private static immutable int yylast_ = ]b4_last[;
  private static immutable int yynnts_ = ]b4_nterms_number[;
  private static immutable int yyfinal_ = ]b4_final_state_number[;
  private static immutable int yyntokens_ = ]b4_tokens_number[;

  private final struct YYStackElement {
    int state;
    Value value;]b4_locations_if(
    b4_location_type[[] location;])[
  }

  private final struct YYStack {
    private YYStackElement[] stack = [];

    public final ulong height()
    {
      return stack.length;
    }

    public final void push (int state, Value value]dnl
  b4_locations_if([, ref Location loc])[)
    {
      stack ~= YYStackElement(state, value]b4_locations_if([, loc])[);
    }

    public final void pop ()
    {
      pop (1);
    }

    public final void pop (int num)
    {
      stack.length -= num;
    }

    public final int stateAt (int i) const
    {
      return stack[$-i-1].state;
    }

]b4_locations_if([[
    public final ref Location locationAt (int i)
    {
      return stack[$-i-1].location;
    }]])[

    public final ref Value valueAt (int i)
    {
      return stack[$-i-1].value;
    }
]b4_parse_trace_if([[
    // Print the state stack on the debug stream.
    public final void print (File stream)
    {
      stream.write ("Stack now");
      for (int i = 0; i < stack.length; i++)
        stream.write (" ", stack[i].state);
      stream.writeln ();
    }]])[
  }
]b4_percent_code_get[
}
]b4_percent_code_get([[epilogue]])[]dnl
b4_epilogue[]dnl
b4_output_end
