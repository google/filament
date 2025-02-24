
describe("streamingHttp", function(){
   "use strict";

   describe("calls through to browser xhr", function(){
      
      it('gives xhr null when body is null', function(){
         var eventBus = pubSub(), xhr = xhrStub();
      
         streamingHttp(eventBus, xhr, 'GET', 'http://example.com', null);
         
         expect( xhr.send ).toHaveBeenCalledWith(null);
      })      
      
      it('give xhr string request body', function(){
         var eventBus = pubSub(), xhr = xhrStub();      
      
         streamingHttp(eventBus, xhr, 'GET', 'http://example.com', 'my_data');
         
         expect( xhr.send ).toHaveBeenCalledWith('my_data');
      })

      it('gives xhr the request headers', function(){
         var eventBus = pubSub(), xhr = xhrStub();           
         var headers = {
            'X-FROODINESS':'frood',
            'X-HOOPINESS':'hoopy'
         };           
           
         streamingHttp(eventBus, xhr, 'GET', 'http://example.com', undefined, headers);
         
         expect( xhr.setRequestHeader ).toHaveBeenCalledWith( 'X-FROODINESS', 'frood' );      
         expect( xhr.setRequestHeader ).toHaveBeenCalledWith( 'X-HOOPINESS', 'hoopy' );      
      });            
      
      it('should be able to abort an xhr once started', function(){
         var eventBus = pubSub(), xhr = xhrStub();
               
         streamingHttp(eventBus, xhr, 'GET', 'http://example.com', 'my_data');
         
         eventBus(ABORTING).emit();
                  
         expect( xhr.abort ).toHaveBeenCalled();                  
      });
      
      
      function xhrStub() {
         return jasmine.createSpyObj('xhr', ['abort', 'open', 'setRequestHeader', 'send']);
      }
   });   
               
});


