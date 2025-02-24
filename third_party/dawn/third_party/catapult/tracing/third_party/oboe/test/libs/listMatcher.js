function listMatcher(expectedList){

   return eq.call( this, this.actual, expectedList );

   function eq( actual, expected ) {

      if( !actual && !expected ) {
         return true;
      }

      if( !actual || !expected ) {
         this.message = function(){
            return 'one false but not both ' +
             actual + ' ' + expected;
         };
         return false;
      }

      if( head(actual) != head(expected) ) {
         this.message = function(){
            return 'different items in list' +
             head(actual) + head(expected);
         };
         return false;
      }

      return eq.call( this, tail(actual), tail(expected) );
   }
}
