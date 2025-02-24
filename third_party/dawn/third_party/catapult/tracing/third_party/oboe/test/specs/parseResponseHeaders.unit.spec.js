
describe("parsing response headers", function() {

   // linefeed carriage return
   var lfcr = "\u000d\u000a";

   it('can parse an empty string', function(){
   
      var parsed = parseResponseHeaders('');
      
      expect(parsed).toEqual({})
   })
   
   it('can parse a single header', function(){
   
      var parsed = parseResponseHeaders('x-powered-by: Express');
      
      expect(parsed).toEqual({'x-powered-by':'Express'})
   });
   
   it('can parse a value containing ": "', function(){
   
      var parsed = parseResponseHeaders('x-title: Episode 2: Another episode');
      
      expect(parsed).toEqual({'x-title':'Episode 2: Another episode'})
   });

   it('can parse a value containing lots of colons ": "', function(){

      var parsed = parseResponseHeaders('x-title: Episode 2: Another episode: Remastered');

      expect(parsed).toEqual({'x-title':'Episode 2: Another episode: Remastered'})
   });

   it('can parse two values containing lots of colons ": "', function(){

      var parsed = parseResponseHeaders('x-title: Episode 2: Another episode: Remastered'  + lfcr + 
                                        'x-subtitle: This file has: a long title' );

      expect(parsed).toEqual({'x-title':'Episode 2: Another episode: Remastered',
                              'x-subtitle': 'This file has: a long title'})
   });   
   
   it('can parse several headers', function(){
      
      var subject =  "x-powered-by: Express" + lfcr + 
                     "Transfer-Encoding: Identity" + lfcr + 
                     "Connection: keep-alive";
                     
      var parsed = parseResponseHeaders(subject);
      
      expect(parsed).toEqual({
         "x-powered-by": "Express" 
      ,  "Transfer-Encoding": "Identity" 
      ,  "Connection": "keep-alive"      
      })
   })
   
});
