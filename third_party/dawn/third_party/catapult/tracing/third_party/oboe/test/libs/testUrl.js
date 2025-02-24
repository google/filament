(function(Platform) {

   if( Platform.isNode ) {
      module.exports = function testUrl( path, https ){
         
         return 'http://localhost:4567/' + path;
      };
   } else {
      window.testUrl = function( path ){
         return '/testServer/' + path;
      };

      window.crossDomainUrl = function (path){
      
         /* This URL is considered cross-domain because the port is different.
          Normally Karma is providing a proxy to the service on :4567 at the
          same port as the test runner so that cross-domain isn't needed.
          Here we go directly to the test JSON service.
      
          This only works because the requested resource specifies
          the Access-Control-Allow-Origin header in an OPTIONS preflight
          request.  */   
                  
         var location = window.location;
         return location.protocol + '//' +
               location.hostname + ':4567' +
               path;
      };
   }

})(typeof Platform == 'undefined'? require('../libs/platform.js') : Platform)
