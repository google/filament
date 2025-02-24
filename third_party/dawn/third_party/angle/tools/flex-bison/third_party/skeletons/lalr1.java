#Java skeleton for Bison - *-java - *-

#Copyright(C) 2007 - 2015, 2018 - 2021 Free Software Foundation, Inc.

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
#along with this program.If not, see < https: // www.gnu.org/licenses/>.

m4_include(b4_skeletonsdir/[java.m4])

b4_header_if([b4_complain([%header/%defines does not make sense in Java])])

m4_define([b4_symbol_no_destructor_assert],
[b4_symbol_if([$1], [has_destructor],
              [b4_complain_at(m4_unquote(b4_symbol([$1], [destructor_loc])),
                              [%destructor does not make sense in Java])])])
b4_symbol_foreach([b4_symbol_no_destructor_assert])

## --------------- ##
## api.push-pull.  ##
## --------------- ##

b4_percent_define_default([[api.push-pull]], [[pull]])
b4_percent_define_check_values([[[[api.push-pull]],
                                 [[pull]], [[push]], [[both]]]])

#Define m4 conditional macros that encode the value
#of the api.push - pull flag.
b4_define_flag_if([pull]) m4_define([b4_pull_flag], [[1]])
b4_define_flag_if([push]) m4_define([b4_push_flag], [[1]])
m4_case(b4_percent_define_get([[api.push-pull]]),
        [pull], [m4_define([b4_push_flag], [[0]])],
        [push], [m4_define([b4_pull_flag], [[0]])])

#Define a macro to be true when api.push - pull has the value "both".
m4_define([b4_both_if],[b4_push_if([b4_pull_if([$1],[$2])],[$2])])

#Handle BISON_USE_PUSH_FOR_PULL for the test suite.So that push parsing
#tests function as written, do not let BISON_USE_PUSH_FOR_PULL modify the
#behavior of Bison at all when push parsing is already requested.
b4_define_flag_if([use_push_for_pull])
b4_use_push_for_pull_if([
  b4_push_if([m4_define([b4_use_push_for_pull_flag], [[0]])],
             [m4_define([b4_push_flag], [[1]])])])

#Define a macro to encapsulate the parse state variables.This
#allows them to be defined either in parse() when doing pull parsing,
#or as class instance variable when doing push parsing.
m4_define([b4_define_state],
[[
    /* Lookahead token kind.  */
    int yychar = YYEMPTY_;
    /* Lookahead symbol kind.  */
    SymbolKind yytoken = null;

    /* State.  */
    int yyn = 0;
    int yylen = 0;
    int yystate = 0;
    YYStack yystack = new YYStack ();
    int label = YYNEWSTATE;

]b4_locations_if([[
    /* The location where the error started.  */
    ]b4_location_type[ yyerrloc = null;

    /* Location. */
    ]b4_location_type[ yylloc = new ]b4_location_type[ (null, null);]])[

    /* Semantic value of the lookahead.  */
    ]b4_yystype[ yylval = null;
]])

#parse.lac
b4_percent_define_default([[parse.lac]], [[none]])
b4_percent_define_check_values([[[[parse.lac]], [[full]], [[none]]]])
b4_define_flag_if([lac])
m4_define([b4_lac_flag],
          [m4_if(b4_percent_define_get([[parse.lac]]),
                 [none], [[0]], [[1]])])


## ------------- ##
## Parser File.  ##
## ------------- ##

