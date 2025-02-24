
describe("url handling", function() {

   // page location needs to be of the form given by window.location:
   //
   // {
   //    protocol:'http:'        (nb: includes the colon, not just name)
   //    host:'google.co.uk',
   //    port:'80'               (or '')
   // }
   //
   // so, cases are:
   //    protocol:
   //          given in ajax, the same as page
   //          given in ajax, different from page
   //          not given in ajax url
   //    host:
   //          given in ajax, the same as page
   //          given in ajax, different from page
   //          not given in ajax url
   //    port: given in both and same
   //          different
   //          both not given
   //          given as 80 in ajax url but not page, and page is http
   //          given as non-80 in ajax url but not page, and page is http
   //          given as 433 in ajax url but not page, and page is https
   //          given as non-433 in ajax url but not page, and page is https
   //          given in page but not ajax url

   describe('can parse URLs', function() {

      var noInformationRegardingOrigin = {
         protocol: '',
         host: '',
         port: ''
      };

      it( 'parses absolute path only', function() {
         expect('/foo/barz').toParseTo(noInformationRegardingOrigin);
      });

   //    it( 'parses absolute path with extension', function() {
   //        expect('/foo/bar.jpg').toParseTo(noInformationRegardingOrigin);
   //    });

   //    it( 'parses absolute path with extension and query', function() {
   //        expect('/foo/bar.jpg?foo=bar&woo=doo').toParseTo(noInformationRegardingOrigin);
   //    });

   //    it( 'parses relative path only', function() {
   //        expect('foo/bar').toParseTo(noInformationRegardingOrigin);
   //    });

   //    it( 'parses relative path with extension', function() {
   //        expect('foo/bar.jpg').toParseTo(noInformationRegardingOrigin);
   //    });

   //    it( 'parses a url with domain but no protocol', function() {

   //        expect('//example.com/foo/bar.jpg').toParseTo({
   //           protocol:'',
   //           host:'example.com',
   //           port:''
   //        });
   //    });

   //    it( 'parses a url with one-word domain', function() {

   //       expect('//database/foo/bar.jpg').toParseTo({
   //          protocol:'',
   //          host:'database',
   //          port:''
   //       });
   //    });

   //    it( 'parses a url with one-word domain and port', function() {

   //       expect('//search:9200/foo/bar').toParseTo({
   //          protocol:'',
   //          host:'search',
   //          port:'9200'
   //       });
   //    });


   //    it( 'parses a url with domain with a hyphen', function() {

   //       expect('//example-site.org/foo/bar.jpg').toParseTo({
   //          protocol:'',
   //          host:'example-site.org',
   //          port:''
   //       });
   //    });

   //    it( 'parses a url with domain with a number', function() {

   //       expect('//123.org.uk/foo/bar.jpg').toParseTo({
   //          protocol:'',
   //          host:'123.org.uk',
   //          port:''
   //       });
   //    });

   //    it( 'parses a url with a protocol', function() {

   //       expect('http://example.com/foo').toParseTo({
   //          protocol:'http:',
   //          host:'example.com',
   //          port:''
   //       });
   //    });

   //    it( 'parses a url with a protocol and a port', function() {

   //       expect('http://elasticsearch:9200/tweets').toParseTo({
   //          protocol:'http:',
   //          host:'elasticsearch',
   //          port:'9200'
   //       });
   //    });

   //    it( 'parses a url with a protocol and a port implicitly at the root', function() {

   //       expect('http://elasticsearch:9200').toParseTo({
   //          protocol:'http:',
   //          host:'elasticsearch',
   //          port:'9200'
   //       });
   //    });

   });


   var testCases = {
      'http://www.current-site.co.uk':{
         '/foo/bar': false,
         'foo/bar': false,
         'http://www.current-site.co.uk/index.html': false,
         'http://localhost:9876/foo': true,
         '//localhost:9876/foo': true,
         'http://otherhost:9876/foo': true,
         'http://localhost:8081/foo': true,
         'https://localhost:9876/foo': true,
         'ftp://localhost:9876/foo': true,
         '//localhost:8080/foo': true,
         'http://www.current-site.co.uk:8080': true,
         'https://www.current-site.co.uk': true
      }

   ,  'http://www.current-site.co.uk:8080/some/page.html': {
         '/foo/bar': false,
         'foo/bar': false,
         'http://www.current-site.co.uk/index.html': true,
         '//www.current-site.co.uk:8080/index.html': false,
         'http://localhost:9876/foo': true,
         '//localhost:9876/foo': true,
         'http://otherhost:9876/foo': true,
         'http://localhost:8081/foo': true,
         'https://localhost:9876/foo': true,
         'ftp://localhost:9876/foo': true,
         '//localhost:8080/foo': true,
         'http://www.current-site.co.uk:8080': false,
         'https://www.current-site.co.uk:8080': true,
         'https://www.current-site.co.uk': true
      }

   ,  'http://www.current-site.co.uk:80/foo': {
         'http://www.current-site.co.uk/foo': false,
         'http://www.current-site.co.uk:80/foo': false,
         '//www.current-site.co.uk/foo': false,
         '//www.current-site.co.uk:80/foo': false
      }

   ,  'http://www.current-site.co.uk/foo': {
         'http://www.current-site.co.uk:80/foo': false,
         'http://www.current-site.co.uk/foo': false,
         '//www.current-site.co.uk:80/foo': false,
         '//www.current-site.co.uk/foo': false
      }

   ,  'https://www.current-site.co.uk:443/foo': {
         'https://www.current-site.co.uk/foo': false,
         'https://www.current-site.co.uk:443/foo': false,
         '//www.current-site.co.uk/foo': false,
         '//www.current-site.co.uk:443/foo': false
      }

   ,  'https://www.current-site.co.uk/foo': {
         'https://www.current-site.co.uk:443/foo': false,
         'https://www.current-site.co.uk/foo': false,
         '//www.current-site.co.uk:443/foo': false,
         '//www.current-site.co.uk/foo': false
      }
   };

   describe('detection of x-origin-ness', function() {

      for( var currentPage in testCases ) {

         describe('testing from page ' + currentPage, function() {

            var expectedResults = testCases[currentPage];

            for (var ajaxUrl in expectedResults) {

               var expectToBeCrossOrigin = expectedResults[ajaxUrl],
                   crossOriginDesc = (expectToBeCrossOrigin ? 'cross-origin' : 'same-origin');

               it('should detect ' + ajaxUrl + ' as ' + crossOriginDesc,
                  function (currentPage, ajaxUrl, expectToBeCrossOrigin) {

                     if( expectToBeCrossOrigin ) {
                        expect(ajaxUrl).toBeCrossOriginOnPage(currentPage);
                     } else {
                        expect(ajaxUrl).not.toBeCrossOriginOnPage(currentPage);
                     }

                  }.bind(this, currentPage, ajaxUrl, expectToBeCrossOrigin)
               );
            }
         });
      }
   });


   beforeEach(function() {
      jasmine.addMatchers({
         toParseTo: function() {
           return {
             compare: function(actual, expected) {
               var result = {};
               var actualUrlParsed = parseUrlOrigin(actual);
               result.pass = (actualUrlParsed.protocol == expected.protocol) &&
                 (actualUrlParsed.host == expected.host) &&
                 (actualUrlParsed.port == expected.port);

               if (!result.pass) {
                 result.message = 'expected ' + actual
                   + ' to parse to ' + JSON.stringify(expected)
                   + ' but got ' + JSON.stringify(actualUrlParsed);
               }

               return result;
             }
           };
         },

        toBeCrossOriginOnPage: function() {
          return {
            compare: function(actual, expected) {
              var ajaxUrl = actual;
              var ajaxHost = parseUrlOrigin(ajaxUrl);
              var curPageHost = parseUrlOrigin(expected);

              return {
                pass: isCrossOrigin(curPageHost, ajaxHost)
              };

            }
          };
        }
      });
   });
});
