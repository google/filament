
describe("functional", function() {

   describe("varargs", function() {
   
      describe('with fixed arguments', function() {
            
            var received1, received2, receivedRest;
         
            function f(a1, a2, array){
               received1 = a1;
               received2 = a2;
               receivedRest = array;
            }
         
            var varargsSpy = varArgs(f);
            
            varargsSpy('a', 'b', 'c', 'd', 'e', 'f');
            
            it( 'passes through the first arguments as-is' , function() {
            
               expect(received1).toBe('a');
               expect(received2).toBe('b');            
            });
            
            it( 'passes through the rest as an array', function() {
            
               expect(receivedRest).toEqual(['c', 'd', 'e', 'f']);
            });
      });
         
      it("works with no fixed arguments", function() {
          
         var received1 = 'not yet set';
      
         function f(r1){
            received1 = r1;
         }
      
         var varargsSpy = varArgs(f);
         
         varargsSpy('a', 'b', 'c', 'd', 'e', 'f');
         
         expect(received1).toEqual(['a', 'b', 'c', 'd', 'e', 'f']);
      });
         
              
      it('propagates the return value', function() {
               
         var varargsTestFn = varArgs(function(){ return 'expected' });
                  
         expect( varargsTestFn() ).toBe('expected');                   
      });   
   });
   
   
   describe('compose', function() {

      function dub(n){ return n*2 }
      function inc(n){ return n+1 }
      function half(n){ return div(n,2) }
      function div(a, b){ return a/b }
   
      it('executes composed functions right-to-left', function(){
         
         var composed = compose(dub, inc, half);  // composed(x) = dub(inc(half(x)))
         
         expect(composed(2)).toBe( 4 ); // if this gives 2.5 the order is wrong      
      });
      
      it('may have an outermost function taking more than one argument', function() {
   
         var composed = compose(inc, div);  // composed(x, y) = x/y +1
   
         expect( composed( 10, 2 ) ).toBe(6); // 10/2 +1 = 5+1 = 6     
      });      
      
      it('can compose head to take an item off a list and then use it', function() {
         var list = cons( {a:1}, emptyList );
   
         expect( compose(attr('a'), head)( list ) ).toBe(1);      
      });
      
      it('can compose one function to give just that function', function() {
   
         expect( compose(div)( 20, 5 ) ).toBe(4);      
      });      
      
      it('gives an identity function when making a composition of zero functions', function() {
         var id = compose();
         
         expect( id(2) ).toBe(2);      
      });


      describe('compose2', function() {
         // optimised version of compose that takes exactly two arguments

         it('executes composed functions right-to-left', function(){
            
            var composed = compose2(dub, inc);  // composed(x) = dub(inc(x))
            
            expect(composed(2)).toBe( 6 ); // if this gives 5 the order is wrong      
         });
         
         it('may have an outermost function taking more than one argument', function() {
      
            var composed = compose2(inc, div);  // composed(x, y) = x/y +1
      
            expect( composed( 10, 2 ) ).toBe(6); // 10/2 +1 = 5+1 = 6     
         });      
         
         it('can compose head to take an item off a list and then use it', function() {
            var list = cons( {a:1}, emptyList );
      
            expect( compose2(attr('a'), head)( list ) ).toBe(1);      
         });
         
         it('passes scope to both functions', function() {
      
            var expectedScope = {};
      
            function f(){
               expect(this).toBe(expectedScope);
            }
      
            var composed = compose2(f, f);  // composed(x, y) = x/y +1
      
            composed.apply(expectedScope);     
         });         
                                    
      })
       
   });
   
   
   
   describe('attr', function() {
      it('can get a value from an object at the named key', function() {
         var getA = attr('A');
      
         expect( getA({A:'B'}) ).toBe( 'B' );
      });
      
      it('can get the length of a string', function() {
         var getLength = attr('length');
      
         expect( getLength("hello") ).toBe( 5 );
      });      
      
      it('can get a numbered array element out', function() {
         var getLength = attr(0);
      
         expect( getLength(['a','b','c']) ).toBe( 'a' );
      });            
   })
   
});