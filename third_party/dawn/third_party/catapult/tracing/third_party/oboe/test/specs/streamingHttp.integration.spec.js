/* Tests the streaming xhr without stubbing anything. Really just a test that
*  we've got the interface of the in-browser XHR object pinned down  */


describe('streaming xhr integration (real http)', function() {
   "use strict";

   var emittedEvents = [HTTP_START, STREAM_DATA, STREAM_END, FAIL_EVENT, ABORTING];

   it('completes',  function(done) {

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/static/json/smallestPossible.json',
         null // this is a GET, no data to send
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, done);
   })

   it('can ajax in a small known file',  function(done) {

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/static/json/smallestPossible.json',
         null // this is a GET, no data to send
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
       expect(streamedContentPassedTo(oboeBus)).toParseTo({})
       done();
     });
   })

   it('fires HTTP_START with status and headers',  function(done) {

      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/echoBackHeadersAsHeaders',
         null, // this is a GET, no data to send
         {'specialheader':'specialValue'}
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       expect(oboeBus(HTTP_START).emit)
         .toHaveBeenCalledWith(
           200,
           jasmine.objectContaining({
             'specialheader': 'specialValue'
           })
         );
       done();
     });
   })

   it('gives XHR header so server knows this is an xhr request',  function(done) {

      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/echoBackHeadersAsHeaders'
      );


     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       expect(oboeBus(HTTP_START).emit)
          .toHaveBeenCalledWith(
            200,
            jasmine.objectContaining({
              'x-requested-with': 'XMLHttpRequest'
            })
          );
       done();
     });

   })

   it('fires HTTP_START, STREAM_DATA and STREAM_END in correct order',  function(done) {

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/echoBackHeadersAsHeaders',
         null, // this is a GET, no data to send
         {'specialheader':'specialValue'}
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
       done();
     });
   })

   it('fires FAIL_EVENT if url does not exist', function(done) {

      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/noSuchUrl',
         null
      );

      waitUntil(FAIL_EVENT).isFiredOn(oboeBus, done);

   })

   it('can ajax in a very large file without missing any',  function(done) {

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/static/json/twentyThousandRecords.json',
         null // this is a GET, no data to send
      );

      waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
        var runFunction = function() {
          return JSON.parse(streamedContentPassedTo(oboeBus));
        };
        expect(runFunction).not.toThrow();
        var parsedResult = runFunction();

        // as per the name, should have 20,000 records in that file:
        expect(parsedResult.result.length).toEqual(20000);
        done()
      });
   })

   it('can ajax in a streaming file without missing any',  function(done) {


      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/tenSlowNumbers?withoutMissingAny',
          null // this is a GET, no data to send
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       // as per the name, should have ten numbers in that file:
       expect(streamedContentPassedTo(oboeBus)).toParseTo([0,1,2,3,4,5,6,7,8,9]);
       expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
       done();
     });
   })

   it('sends cookies by default',  function(done) {

      document.cookie = "token=123456; path=/";

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/echoBackHeadersAsBodyJson',
         null
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       var parsedResult = JSON.parse(streamedContentPassedTo(oboeBus));
       expect(parsedResult.cookie).toMatch('token=123456');
       done();
     });

   })

   it('does not send cookies by default to cross-domain requests',  function(done) {

      document.cookie = "deniedToken=123456; path=/";

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         crossDomainUrl('/echoBackHeadersAsBodyJson'),
         null
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       var parsedResult = JSON.parse(streamedContentPassedTo(oboeBus));
       expect(parsedResult.cookie).not.toMatch('deniedToken=123456');
       done();
     });
   })

   it('sends cookies to cross-domain requests if withCredentials is true',  function(done) {

      document.cookie = "corsToken=123456; path=/";

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         crossDomainUrl('/echoBackHeadersAsBodyJson'),
         null, // data
         null, // headers
         true  // withCredentials
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
       var parsedResult = JSON.parse(streamedContentPassedTo(oboeBus));
       expect(parsedResult.cookie).toMatch('corsToken=123456');
       done();
     });
   })

   it('can make a post request',  function(done) {

      var payload = {'thisWill':'bePosted','andShould':'be','echoed':'back'};

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'POST',
         '/testServer/echoBackBody',
         JSON.stringify(payload)
      );

      waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
        expect(streamedContentPassedTo(oboeBus)).toParseTo(payload);
        expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
        done();
      });

   })

   it('can make a put request',  function(done) {

      var payload = {'thisWill':'bePut','andShould':'be','echoed':'back'};

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'PUT',
         '/testServer/echoBackBody',
         JSON.stringify(payload)
      );

      waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
        expect(streamedContentPassedTo(oboeBus)).toParseTo(payload);
        expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
        done();
      });

   })


   it('can make a patch request',  function(done) {

      if( Platform.isInternetExplorer ) {
         console.warn('PATCH requests don\'t work well under IE. Skipping PATCH integration test');
         return;
      }

      var payload = {'thisWill':'bePatched','andShould':'be','echoed':'back'};

      // in practice, since we're running on an internal network and this is a small file,
      // we'll probably only get one callback
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'PATCH',
         '/testServer/echoBackBody',
         JSON.stringify(payload)
      );

      waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
        if( streamedContentPassedTo(oboeBus) == '' &&
          (Platform.isPhantom) ) {
          console.warn( 'this user agent seems not to support giving content'
                       + ' back for of PATCH requests.'
                       + ' This happens on PhantomJS');
        } else {
           expect(streamedContentPassedTo(oboeBus)).toParseTo(payload);
           expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder();
        }
        done();
      });

   })


   // this test is only activated for non-IE browsers and IE 10 or newer.
   // old and rubbish browsers buffer the xhr response meaning that this
   // will never pass. But for good browsers it is good to have an integration
   // test to confirm that we're getting it right.
   if( !Platform.isInternetExplorer || Platform.isInternetExplorer >= 10 ) {
      it('gives multiple callbacks when loading a streaming resource',  function(done) {

         var oboeBus = fakePubSub(emittedEvents)
         streamingHttp(
            oboeBus,
            httpTransport(),
            'GET',

            '/testServer/tenSlowNumbers',
             null // this is a get: no data to send
         );

         waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
           // realistically, should have had 10 or 20, but this isn't deterministic so
           // 3 is enough to indicate the results didn't all arrive in one big blob.
           expect(oboeBus.callCount[STREAM_DATA]).toBeGreaterThan(3)
           expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder()
           done();
         });
      })

      // TODO, this test should work
      xit('gives multiple callbacks when loading a gzipped streaming resource',  function(done) {

         var oboeBus = fakePubSub(emittedEvents)
         streamingHttp(
            oboeBus,
            httpTransport(),
            'GET',

            '/testServer/gzippedTwoHundredItems',
             null // this is a get: no data to send
         );

         waitUntil(STREAM_END).isFiredOn(oboeBus, function() {
           // some platforms can't help but not work here so warn but don't
           // fail the test:
           if( oboeBus.callCount[STREAM_DATA] == 1 &&
                 (Platform.isInternetExplorer || Platform.isPhantom) ) {
              console.warn('This user agent seems to give gzipped responses' +
                  'as a single event, not progressively. This happens on ' +
                  'PhantomJS and IE < 9');
           } else {
              expect(oboeBus.callCount[STREAM_DATA]).toBeGreaterThan(1);
           }

           expect(oboeBus).toHaveGivenStreamEventsInCorrectOrder();
           done();
         });
      })
   }

   // TODO test should pass
   xit('does not call back with zero-length bites',  function(done) {

      // since this is a large file, even serving locally we're going to get multiple callbacks:
      var oboeBus = fakePubSub(emittedEvents)
      streamingHttp(
         oboeBus,
         httpTransport(),
         'GET',
         '/testServer/static/json/oneHundredRecords.json',
         null // this is a GET: no data to send
      );

     waitUntil(STREAM_END).isFiredOn(oboeBus, function () {

       var dripsReceived = oboeBus.eventTypesEmitted[STREAM_DATA].map(function( args ){
          return args[0];
       });

       expect(dripsReceived.length).toBeGreaterThan(0);

       dripsReceived.forEach(function(drip) {
         expect(drip.length).not.toEqual(0);
       });
    });
 })

  // This is the equivalent of the old waitsFor/runs syntax
  // which was removed from Jasmine 2
  function waitsForAndRuns(escapeFunction, runFunction, escapeTime) {
    // check the escapeFunction every millisecond so as soon as it is met we can escape the function
    var interval = setInterval(function() {
      if (escapeFunction()) {
        clearMe();
        runFunction();
      }
    }, 1);

    // in case we never reach the escapeFunction, we will time out
    // at the escapeTime
    var timeOut = setTimeout(function() {
      clearMe();
      runFunction();
    }, escapeTime);

    // clear the interval and the timeout
    function clearMe(){
      clearInterval(interval);
      clearTimeout(timeOut);
    }
  };

   function waitUntil(event) {
     return {
       isFiredOn: function (eventBus, cb){
         waitsForAndRuns(function() {
           return !!eventBus(event).emit.calls.count();
         }, cb, ASYNC_TEST_TIMEOUT);
      }
     };
   }

   function streamedContentPassedTo(eventBus){

      return eventBus.eventTypesEmitted[STREAM_DATA].map(function(args){
         return args[0];
      }).join('');
   }

   beforeEach(function(){

      jasmine.addMatchers({
         toHaveGivenStreamEventsInCorrectOrder: function(){
           return {
             compare: function(actual, expected) {
               var result = {};
               var eventNames = actual.eventNames;

               result.pass = eventNames[0] === HTTP_START
                 && eventNames[1] === STREAM_DATA
                 && eventNames[eventNames.length-1] === STREAM_END;

               if(!result.pass) {
                 result.message = 'events not in correct order. We have: ' +
                     JSON.stringify(
                       eventNames.map(prettyPrintEvent)
                     ) + ' but should follow "start", "data"*, "end"';

               }

               return result;
             }

           };



         },

         toParseTo: function(){
           return {
             compare: function(actual, expected) {
               var result = {};
               var normalisedActual;
               if( !actual ) {
                 result.pass = false;
                 result.message = 'no content has been received';
                 return result;
               }

               try {
                   normalisedActual = JSON.stringify( JSON.parse(actual) );
               } catch (e) {
                 result.pass = false;
                 result.message = "Expected to be able to parse the found " +
                   "content as json '" + actual + "' but it " +
                   "could not be parsed";
                 return result;
               }

               result.pass = normalisedActual === JSON.stringify(expected);
               if (!result.pass) {
                 result.message = "The found json parsed but did not match " + JSON.stringify(expected) +
                   " because found " + this.actual;

               }
               return result;
             }
           };
         }
      });



   });


});
