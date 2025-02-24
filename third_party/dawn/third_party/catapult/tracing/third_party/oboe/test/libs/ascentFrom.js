/** pass me an object with only one child at each level and will return a nodeStack
 *  and pathStack which describes the only possibly descent top-to-bottom
 *  
 *  @param {Object} description eg { a:       {foo:      {a:'target'}}}
 */
function ascentFrom( description ) {

   function onlyMapping(obj) {
      // this for won't loop but it is the most obvious way to extract a 
      // key/value pair where the key is unknown
      for( var i in obj ) {
         return {key:i, node:obj[i]};
      } 
   }   
   
   var ascent = list({key:ROOT_PATH, node:description}),
       curDesc = description;
         
   while( typeof curDesc == 'object' ) {
   
      var mapping = onlyMapping(curDesc);
       
      curDesc = mapping.node;
      
      ascent = cons( mapping, ascent ); 
   }
         
   return ascent;            
}