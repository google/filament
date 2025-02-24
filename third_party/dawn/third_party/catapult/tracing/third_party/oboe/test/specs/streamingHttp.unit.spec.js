describe('streaming http in a browser', function() {

   function fakeBusWithEventsForStreamingHttp() {
      return fakePubSub([HTTP_START, STREAM_DATA, STREAM_END, FAIL_EVENT, ABORTING]);
   }

   function fakeXhr2() {
      var xhr = new sinon.FakeXMLHttpRequest();
      //xhr.onprogress = function(){};
      xhr.chunkSize = 2;
      return xhr;
   }

   it('raises an event for first bit of content',function() {

      var fakeBus = fakeBusWithEventsForStreamingHttp();
      var fakeXhr = fakeXhr2();
      var responseBody = '[1,2,3,4,5,6,7,8,9,10]';

      streamingHttp(fakeBus, fakeXhr, 'GET', 'http://example.com', null, {}, false);
      
      debugger;
      fakeXhr.respond(200, {}, responseBody);

      //fakeXhr.setResponseBody('here is a response body');

      console.log(JSON.stringify(fakeBus.events));
   });

});
