/**
 * The jsonPath evaluator compiler used for Oboe.js. 
 * 
 * One function is exposed. This function takes a String JSONPath spec and 
 * returns a function to test candidate ascents for matches.
 * 
 *  String jsonPath -> (List ascent) -> Boolean|Object
 *
 * This file is coded in a pure functional style. That is, no function has 
 * side effects, every function evaluates to the same value for the same 
 * arguments and no variables are reassigned.
 */  
// the call to jsonPathSyntax injects the token syntaxes that are needed 
// inside the compiler
var jsonPathCompiler = jsonPathSyntax(function (pathNodeSyntax, 
                                                doubleDotSyntax, 
                                                dotSyntax,
                                                bangSyntax,
                                                emptySyntax ) {

   var CAPTURING_INDEX = 1;
   var NAME_INDEX = 2;
   var FIELD_LIST_INDEX = 3;

   var headKey  = compose2(keyOf, head),
       headNode = compose2(nodeOf, head);
                   
   /**
    * Create an evaluator function for a named path node, expressed in the
    * JSONPath like:
    *    foo
    *    ["bar"]
    *    [2]   
    */
   function nameClause(previousExpr, detection ) {
     
      var name = detection[NAME_INDEX],
            
          matchesName = ( !name || name == '*' ) 
                           ?  always
                           :  function(ascent){return headKey(ascent) == name};
     

      return lazyIntersection(matchesName, previousExpr);
   }

   /**
    * Create an evaluator function for a a duck-typed node, expressed like:
    * 
    *    {spin, taste, colour}
    *    .particle{spin, taste, colour}
    *    *{spin, taste, colour}
    */
   function duckTypeClause(previousExpr, detection) {

      var fieldListStr = detection[FIELD_LIST_INDEX];

      if (!fieldListStr) 
         return previousExpr; // don't wrap at all, return given expr as-is      

      var hasAllrequiredFields = partialComplete(
                                    hasAllProperties, 
                                    arrayAsList(fieldListStr.split(/\W+/))
                                 ),
                                 
          isMatch =  compose2( 
                        hasAllrequiredFields, 
                        headNode
                     );

      return lazyIntersection(isMatch, previousExpr);
   }

   /**
    * Expression for $, returns the evaluator function
    */
   function capture( previousExpr, detection ) {

      // extract meaning from the detection      
      var capturing = !!detection[CAPTURING_INDEX];

      if (!capturing)          
         return previousExpr; // don't wrap at all, return given expr as-is      
      
      return lazyIntersection(previousExpr, head);
            
   }            
      
   /**
    * Create an evaluator function that moves onto the next item on the 
    * lists. This function is the place where the logic to move up a 
    * level in the ascent exists. 
    * 
    * Eg, for JSONPath ".foo" we need skip1(nameClause(always, [,'foo']))
    */
   function skip1(previousExpr) {
   
   
      if( previousExpr == always ) {
         /* If there is no previous expression this consume command 
            is at the start of the jsonPath.
            Since JSONPath specifies what we'd like to find but not 
            necessarily everything leading down to it, when running
            out of JSONPath to check against we default to true */
         return always;
      }

      /** return true if the ascent we have contains only the JSON root,
       *  false otherwise
       */
      function notAtRoot(ascent){
         return headKey(ascent) != ROOT_PATH;
      }
      
      return lazyIntersection(
               /* If we're already at the root but there are more 
                  expressions to satisfy, can't consume any more. No match.

                  This check is why none of the other exprs have to be able 
                  to handle empty lists; skip1 is the only evaluator that 
                  moves onto the next token and it refuses to do so once it 
                  reaches the last item in the list. */
               notAtRoot,
               
               /* We are not at the root of the ascent yet.
                  Move to the next level of the ascent by handing only 
                  the tail to the previous expression */ 
               compose2(previousExpr, tail) 
      );
                                                                                                               
   }   
   
   /**
    * Create an evaluator function for the .. (double dot) token. Consumes
    * zero or more levels of the ascent, the fewest that are required to find
    * a match when given to previousExpr.
    */   
   function skipMany(previousExpr) {

      if( previousExpr == always ) {
         /* If there is no previous expression this consume command 
            is at the start of the jsonPath.
            Since JSONPath specifies what we'd like to find but not 
            necessarily everything leading down to it, when running
            out of JSONPath to check against we default to true */            
         return always;
      }
          
      var 
          // In JSONPath .. is equivalent to !.. so if .. reaches the root
          // the match has succeeded. Ie, we might write ..foo or !..foo
          // and both should match identically.
          terminalCaseWhenArrivingAtRoot = rootExpr(),
          terminalCaseWhenPreviousExpressionIsSatisfied = previousExpr,
          recursiveCase = skip1(function(ascent) {
             return cases(ascent);
          }),

          cases = lazyUnion(
                     terminalCaseWhenArrivingAtRoot
                  ,  terminalCaseWhenPreviousExpressionIsSatisfied
                  ,  recursiveCase  
                  );
      
      return cases;
   }      
   
   /**
    * Generate an evaluator for ! - matches only the root element of the json
    * and ignores any previous expressions since nothing may precede !. 
    */   
   function rootExpr() {
      
      return function(ascent){
         return headKey(ascent) == ROOT_PATH;
      };
   }   
         
   /**
    * Generate a statement wrapper to sit around the outermost 
    * clause evaluator.
    * 
    * Handles the case where the capturing is implicit because the JSONPath
    * did not contain a '$' by returning the last node.
    */   
   function statementExpr(lastClause) {
      
      return function(ascent) {
   
         // kick off the evaluation by passing through to the last clause
         var exprMatch = lastClause(ascent);
                                                     
         return exprMatch === true ? head(ascent) : exprMatch;
      };
   }      
                          
   /**
    * For when a token has been found in the JSONPath input.
    * Compiles the parser for that token and returns in combination with the
    * parser already generated.
    * 
    * @param {Function} exprs  a list of the clause evaluator generators for
    *                          the token that was found
    * @param {Function} parserGeneratedSoFar the parser already found
    * @param {Array} detection the match given by the regex engine when 
    *                          the feature was found
    */
   function expressionsReader( exprs, parserGeneratedSoFar, detection ) {
                     
      // if exprs is zero-length foldR will pass back the 
      // parserGeneratedSoFar as-is so we don't need to treat 
      // this as a special case
      
      return   foldR( 
                  function( parserGeneratedSoFar, expr ){
         
                     return expr(parserGeneratedSoFar, detection);
                  }, 
                  parserGeneratedSoFar, 
                  exprs
               );                     

   }

   /** 
    *  If jsonPath matches the given detector function, creates a function which
    *  evaluates against every clause in the clauseEvaluatorGenerators. The
    *  created function is propagated to the onSuccess function, along with
    *  the remaining unparsed JSONPath substring.
    *  
    *  The intended use is to create a clauseMatcher by filling in
    *  the first two arguments, thus providing a function that knows
    *  some syntax to match and what kind of generator to create if it
    *  finds it. The parameter list once completed is:
    *  
    *    (jsonPath, parserGeneratedSoFar, onSuccess)
    *  
    *  onSuccess may be compileJsonPathToFunction, to recursively continue 
    *  parsing after finding a match or returnFoundParser to stop here.
    */
   function generateClauseReaderIfTokenFound (
     
                        tokenDetector, clauseEvaluatorGenerators,
                         
                        jsonPath, parserGeneratedSoFar, onSuccess) {
                        
      var detected = tokenDetector(jsonPath);

      if(detected) {
         var compiledParser = expressionsReader(
                                 clauseEvaluatorGenerators, 
                                 parserGeneratedSoFar, 
                                 detected
                              ),
         
             remainingUnparsedJsonPath = jsonPath.substr(len(detected[0]));                
                               
         return onSuccess(remainingUnparsedJsonPath, compiledParser);
      }         
   }
                 
   /**
    * Partially completes generateClauseReaderIfTokenFound above. 
    */
   function clauseMatcher(tokenDetector, exprs) {
        
      return   partialComplete( 
                  generateClauseReaderIfTokenFound, 
                  tokenDetector, 
                  exprs 
               );
   }

   /**
    * clauseForJsonPath is a function which attempts to match against 
    * several clause matchers in order until one matches. If non match the
    * jsonPath expression is invalid and an error is thrown.
    * 
    * The parameter list is the same as a single clauseMatcher:
    * 
    *    (jsonPath, parserGeneratedSoFar, onSuccess)
    */     
   var clauseForJsonPath = lazyUnion(

      clauseMatcher(pathNodeSyntax   , list( capture, 
                                             duckTypeClause, 
                                             nameClause, 
                                             skip1 ))
                                                     
   ,  clauseMatcher(doubleDotSyntax  , list( skipMany))
       
       // dot is a separator only (like whitespace in other languages) but 
       // rather than make it a special case, use an empty list of 
       // expressions when this token is found
   ,  clauseMatcher(dotSyntax        , list() )  
                                                                                      
   ,  clauseMatcher(bangSyntax       , list( capture,
                                             rootExpr))
                                                          
   ,  clauseMatcher(emptySyntax      , list( statementExpr))
   
   ,  function (jsonPath) {
         throw Error('"' + jsonPath + '" could not be tokenised')      
      }
   );


   /**
    * One of two possible values for the onSuccess argument of 
    * generateClauseReaderIfTokenFound.
    * 
    * When this function is used, generateClauseReaderIfTokenFound simply 
    * returns the compiledParser that it made, regardless of if there is 
    * any remaining jsonPath to be compiled.
    */
   function returnFoundParser(_remainingJsonPath, compiledParser){ 
      return compiledParser 
   }     
              
   /**
    * Recursively compile a JSONPath expression.
    * 
    * This function serves as one of two possible values for the onSuccess 
    * argument of generateClauseReaderIfTokenFound, meaning continue to
    * recursively compile. Otherwise, returnFoundParser is given and
    * compilation terminates.
    */
   function compileJsonPathToFunction( uncompiledJsonPath, 
                                       parserGeneratedSoFar ) {

      /**
       * On finding a match, if there is remaining text to be compiled
       * we want to either continue parsing using a recursive call to 
       * compileJsonPathToFunction. Otherwise, we want to stop and return 
       * the parser that we have found so far.
       */
      var onFind =      uncompiledJsonPath
                     ?  compileJsonPathToFunction 
                     :  returnFoundParser;
                   
      return   clauseForJsonPath( 
                  uncompiledJsonPath, 
                  parserGeneratedSoFar, 
                  onFind
               );                              
   }

   /**
    * This is the function that we expose to the rest of the library.
    */
   return function(jsonPath){
        
      try {
         // Kick off the recursive parsing of the jsonPath 
         return compileJsonPathToFunction(jsonPath, always);
         
      } catch( e ) {
         throw Error( 'Could not compile "' + jsonPath + 
                      '" because ' + e.message
         );
      }
   }

});
