/**
 * Write anything out as a string. For error messages when tests fail. 
 */
function anyToString(thing){
   
   if( thing === null ) {
      return 'null';
   }
   
   if( thing === undefined ) {
      return 'undefined';
   }

   if( typeof thing == 'string' ) {
      return '(string)' + thing;
   }
   
   if( thing.constructor == Array ) {
      return '[' + thing.map(anyToString).join(', ') + ']';
   }      

   if( typeof thing == 'function' ) {
      return thing.name? 'function ' + thing.name : 'anon function'; 
   }
   
   if( typeof thing == 'object' ) {
         
      return (
               thing.constructor == Object? 
                  '' 
               :  '(' + (thing.constructor.name) + ')'
             ) 
             + JSON.stringify(thing);
   }
   
   return JSON.stringify(thing);
}