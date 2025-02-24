var calledLikeMatcher = {
         /* Under Jasmine's toHaveBeenCalledLike, subject(foo, undefined)
            is considered different from subject(foo). This is slightly
            looser and considers those equal.  
          */
         toHaveBeenCalledLike:function(/*expectedArgs*/){
            var expectedArgs = Array.prototype.slice.apply(arguments);
            var actualCalls = this.actual.calls;
            
            if( !actualCalls || actualCalls.length == 0 ){
               this.message = function(){
                  return "Expected spy " + this.actual.identity + " to have been called like " + jasmine.pp(expectedArgs) + " but it has never been called."
               };
               return false;
            }
            
            var equals = this.env.equals_.bind(this.env);
            
            this.message = function() {
               var invertedMessage = "Expected spy " + this.actual.identity + " not to have been called like " + jasmine.pp(expectedArgs) + " but it was.";
               var positiveMessage = "";
               if (this.actual.callCount === 0) {
                  positiveMessage = "Expected spy " + this.actual.identity + " to have been called like " + jasmine.pp(expectedArgs) + " but it was never called.";
               } else {
                  positiveMessage = "Expected spy " + this.actual.identity + " to have been called like " + jasmine.pp(expectedArgs) + " but actual calls were " + jasmine.pp(this.actual.argsForCall).replace(/^\[ | \]$/g, '')
               }
               return [positiveMessage, invertedMessage];
            };            
                        
            return actualCalls.some(function( actualCall ){
            
               var actualArgs = actualCall.args;

               // check for one too many arguments given. But this is ok
               // if the extra arg is undefined.               
               if( actualArgs[expectedArgs.length] != undefined ) {

                  return false;
               }
               
               return expectedArgs.every(function( expectedArg, index ){
                  
                  return equals( actualArgs[index], expectedArg );                  
               });
            
            });
         }
      }
