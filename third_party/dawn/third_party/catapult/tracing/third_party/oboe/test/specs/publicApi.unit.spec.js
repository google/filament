
/*
   Tests that calling the public api gets through correctly to the writing
   correctly. streamingXhr is a stub so no actual calls are made. 
 */

describe("public api", function(){
   "use strict";

   describe("propagates through to wiring function", function(){
  
      beforeEach(function() {
         spyOn(window, 'applyDefaults');      
         spyOn(window, 'wire');      
      });

      it('exports a usable function for GETs', function(){   
      
         oboe('http://example.com/oboez')
      
         expect(applyDefaults).toHaveBeenCalledLike(
            wire,
            'http://example.com/oboez'
         )      
      })
      
      it('can create a no-ajax instance', function(){   
      
         oboe()
      
         expect(wire).toHaveBeenCalledLike()      
      })      
         
      describe('GET', function(){
         
         it('works via arguments', function(){   
         
            oboe('http://example.com/oboez')
         
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez'
            )      
               
         })
                     
         it('works via options object', function(){   
              
            oboe({url: 'http://example.com/oboez'})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez'
            )   
         })
         
         it('can disable caching', function(){   
              
                                         
            oboe({url: 'http://example.com/oboez', cached:false})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               undefined, undefined, undefined, undefined,
               false
            )
         })
         
         it('can explicitly not disable caching', function(){   
              
            oboe({url: 'http://example.com/oboez', cached:true})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               undefined, undefined, undefined, undefined,
               true
            )
         })         
         
         it('propogates headers', function(){

            var headers = {'X-HEADER-1':'value1', 'X-HEADER-2':'value2'};
            
            oboe({url: 'http://example.com/oboez',
                  method:'GET', 
                  headers:headers})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'GET',
               undefined,
               headers
            )   
         })       
                   
      });
      
      describe('delete', function(){

         
         it('works via options object', function(){   
               
            oboe({url: 'http://example.com/oboez',
                  method: 'DELETE'})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'DELETE'
            )   
         })   
         
  
      });
        
            
      describe('post', function(){
         
         it('can post an object', function(){
               
            oboe({   method:'POST',
                     url:'http://example.com/oboez',
                     body:[1,2,3,4,5]
            })
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'POST',
               [1,2,3,4,5]
            )   
         })   
         
         it('can post a string', function(){
                        
            oboe({   method:'POST',
                     url:'http://example.com/oboez',
                     body:'my_data'
            })
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'POST',
               'my_data'
            )   
         })   
         
                     
      });
      
      describe('put', function(){   
         it('can put a string', function(){
               
            oboe({   method:'PUT',
                     url:'http://example.com/oboez', 
                     'body':'my_data'})
            
            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'PUT',
               'my_data'
            )   
         })
         

      });

      describe('patch', function(){
         it('can patch a string', function(){
            oboe({url:'http://example.com/oboez',
                  body:'my_data',
                  method:'PATCH'});

            expect(applyDefaults).toHaveBeenCalledLike(
               wire,
               'http://example.com/oboez',
               'PATCH',
               'my_data'
            )
         })
         
      })
      
   });
   
   this.beforeEach(function(){
   
      this.addMatchers(calledLikeMatcher);
   })   
      
});