b4_output_begin([b4_parser_file_name])[
]b4_copyright([Skeleton implementation for Bison LALR(1) parsers in Java],
              [2007-2015, 2018-2021])[
]b4_disclaimer[
]b4_percent_define_ifdef([api.package], [package b4_percent_define_get([api.package]);[
]])[
]b4_user_pre_prologue[
]b4_user_post_prologue[
import java.text.MessageFormat;
import java.util.ArrayList;
]b4_percent_code_get([[imports]])[
/**
 * A Bison parser, automatically generated from <tt>]m4_bpatsubst(b4_file_name, [^"\(.*\)"$], [\1])[</tt>.
 *
 * @@author LALR (1) parser skeleton written by Paolo Bonzini.
 */
]b4_parser_class_declaration[
{
]b4_identification[
][
]b4_parse_error_bmatch(
           [detailed\|verbose], [[
  /**
   * True if verbose error messages are enabled.
   */
  private boolean yyErrorVerbose = true;

  /**
   * Whether verbose error messages are enabled.
   */
  public final boolean getErrorVerbose() {
        return yyErrorVerbose; }

  /**
   * Set the verbosity of error messages.
   * @@param verbose True to request verbose error messages.
   */
  public final void setErrorVerbose(boolean verbose)
  {
        yyErrorVerbose = verbose; }
]])[

]b4_locations_if([[
  /**
   * A class defining a pair of positions.  Positions, defined by the
   * <code>]b4_position_type[</code> class, denote a point in the input.
   * Locations represent a part of the input through the beginning
   * and ending positions.
   */
  public static class ]b4_location_type[ {
        /**
         * The first, inclusive, position in the range.
         */
    public ]b4_position_type[ begin;

    /**
     * The first position beyond the range.
     */
    public ]b4_position_type[ end;

    /**
     * Create a <code>]b4_location_type[</code> denoting an empty range located at
     * a given point.
     * @@param loc The position at which the range is anchored.
     */
    public ]b4_location_type[ (]b4_position_type[ loc) {
            this.begin = this.end = loc;
    }

    /**
     * Create a <code>]b4_location_type[</code> from the endpoints of the range.
     * @@param begin The first position included in the range.
     * @@param end   The first position beyond the range.
     */
    public ]b4_location_type[ (]b4_position_type[ begin, ]b4_position_type[ end) {
            this.begin = begin;
            this.end   = end;
    }

    /**
     * Print a representation of the location.  For this to be correct,
     * <code>]b4_position_type[</code> should override the <code>equals</code>
     * method.
     */
    public String toString() {
            if (begin.equals(end))
                return begin.toString();
            else
                return begin.toString() + "-" + end.toString();
    }
  }

  private ]b4_location_type[ yylloc(YYStack rhs, int n)
  {
        if (0 < n)
      return new ]b4_location_type[(rhs.locationAt(n-1).begin, rhs.locationAt(0).end);
        else
      return new ]b4_location_type[(rhs.locationAt(0).end);
  }]])[

]b4_declare_symbol_enum[

  /**
   * Communication interface between the scanner and the Bison-generated
   * parser <tt>]b4_parser_class[</tt>.
   */
  public interface Lexer {
]b4_token_enums[
    /** Deprecated, use ]b4_symbol(eof, id)[ instead.  */
    public static final int EOF = ]b4_symbol(eof, id)[;
]b4_pull_if([b4_locations_if([[
    /**
     * Method to retrieve the beginning position of the last scanned token.
     * @@return the position at which the last scanned token starts.
     */
    ]b4_position_type[ getStartPos();

    /**
     * Method to retrieve the ending position of the last scanned token.
     * @@return the first position beyond the last scanned token.
     */
    ]b4_position_type[ getEndPos();]])[

    /**
     * Method to retrieve the semantic value of the last scanned token.
     * @@return the semantic value of the last scanned token.
     */
    ]b4_yystype[ getLVal();

    /**
     * Entry point for the scanner.  Returns the token identifier corresponding
     * to the next token and prepares to return the semantic value
     * ]b4_locations_if([and beginning/ending positions ])[of the token.
     * @@return the token identifier corresponding to the next token.
     */
    int yylex()]b4_maybe_throws([b4_lex_throws])[;
]])[
    /**
     * Emit an error]b4_locations_if([ referring to the given location])[in a user-defined way.
     *
     *]b4_locations_if([[ @@param loc The location of the element to which the
     *                error message is related.]])[
     * @@param msg The string for the error message.
     */
     void yyerror(]b4_locations_if([b4_location_type[ loc, ]])[String msg);

]b4_parse_error_bmatch(
           [custom], [[
    /**
     * Build and emit a "syntax error" message in a user-defined way.
     *
     * @@param ctx  The context of the error.
     */
     void reportSyntaxError(Context ctx);
]])[
  }

]b4_lexer_if([[
  private class YYLexer implements Lexer {
]b4_percent_code_get([[lexer]])[
  }

]])[
  /**
   * The object doing lexical analysis for us.
   */
  private Lexer yylexer;

]b4_parse_param_vars[

]b4_lexer_if([[
  /**
   * Instantiates the Bison-generated parser.
   */
  public ]b4_parser_class[(]b4_parse_param_decl([b4_lex_param_decl])[)]b4_maybe_throws([b4_init_throws])[
  {
]b4_percent_code_get([[init]])[]b4_lac_if([[
    this.yylacStack = new ArrayList<Integer>();
    this.yylacEstablished = false;]])[
    this.yylexer = new YYLexer(]b4_lex_param_call[);
]b4_parse_param_cons[
  }
]])[

  /**
   * Instantiates the Bison-generated parser.
   * @@param yylexer The scanner that will supply tokens to the parser.
   */
  ]b4_lexer_if([[protected]], [[public]]) b4_parser_class[(]b4_parse_param_decl([[Lexer yylexer]])[)]b4_maybe_throws([b4_init_throws])[
  {
]b4_percent_code_get([[init]])[]b4_lac_if([[
    this.yylacStack = new ArrayList<Integer>();
    this.yylacEstablished = false;]])[
    this.yylexer = yylexer;
]b4_parse_param_cons[
  }

]b4_parse_trace_if([[
  private java.io.PrintStream yyDebugStream = System.err;

  /**
   * The <tt>PrintStream</tt> on which the debugging output is printed.
   */
  public final java.io.PrintStream getDebugStream() {
        return yyDebugStream; }

  /**
   * Set the <tt>PrintStream</tt> on which the debug output is printed.
   * @@param s The stream that is used for debugging output.
   */
  public final void setDebugStream(java.io.PrintStream s) {
        yyDebugStream = s; }

  private int yydebug = 0;

  /**
   * Answer the verbosity of the debugging output; 0 means that all kinds of
   * output from the parser are suppressed.
   */
  public final int getDebugLevel() {
        return yydebug; }

  /**
   * Set the verbosity of the debugging output; 0 means that all kinds of
   * output from the parser are suppressed.
   * @@param level The verbosity level for debugging output.
   */
  public final void setDebugLevel(int level) {
        yydebug = level; }
]])[

  private int yynerrs = 0;

  /**
   * The number of syntax errors so far.
   */
  public final int getNumberOfErrors() {
        return yynerrs; }

  /**
   * Print an error message via the lexer.
   *]b4_locations_if([[ Use a <code>null</code> location.]])[
   * @@param msg The error message.
   */
  public final void yyerror(String msg) {
      yylexer.yyerror(]b4_locations_if([[(]b4_location_type[)null, ]])[msg);
  }
]b4_locations_if([[
  /**
   * Print an error message via the lexer.
   * @@param loc The location associated with the message.
   * @@param msg The error message.
   */
  public final void yyerror(]b4_location_type[ loc, String msg) {
        yylexer.yyerror(loc, msg);
  }

  /**
   * Print an error message via the lexer.
   * @@param pos The position associated with the message.
   * @@param msg The error message.
   */
  public final void yyerror(]b4_position_type[ pos, String msg) {
      yylexer.yyerror(new ]b4_location_type[ (pos), msg);
  }]])[
]b4_parse_trace_if([[
  protected final void yycdebugNnl(String s) {
        if (0 < yydebug) yyDebugStream.print(s);
  }

  protected final void yycdebug(String s) {
        if (0 < yydebug) yyDebugStream.println(s);
  }]])[

  private final class YYStack {
        private int[] stateStack          = new int[16];]b4_locations_if([[
    private ]b4_location_type[[] locStack = new ]b4_location_type[[16];]])[
    private ]b4_yystype[[] valueStack = new ]b4_yystype[[16];

    public int size = 16;
    public int height = -1;

    public final void push(int state, ]b4_yystype[ value]b4_locations_if([, ]b4_location_type[ loc])[) {
            height++;
            if (size == height)
            {
                int[] newStateStack = new int[size * 2];
                System.arraycopy(stateStack, 0, newStateStack, 0, height);
                stateStack               = newStateStack;]b4_locations_if([[
        ]b4_location_type[[] newLocStack = new ]b4_location_type[[size * 2];
        System.arraycopy(locStack, 0, newLocStack, 0, height);
        locStack = newLocStack;]])

        b4_yystype[[] newValueStack = new ]b4_yystype[[size * 2];
        System.arraycopy(valueStack, 0, newValueStack, 0, height);
        valueStack = newValueStack;

        size *= 2;
            }

            stateStack[height] = state;]b4_locations_if([[
      locStack[height] = loc;]])[
      valueStack[height] = value;
    }

    public final void pop() {
            pop(1);
    }

    public final void pop(int num) {
            // Avoid memory leaks... garbage collection is a white lie!
            if (0 < num)
            {
                java.util.Arrays.fill(valueStack, height - num + 1, height + 1, null);]b4_locations_if([[
        java.util.Arrays.fill(locStack, height - num + 1, height + 1, null);]])[
            }
            height -= num;
    }

    public final int stateAt(int i) {
            return stateStack[height - i];
    }
]b4_locations_if([[

    public final ]b4_location_type[ locationAt(int i) {
            return locStack[height - i];
    }
]])[
    public final ]b4_yystype[ valueAt(int i) {
            return valueStack[height - i];
    }

    // Print the state stack on the debug stream.
    public void print(java.io.PrintStream out) {
            out.print("Stack now");

            for (int i = 0; i <= height; i++)
            {
                out.print(' ');
                out.print(stateStack[i]);
            }
            out.println();
    }
  }

  /**
   * Returned by a Bison action in order to stop the parsing process and
   * return success (<tt>true</tt>).
   */
  public static final int YYACCEPT = 0;

  /**
   * Returned by a Bison action in order to stop the parsing process and
   * return failure (<tt>false</tt>).
   */
  public static final int YYABORT = 1;

]b4_push_if([
  /**
   * Returned by a Bison action in order to request a new token.
   */
  public static final int YYPUSH_MORE = 4;])[

  /**
   * Returned by a Bison action in order to start error recovery without
   * printing an error message.
   */
  public static final int YYERROR = 2;

  /**
   * Internal return codes that are not supported for user semantic
   * actions.
   */
  private static final int YYERRLAB = 3;
  private static final int YYNEWSTATE = 4;
  private static final int YYDEFAULT = 5;
  private static final int YYREDUCE = 6;
  private static final int YYERRLAB1 = 7;
  private static final int YYRETURN = 8;
]b4_push_if([[  private static final int YYGETTOKEN = 9; /* Signify that a new token is expected when doing push-parsing.  */]])[

  private int yyerrstatus_ = 0;

]b4_push_if([b4_define_state])[
  /**
   * Whether error recovery is being done.  In this state, the parser
   * reads token until it reaches a known state, and then restarts normal
   * operation.
   */
  public final boolean recovering ()
  {
        return yyerrstatus_ == 0;
  }

  /** Compute post-reduction state.
   * @@param yystate   the current state
   * @@param yysym     the nonterminal to push on the stack
   */
  private int yyLRGotoState(int yystate, int yysym) {
        int yyr = yypgoto_[yysym - YYNTOKENS_] + yystate;
        if (0 <= yyr && yyr <= YYLAST_ && yycheck_[yyr] == yystate)
            return yytable_[yyr];
        else
            return yydefgoto_[yysym - YYNTOKENS_];
  }

  private int yyaction(int yyn, YYStack yystack, int yylen)]b4_maybe_throws([b4_throws])[
  {
        /* If YYLEN is nonzero, implement the default value of the action:
           '$$ = $1'.  Otherwise, use the top of the stack.

           Otherwise, the following line sets YYVAL to garbage.
           This behavior is undocumented and Bison
           users should not rely upon it.  */
    ]b4_yystype[ yyval = (0 < yylen) ? yystack.valueAt(yylen - 1) : yystack.valueAt(0);]b4_locations_if([[
    ]b4_location_type[ yyloc = yylloc(yystack, yylen);]])[]b4_parse_trace_if([[

    yyReducePrint(yyn, yystack);]])[

    switch (yyn)
      {
        ]b4_user_actions[
        default: break;
      }]b4_parse_trace_if([[

    yySymbolPrint("-> $$ =", SymbolKind.get(yyr1_[yyn]), yyval]b4_locations_if([, yyloc])[);]])[

    yystack.pop(yylen);
    yylen = 0;
    /* Shift the result of the reduction.  */
    int yystate = yyLRGotoState(yystack.stateAt(0), yyr1_[yyn]);
    yystack.push(yystate, yyval]b4_locations_if([, yyloc])[);
    return YYNEWSTATE;
  }

]b4_parse_trace_if([[
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  private void yySymbolPrint(String s, SymbolKind yykind,
                             ]b4_yystype[ yyvalue]b4_locations_if([, ]b4_location_type[ yylocation])[) {
        if (0 < yydebug)
        {
          yycdebug(s
                   + (yykind.getCode() < YYNTOKENS_ ? " token " : " nterm ")
                   + yykind.getName() + " ("]b4_locations_if([
                   + yylocation + ": "])[
                   + (yyvalue == null ? "(null)" : yyvalue.toString()) + ")");
        }
  }]])[

]b4_push_if([],[[
  /**
   * Parse input from the scanner that was specified at object construction
   * time.  Return whether the end of the input was reached successfully.
   *
   * @@return <tt>true</tt> if the parsing succeeds.  Note that this does not
   *          imply that there were no syntax errors.
   */
  public boolean parse()]b4_maybe_throws([b4_list2([b4_lex_throws], [b4_throws])])[]])[
]b4_push_if([
  /**
   * Push Parse input from external lexer
   *
   * @@param yylextoken current token
   * @@param yylexval current lval]b4_locations_if([[
   * @@param yylexloc current position]])[
   *
   * @@return <tt>YYACCEPT, YYABORT, YYPUSH_MORE</tt>
   */
  public int push_parse(int yylextoken, b4_yystype yylexval[]b4_locations_if([, b4_location_type yylexloc]))b4_maybe_throws([b4_list2([b4_lex_throws], [b4_throws])])])[
  {]b4_locations_if([[
    /* @@$.  */
    ]b4_location_type[ yyloc;]])[
]b4_push_if([],[[
]b4_define_state[
]b4_lac_if([[
    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yylacDiscard("init");]])[
]b4_parse_trace_if([[
    yycdebug ("Starting parse");]])[
    yyerrstatus_ = 0;
    yynerrs = 0;

    /* Initialize the stack.  */
    yystack.push (yystate, yylval]b4_locations_if([, yylloc])[);
]m4_ifdef([b4_initial_action], [
b4_dollar_pushdef([yylval], [], [], [yylloc])dnl
    b4_user_initial_action
b4_dollar_popdef[]dnl
])[
]])[
]b4_push_if([[
    if (!this.push_parse_initialized)
      {
            push_parse_initialize();
]m4_ifdef([b4_initial_action], [
b4_dollar_pushdef([yylval], [], [], [yylloc])dnl
    b4_user_initial_action
b4_dollar_popdef[]dnl
])[]b4_parse_trace_if([[
        yycdebug ("Starting parse");]])[
        yyerrstatus_ = 0;
      } else
        label = YYGETTOKEN;

    boolean push_token_consumed = true;
]])[
    for (;;)
      switch (label)
      {
                /* New state.  Unlike in the C/C++ skeletons, the state is already
                   pushed when we come here.  */
            case YYNEWSTATE:]b4_parse_trace_if([[
        yycdebug ("Entering state " + yystate);
        if (0 < yydebug)
          yystack.print (yyDebugStream);]])[

        /* Accept?  */
        if (yystate == YYFINAL_)
          ]b4_push_if([{
                    label = YYACCEPT;
                    break;}],
                      [return true;])[

        /* Take a decision.  First try without lookahead.  */
        yyn = yypact_[yystate];
        if (yyPactValueIsDefault (yyn))
          {
                    label = YYDEFAULT;
                    break;
          }
]b4_push_if([        /* Fall Through */

      case YYGETTOKEN:])[
        /* Read a lookahead token.  */
        if (yychar == YYEMPTY_)
          {
]b4_push_if([[
            if (!push_token_consumed)
              return YYPUSH_MORE;]b4_parse_trace_if([[
            yycdebug ("Reading a token");]])[
            yychar = yylextoken;
            yylval = yylexval;]b4_locations_if([
            yylloc = yylexloc;])[
            push_token_consumed = false;]], [b4_parse_trace_if([[
            yycdebug ("Reading a token");]])[
            yychar = yylexer.yylex ();
            yylval = yylexer.getLVal();]b4_locations_if([[
            yylloc = new ]b4_location_type[(yylexer.getStartPos(),
                                          yylexer.getEndPos());]])[
]])[
          }

        /* Convert token to internal form.  */
        yytoken = yytranslate_ (yychar);]b4_parse_trace_if([[
        yySymbolPrint("Next token is", yytoken,
                      yylval]b4_locations_if([, yylloc])[);]])[

        if (yytoken == ]b4_symbol(error, kind)[)
          {
                    // The scanner already issued an error message, process directly
                    // to error recovery.  But do not keep the error token as
                    // lookahead, it is too special and may lead us to an endless
                    // loop in error recovery. */
            yychar = Lexer.]b4_symbol(undef, id)[;
            yytoken = ]b4_symbol(undef, kind)[;]b4_locations_if([[
            yyerrloc = yylloc;]])[
            label = YYERRLAB1;
          }
        else
          {
                    /* If the proper action on seeing token YYTOKEN is to reduce or to
                       detect an error, take that action.  */
                    yyn += yytoken.getCode();
                    if (yyn < 0 || YYLAST_ < yyn || yycheck_[yyn] != yytoken.getCode())
                    {]b4_lac_if([[
              if (!yylacEstablish(yystack, yytoken)) {
                            label = YYERRLAB;
              } else]])[
              label = YYDEFAULT;
                    }

                    /* <= 0 means reduce or error.  */
                    else if ((yyn = yytable_[yyn]) <= 0)
                    {
                        if (yyTableValueIsError(yyn))
                        {
                            label = YYERRLAB;
                        }]b4_lac_if([[ else if (!yylacEstablish(yystack, yytoken)) {
                            label = YYERRLAB;
                }]])[ else {
                            yyn   = -yyn;
                            label = YYREDUCE;
                }
                    }

                    else
                    {
                /* Shift the lookahead token.  */]b4_parse_trace_if([[
                yySymbolPrint("Shifting", yytoken,
                              yylval]b4_locations_if([, yylloc])[);
]])[
                /* Discard the token being shifted.  */
                yychar = YYEMPTY_;

                /* Count tokens shifted since error; after three, turn off error
                   status.  */
                if (yyerrstatus_ > 0)
                  --yyerrstatus_;

                yystate = yyn;
                yystack.push(yystate, yylval]b4_locations_if([, yylloc])[);]b4_lac_if([[
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
        label = yyaction(yyn, yystack, yylen);
        yystate = yystack.stateAt(0);
        break;

      /*------------------------------------.
      | yyerrlab -- here on detecting error |
      `------------------------------------*/
      case YYERRLAB:
        /* If not already recovering from an error, report this error.  */
        if (yyerrstatus_ == 0)
          {
                    ++yynerrs;
                    if (yychar == YYEMPTY_) yytoken = null;
            yyreportSyntaxError(new Context(this, yystack, yytoken]b4_locations_if([[, yylloc]])[));
          }
]b4_locations_if([[
        yyerrloc = yylloc;]])[
        if (yyerrstatus_ == 3)
          {
                    /* If just tried and failed to reuse lookahead token after an
                       error, discard it.  */

            if (yychar <= Lexer.]b4_symbol(eof, id)[)
            {
                /* Return failure if at end of input.  */
                if (yychar == Lexer.]b4_symbol(eof, id)[)
                  ]b4_push_if([{
                                label = YYABORT;
                                break;}], [return false;])[
            }
            else
                yychar = YYEMPTY_;
          }

        /* Else will try to reuse lookahead token after shifting the error
           token.  */
        label = YYERRLAB1;
        break;

      /*-------------------------------------------------.
      | errorlab -- error raised explicitly by YYERROR.  |
      `-------------------------------------------------*/
      case YYERROR:]b4_locations_if([[
        yyerrloc = yystack.locationAt (yylen - 1);]])[
        /* Do not reclaim the symbols of the rule which action triggered
           this YYERROR.  */
        yystack.pop (yylen);
        yylen = 0;
        yystate = yystack.stateAt(0);
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
                yyn += ]b4_symbol(error, kind)[.getCode();
                if (0 <= yyn && yyn <= YYLAST_
                    && yycheck_[yyn] == ]b4_symbol(error, kind)[.getCode())
                  {
                            yyn = yytable_[yyn];
                            if (0 < yyn) break;
                  }
                    }

                    /* Pop the current state because it cannot handle the
                     * error token.  */
                    if (yystack.height == 0)
              ]b4_push_if([{
                            label = YYABORT;
                            break;}],[return false;])[

]b4_locations_if([[
            yyerrloc = yystack.locationAt (0);]])[
            yystack.pop ();
            yystate = yystack.stateAt(0);]b4_parse_trace_if([[
            if (0 < yydebug)
              yystack.print (yyDebugStream);]])[
          }

        if (label == YYABORT)
          /* Leave the switch.  */
          break;

]b4_locations_if([[
        /* Muck with the stack to setup for yylloc.  */
        yystack.push (0, null, yylloc);
        yystack.push (0, null, yyerrloc);
        yyloc = yylloc (yystack, 2);
        yystack.pop (2);]])[

        /* Shift the error token.  */]b4_lac_if([[
        yylacDiscard("error recovery");]])[]b4_parse_trace_if([[
        yySymbolPrint("Shifting", SymbolKind.get(yystos_[yyn]),
                      yylval]b4_locations_if([, yyloc])[);]])[

        yystate = yyn;
        yystack.push (yyn, yylval]b4_locations_if([, yyloc])[);
        label = YYNEWSTATE;
        break;

        /* Accept.  */
      case YYACCEPT:
        ]b4_push_if([this.push_parse_initialized = false; return YYACCEPT;],
                    [return true;])[

        /* Abort.  */
      case YYABORT:
        ]b4_push_if([this.push_parse_initialized = false; return YYABORT;],
                    [return false;])[
      }
}
]b4_push_if([[
  boolean push_parse_initialized = false;

    /**
     * (Re-)Initialize the state of the push parser.
     */
  public void push_parse_initialize ()
  {
        /* Lookahead and lookahead in internal form.  */
        this.yychar  = YYEMPTY_;
        this.yytoken = null;

        /* State.  */
        this.yyn          = 0;
        this.yylen        = 0;
        this.yystate      = 0;
        this.yystack      = new YYStack();]b4_lac_if([[
    this.yylacStack = new ArrayList<Integer>();
    this.yylacEstablished = false;]])[
    this.label = YYNEWSTATE;

    /* Error handling.  */
    this.yynerrs = 0;]b4_locations_if([[
    /* The location where the error started.  */
    this.yyerrloc = null;
    this.yylloc = new ]b4_location_type[ (null, null);]])[

    /* Semantic value of the lookahead.  */
    this.yylval = null;

    yystack.push (this.yystate, this.yylval]b4_locations_if([, this.yylloc])[);

    this.push_parse_initialized = true;

  }
]b4_locations_if([[
  /**
   * Push parse given input from an external lexer.
   *
   * @@param yylextoken current token
   * @@param yylexval current lval
   * @@param yyylexpos current position
   *
   * @@return <tt>YYACCEPT, YYABORT, YYPUSH_MORE</tt>
   */
  public int push_parse(int yylextoken, ]b4_yystype[ yylexval, ]b4_position_type[ yylexpos)]b4_maybe_throws([b4_list2([b4_lex_throws], [b4_throws])])[ {
      return push_parse(yylextoken, yylexval, new ]b4_location_type[(yylexpos));
  }
]])])[

]b4_both_if([[
  /**
   * Parse input from the scanner that was specified at object construction
   * time.  Return whether the end of the input was reached successfully.
   * This version of parse() is defined only when api.push-push=both.
   *
   * @@return <tt>true</tt> if the parsing succeeds.  Note that this does not
   *          imply that there were no syntax errors.
   */
  public boolean parse()]b4_maybe_throws([b4_list2([b4_lex_throws], [b4_throws])])[ {
        if (yylexer == null) throw new NullPointerException("Null Lexer");
        int status;
        do
        {
            int token       = yylexer.yylex();
          ]b4_yystype[ lval = yylexer.getLVal();]b4_locations_if([[
          ]b4_location_type[ yyloc = new ]b4_location_type[(yylexer.getStartPos(), yylexer.getEndPos());
          status = push_parse(token, lval, yyloc);]], [[
          status = push_parse(token, lval);]])[
        } while (status == YYPUSH_MORE);
        return status == YYACCEPT;
  }
]])[

  /**
   * Information needed to get the list of expected tokens and to forge
   * a syntax error diagnostic.
   */
  public static final class Context {
    Context(]b4_parser_class[ parser, YYStack stack, SymbolKind token]b4_locations_if([[, ]b4_location_type[ loc]])[)
    {
        yyparser = parser;
        yystack  = stack;
        yytoken  = token;]b4_locations_if([[
      yylocation = loc;]])[
    }

    private ]b4_parser_class[ yyparser;
    private YYStack yystack;


    /**
     * The symbol kind of the lookahead token.
     */
    public final SymbolKind getToken() {
            return yytoken;
    }

    private SymbolKind yytoken;]b4_locations_if([[

    /**
     * The location of the lookahead.
     */
    public final ]b4_location_type[ getLocation() {
            return yylocation;
    }

    private ]b4_location_type[ yylocation;]])[
    static final int NTOKENS = ]b4_parser_class[.YYNTOKENS_;

    /**
     * Put in YYARG at most YYARGN of the expected tokens given the
     * current YYCTX, and return the number of tokens stored in YYARG.  If
     * YYARG is null, return the number of expected tokens (guaranteed to
     * be less than YYNTOKENS).
     */
    int getExpectedTokens(SymbolKind yyarg[], int yyargn) {
            return getExpectedTokens(yyarg, 0, yyargn);
    }

    int getExpectedTokens(SymbolKind yyarg[], int yyoffset, int yyargn) {
            int yycount = yyoffset;]b4_lac_if([b4_parse_trace_if([[
      // Execute LAC once. We don't care if it is successful, we
      // only do it for the sake of debugging output.
      if (!yyparser.yylacEstablished)
        yyparser.yylacCheck(yystack, yytoken);
]])[
      for (int yyx = 0; yyx < YYNTOKENS_; ++yyx)
        {
                SymbolKind yysym = SymbolKind.get(yyx);
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
                int yychecklim = YYLAST_ - yyn + 1;
                int yyxend     = yychecklim < NTOKENS ? yychecklim : NTOKENS;
                for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck_[yyx + yyn] == yyx && yyx != ]b4_symbol(error, kind)[.getCode()
                && !yyTableValueIsError(yytable_[yyx + yyn]))
            {
                if (yyarg == null)
                    yycount += 1;
                else if (yycount == yyargn)
                    return 0; // FIXME: this is incorrect.
                else
                    yyarg[yycount++] = SymbolKind.get(yyx);
            }
        }]])[
      if (yyarg != null && yycount == yyoffset && yyoffset < yyargn)
        yyarg[yycount] = null;
      return yycount - yyoffset;
    }
  }

]b4_lac_if([[
    /** Check the lookahead yytoken.
     * \returns  true iff the token will be eventually shifted.
     */
    boolean yylacCheck(YYStack yystack, SymbolKind yytoken)
    {
        // Logically, the yylacStack's lifetime is confined to this function.
        // Clear it, to get rid of potential left-overs from previous call.
        yylacStack.clear();
        // Reduce until we encounter a shift and thereby accept the token.
        yycdebugNnl("LAC: checking lookahead " + yytoken.getName() + ":");
        int lacTop = 0;
        while (true)
        {
            int topState = (yylacStack.isEmpty() ? yystack.stateAt(lacTop)
                                                 : yylacStack.get(yylacStack.size() - 1));
            int yyrule   = yypact_[topState];
            if (yyPactValueIsDefault(yyrule) || (yyrule += yytoken.getCode()) < 0
                    || YYLAST_ < yyrule || yycheck_[yyrule] != yytoken.getCode())
            {
                // Use the default action.
                yyrule = yydefact_[+topState];
                if (yyrule == 0)
                {
                    yycdebug(" Err");
                    return false;
                }
            }
            else
            {
                // Use the action from yytable.
                yyrule = yytable_[yyrule];
                if (yyTableValueIsError(yyrule))
                {
                    yycdebug(" Err");
                    return false;
                }
                if (0 < yyrule)
                {
                    yycdebug(" S" + yyrule);
                    return true;
                }
                yyrule = -yyrule;
            }
            // By now we know we have to simulate a reduce.
            yycdebugNnl(" R" + (yyrule - 1));
            // Pop the corresponding number of values from the stack.
            {
                int yylen = yyr2_[yyrule];
                // First pop from the LAC stack as many tokens as possible.
                int lacSize = yylacStack.size();
                if (yylen < lacSize)
                {
                    // yylacStack.setSize(lacSize - yylen);
                    for (/* Nothing */; 0 < yylen; yylen -= 1)
                    {
                        yylacStack.remove(yylacStack.size() - 1);
                    }
                    yylen = 0;
                }
                else if (lacSize != 0)
                {
                    yylacStack.clear();
                    yylen -= lacSize;
                }
                // Only afterwards look at the main stack.
                // We simulate popping elements by incrementing lacTop.
                lacTop += yylen;
            }
            // Keep topState in sync with the updated stack.
            topState = (yylacStack.isEmpty() ? yystack.stateAt(lacTop)
                                             : yylacStack.get(yylacStack.size() - 1));
            // Push the resulting state of the reduction.
            int state = yyLRGotoState(topState, yyr1_[yyrule]);
            yycdebugNnl(" G" + state);
            yylacStack.add(state);
        }
    }

    /** Establish the initial context if no initial context currently exists.
     * \returns  true iff the token will be eventually shifted.
     */
    boolean yylacEstablish(YYStack yystack, SymbolKind yytoken) {
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
        {
            return true;
        }
        else
        {
            yycdebug("LAC: initial context established for " + yytoken.getName());
            yylacEstablished = true;
            return yylacCheck(yystack, yytoken);
        }
    }

    /** Discard any previous initial lookahead context because of event.
     * \param event  the event which caused the lookahead to be discarded.
     *               Only used for debbuging output.  */
    void yylacDiscard(String event) {
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
        {
            yycdebug("LAC: initial context discarded due to " + event);
            yylacEstablished = false;
        }
    }

    /** The stack for LAC.
     * Logically, the yylacStack's lifetime is confined to the function
     * yylacCheck. We just store it as a member of this class to hold
     * on to the memory and to avoid frequent reallocations.
     */
    ArrayList<Integer> yylacStack;
    /**  Whether an initial LAC context was established. */
    boolean yylacEstablished;
]])[

]b4_parse_error_bmatch(
[detailed\|verbose], [[
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
        if (yyctx.getToken() != null)
        {
            if (yyarg != null) yyarg[yycount] = yyctx.getToken();
            yycount += 1;
            yycount += yyctx.getExpectedTokens(yyarg, 1, yyargn);
        }
        return yycount;
  }
]])[

  /**
   * Build and emit a "syntax error" message in a user-defined way.
   *
   * @@param ctx  The context of the error.
   */
  private void yyreportSyntaxError(Context yyctx) {]b4_parse_error_bmatch(
[custom], [[
      yylexer.reportSyntaxError(yyctx);]],
[detailed\|verbose], [[
      if (yyErrorVerbose) {
            final int argmax   = 5;
            SymbolKind[] yyarg = new SymbolKind[argmax];
            int yycount        = yysyntaxErrorArguments(yyctx, yyarg, argmax);
            String[] yystr     = new String[yycount];
            for (int yyi = 0; yyi < yycount; ++yyi)
            {
                yystr[yyi] = yyarg[yyi].getName();
            }
            String yyformat;
            switch (yycount)
            {
                default:
                case 0: yyformat = ]b4_trans(["syntax error"])[; break;
              case 1: yyformat = ]b4_trans(["syntax error, unexpected {0}"])[; break;
              case 2: yyformat = ]b4_trans(["syntax error, unexpected {0}, expecting {1}"])[; break;
              case 3: yyformat = ]b4_trans(["syntax error, unexpected {0}, expecting {1} or {2}"])[; break;
              case 4: yyformat = ]b4_trans(["syntax error, unexpected {0}, expecting {1} or {2} or {3}"])[; break;
              case 5: yyformat = ]b4_trans(["syntax error, unexpected {0}, expecting {1} or {2} or {3} or {4}"])[; break;
            }
          yyerror(]b4_locations_if([[yyctx.yylocation, ]])[new MessageFormat(yyformat).format(yystr));
      } else {
          yyerror(]b4_locations_if([[yyctx.yylocation, ]])[]b4_trans(["syntax error"])[);
      }]],
[simple], [[
      yyerror(]b4_locations_if([[yyctx.yylocation, ]])[]b4_trans(["syntax error"])[);]])[
  }

  /**
   * Whether the given <code>yypact_</code> value indicates a defaulted state.
   * @@param yyvalue   the value to check
   */
  private static boolean yyPactValueIsDefault(int yyvalue) {
        return yyvalue == yypact_ninf_;
  }

  /**
   * Whether the given <code>yytable_</code>
   * value indicates a syntax error.
   * @@param yyvalue the value to check
   */
  private static boolean yyTableValueIsError(int yyvalue) {
        return yyvalue == yytable_ninf_;
  }

  private static final ]b4_int_type_for([b4_pact])[ yypact_ninf_ = ]b4_pact_ninf[;
  private static final ]b4_int_type_for([b4_table])[ yytable_ninf_ = ]b4_table_ninf[;

]b4_parser_tables_define[

]b4_parse_trace_if([[
  ]b4_integral_parser_table_define([rline], [b4_rline],
  [[YYRLINE[YYN] -- Source line where rule number YYN was defined.]])[


  // Report on the debug stream that the rule yyrule is going to be reduced.
  private void yyReducePrint (int yyrule, YYStack yystack)
  {
        if (yydebug == 0) return;

        int yylno  = yyrline_[yyrule];
        int yynrhs = yyr2_[yyrule];
        /* Print the symbols being reduced, and their result.  */
        yycdebug("Reducing stack by rule " + (yyrule - 1) + " (line " + yylno + "):");

        /* The symbols being reduced.  */
        for (int yyi = 0; yyi < yynrhs; yyi++)
      yySymbolPrint("   $" + (yyi + 1) + " =",
                    SymbolKind.get(yystos_[yystack.stateAt(yynrhs - (yyi + 1))]),
                    ]b4_rhs_data(yynrhs, yyi + 1)b4_locations_if([,
                    b4_rhs_location(yynrhs, yyi + 1)])[);
  }]])[

  /* YYTRANSLATE_(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
     as returned by yylex, with out-of-bounds checking.  */
  private static final SymbolKind yytranslate_(int t)
]b4_api_token_raw_if(dnl
[[  {
        return SymbolKind.get(t);
  }
]],
[[  {
        // Last valid token kind.
    int code_max = ]b4_code_max[;
    if (t <= 0)
      return ]b4_symbol(eof, kind)[;
    else if (t <= code_max)
      return SymbolKind.get(yytranslate_table_[t]);
    else
      return ]b4_symbol(undef, kind)[;
  }
  ]b4_integral_parser_table_define([translate_table], [b4_translate])[
]])[

  private static final int YYLAST_ = ]b4_last[;
  private static final int YYEMPTY_ = -2;
  private static final int YYFINAL_ = ]b4_final_state_number[;
  private static final int YYNTOKENS_ = ]b4_tokens_number[;

]b4_percent_code_get[
}
]b4_percent_code_get([[epilogue]])[]dnl
b4_epilogue[]dnl
b4_output_end
