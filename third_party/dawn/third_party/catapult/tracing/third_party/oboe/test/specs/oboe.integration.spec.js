(function(Platform) {

  // Used to spy on global functions like setTimeout
  var globalContext;

  if ( !Platform.isNode ) {
    globalContext = window;
  } else {
    globalContext = GLOBAL;
  }


  describe("oboe integration (real http)", function() {

    var ASYNC_TEST_TIMEOUT = 15 * 1000; // 15 seconds
    jasmine.DEFAULT_TIMEOUT_INTERVAL = ASYNC_TEST_TIMEOUT;

    var oboe;
    var testUrl;

    if (Platform.isNode) {
      oboe = require('../../dist/oboe-node.js') ;
      testUrl = require('../libs/testUrl.js');
    } else {
      oboe = window.oboe;
      testUrl = window.testUrl;
    }

    describe('advertising the url on the Oboe instance', function() {

      var exampleUrl = testUrl('echoBackHeadersAsBodyJson');

      it('works after instatiation', function() {

        var instance = oboe({
          url: exampleUrl
        });

        expect(instance.source).toBe(exampleUrl);
      });

      it('works from callbacks', function(done) {

        var urlOnDone;

        oboe({
          url: exampleUrl
        }).done(function() {
          urlOnDone = this.source;
          expect(urlOnDone).toBe(exampleUrl);
          done();
        });
      });
    });

    Platform.isNode && describe('when running under node', function(){

      var http = require('http'),
          fs = require('fs'),
          request = require('request');

      it('can read from a stream that is passed in', function(done) {
        var callbackSpy = jasmine.createSpy('callbackSpy');

        http.request( 'http://localhost:4567/tenSlowNumbers' )
          .on('response', function( res ) {

            oboe(res)
              .node('![*]', callbackSpy)
              .done(function() {
                expect(callbackSpy.calls.count()).toBe(10);
                done();
              });
          }).end();
      });

      it('can read from a stream from Request - https://github.com/jimhigson/oboe.js/issues/65', function(done) {
        var callbackSpy = jasmine.createSpy('callbackSpy');

        oboe(request('http://localhost:4567/tenSlowNumbers'))
          .node('![*]', callbackSpy)
          .done(function() {
            expect(callbackSpy.calls.count()).toBe(10);
            done();
          });
      });

      it('can read from a local file', function(done) {
        var callbackSpy = jasmine.createSpy('callbackSpy');
        var fileStream = fs.createReadStream('test/json/firstTenNaturalNumbers.json');

        oboe(fileStream)
          .node('![*]', callbackSpy)
          .done(function() {
            expect(callbackSpy.calls.count()).toBe(10);
            done();
          });
      });

      it('can read an empty key', function(done) {
        var callbackSpy = jasmine.createSpy('callbackSpy');
        var fileStream = fs.createReadStream('test/json/emptyKey.json');

        oboe(fileStream)
          .node('!.*', callbackSpy)
          .done(function() {
            expect(callbackSpy.calls.count()).toBe(2);
            done();
          });
      });

      it('doesnt get confused if a stream has a "url" property', function(done) {
        var fileStream = fs.createReadStream('test/json/firstTenNaturalNumbers.json');
        fileStream.url = 'http://howodd.com';

        oboe(fileStream)
          .done(done);
      });

      // Tests that depend on network connections can be brittle. Skip for now.
      xit('can read from https', function(done) {
        var callbackSpy = jasmine.createSpy('callbackSpy');

        // rather than set up a https server in the tests
        // let's just use npm since this is an integration test
        // by confirming the Oboe.js homepage...

        oboe({
          url: 'https://registry.npmjs.org/oboe'
        })
          .node('!.homepage', callbackSpy)
          .done(function(obj) {
            var oboeHomepage = 'http://oboejs.com';
            expect(callbackSpy.calls.mostRecent().args[0]).toEqual(oboeHomepage);
            done();
          });
      });

      xit('complains if given a non-http(s) url', function(done) {
        expect(function() {
          oboe('ftp://ftp.mozilla.org/pub/mozilla.org/')
        }).toThrow();

        expect(function() {
          oboe('http://registry.npmjs.org/oboe')
        }).not.toThrow();
      });

    });

    (!Platform.isNode) && describe('running under a browser', function(){
      it('does not send cookies cross-domain by default', function(done) {
        document.cookie = "oboeIntegrationDontSend=123; path=/";

        oboe({
          url: crossDomainUrl('/echoBackHeadersAsBodyJson')
        }).done(function() {
          expect( this.root().cookie ).toBeUndefined();
          done();
        });
      });

      it('can send cookies cross-domain', function(done) {
        document.cookie = "oboeIntegrationSend=123; path=/";

        oboe({
          url: crossDomainUrl('/echoBackHeadersAsBodyJson'),
          withCredentials: true
        }).done(function(obj) {
          expect(this.root().cookie ).toMatch('oboeIntegrationSend=123');
          done();
        });
      });

      it('adds X-Requested-With to cross-domain by default on cross-domain', function(done) {

        oboe({
          url: testUrl('echoBackHeadersAsBodyJson')
        }).done(function(obj) {
          expect(this.root()['x-requested-with'] ).toEqual('XMLHttpRequest');
          done();
        });
      });

      it('does not add X-Requested-With to cross-domain by default on cross-domain', function(done) {

        oboe({
          url: crossDomainUrl('/echoBackHeadersAsBodyJson')
        }).done(function(obj) {
          expect(this.root()['x-requested-with'] ).toBeUndefined();
          done();
        });
      });
    });

    it('gets all expected callbacks by time request finishes', function(done) {
      var callbackSpy = jasmine.createSpy('callbackSpy');

      oboe(testUrl('tenSlowNumbers'))
        .node('![*]', callbackSpy)
        .done(function(fullResponse) {
          expect(callbackSpy).toHaveBeenCalledWith(0, [0], [fullResponse, 0]);
          expect(callbackSpy).toHaveBeenCalledWith(1, [1], [fullResponse, 1]);
          expect(callbackSpy).toHaveBeenCalledWith(2, [2], [fullResponse, 2]);
          expect(callbackSpy).toHaveBeenCalledWith(3, [3], [fullResponse, 3]);
          expect(callbackSpy).toHaveBeenCalledWith(4, [4], [fullResponse, 4]);
          expect(callbackSpy).toHaveBeenCalledWith(5, [5], [fullResponse, 5]);
          expect(callbackSpy).toHaveBeenCalledWith(6, [6], [fullResponse, 6]);
          expect(callbackSpy).toHaveBeenCalledWith(7, [7], [fullResponse, 7]);
          expect(callbackSpy).toHaveBeenCalledWith(8, [8], [fullResponse, 8]);
          expect(callbackSpy).toHaveBeenCalledWith(9, [9], [fullResponse, 9]);
          done();
        });
    });

    it('can make nested requests from arrays', function(done) {
      var fullResponse;
      var callbackSpy = jasmine.createSpy('callbackSpy');

      oboe(testUrl('tenSlowNumbers'))
        .node('![*]', function(outerNumber){

          oboe(testUrl('tenSlowNumbers'))
            .node('![*]', function(innerNumber){
              callbackSpy();
            });
        })
        .done(function(obj) {
          fullResponse = obj;
        });

      // makes a lot of requests so give it a while to complete
      waitsForAndRuns(function() {
        // console.log('callbackSpy.calls.count()', callbackSpy.calls.count())
        return callbackSpy.calls.count() == 100;
      }, done, 40 * 1000);
    });


    xit('continues to parse after a callback throws an exception', function(done) {

      var fullResponse;
      var callbackSpy = jasmine.createSpy('callbackSpy');

      spyOn(globalContext, 'setTimeout');
      oboe(testUrl('static/json/tenRecords.json'))
        .node('{id name}', function(){

          callbackSpy()
          throw new Error('uh oh!');
        })
        .done(function(obj) {
          fullResponse = obj;
        });

      waitsForAndRuns(function () {
        return callbackSpy.calls.count() == 10;
      }, function() {
        expect(globalContext.setTimeout.calls.count()).toEqual(10);
        for(var i = 0; i < 10; i++) {
          expect(function() {
            globalContext.setTimeout.calls.argsFor(i)[0]()
          }).toThrowError('uh oh!');
          expect(globalContext.setTimeout.calls.argsFor(i)[0]).toThrow('uh oh!')
        }
        done();
      }, ASYNC_TEST_TIMEOUT);
    });

    it('can abort once some data has been found in streamed response', function(done) {

      var aborted;
      var req = oboe(testUrl('tenSlowNumbers'))
            .node('![5]', function() {
              this.abort();
              aborted = true;
            });

      waitsForAndRuns(function() {
        return aborted;
      }, function() {
        expect(req.root()).toEqual([0, 1, 2, 3, 4, 5]);
        done();
      }, ASYNC_TEST_TIMEOUT);
    });

    it('can deregister from inside the callback', function(done) {
      var callbackSpy = jasmine.createSpy('callbackSpy');

      oboe(testUrl('tenSlowNumbers'))
        .node('![*]', function() {
          callbackSpy();
          if(callbackSpy.calls.count() === 5) {
            this.forget();
          }
        })
        .done(function(obj) {
          expect(callbackSpy.calls.count()).toBe(5);
          done();
        });
    });

    it('can still gets the whole resource after deregistering the callback', function(done) {
      var callbackSpy = jasmine.createSpy('callbackSpy');

      oboe(testUrl('tenSlowNumbers'))
        .node('![*]', function() {
          callbackSpy();
          if(callbackSpy.calls.count() === 5) {
            this.forget();
          }
        })
        .done(function(fullResponse) {
          expect(fullResponse).toEqual([0,1,2,3,4,5,6,7,8,9]);
          done();
        });
    });

    it('can send query params', function(done) {
      oboe(testUrl('echoBackQueryParamsAsBodyJson?apiKey=123'))
        .node('apiKey', function(keyArg) {
          expect(keyArg).toBe('123');
          done();
        });
    });

    it('can abort once some data has been found in not very streamed response', function(done) {

      // like above but we're getting a static file not the streamed numbers. This means
      // we'll almost certainly read in the whole response as one onprogress it is on localhost
      // and the json is very small

      var aborted;
      var req = oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
            .node('![5]', function() {
              this.abort();
              aborted = true;
            });

      waitsForAndRuns(function() {
        return aborted;
      }, function() {
        expect(req.root()).toEqual([0, 1, 2, 3, 4, 5]);
        done();
      }, ASYNC_TEST_TIMEOUT);
    });

    it('gives full json to callback when request finishes', function(done) {

      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .done(function(json) {
          expect(json).toEqual([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);
          done();
        });
    });

    it('gives content type as JSON when provided with an Object body', function(done) {

      oboe({
        method:'PUT',
        url:testUrl('echoBackHeadersAsBodyJson'),
        body:{'potatoes':3, 'cabbages':4}
      }).done(function(json){
        contentType = json['content-type'].split(';')[0];
        expect(contentType).toBe('application/json');
        done();
      });
    });

    it('gives allows content type to be overridden when provided with an Object body', function(done) {

      oboe({
        method:'PUT',
        url:testUrl('echoBackHeadersAsBodyJson'),
        body:{'potatoes':3, 'cabbages':4},
        headers:{'Content-Type':'application/vegetableDiffThing'}
      }).done(function(json){
        contentType = json['content-type'].split(';')[0];
        expect(contentType).toBe('application/vegetableDiffThing');
        done();
      });

    });

    it('gives header to server side', function(done) {

      var countGotBack = 0;

      oboe({
        url:testUrl('echoBackHeadersAsBodyJson'),
        headers:{'x-snarfu':'SNARF', 'x-foo':'BAR'}
      }).node('x-snarfu', function (headerValue) {
        expect(headerValue).toBe('SNARF');
        countGotBack++;
      }).node('x-foo', function (headerValue) {
        expect(headerValue).toBe('BAR');
        countGotBack++;
      }).done(function() {
        expect(countGotBack).toBe(2);
        done();
      });
    });

    it('can read response headers', function(done) {

      oboe({
        url:testUrl('echoBackHeadersAsHeaders'),
        headers:{'x-sso':'sso', 'x-sso2':'sso2'}
      }).done(function(){
        expect(this.header('x-sso')).toEqual('sso');
        expect(this.header('x-sso2')).toEqual('sso2');

        expect(this.header()['x-sso']).toEqual('sso');
        expect(this.header()['x-sso2']).toEqual('sso2');
        expect(this.header()['x-sso3']).toBeUndefined();

        done();
      });
    });

    it('gives undefined for headers before they are ready', function(done) {

      var res = oboe({
        url:testUrl('echoBackHeadersAsHeaders'),
        headers:{'x-sso':'sso', 'x-sso2':'sso2'}
      });

      expect(res.header()).toBeUndefined();
      expect(res.header('x-sso')).toBeUndefined();
      expect(res.header('x-sso2')).toBeUndefined();
      done();
    });

    it('notifies of response starting by giving status code and headers to callback', function(done) {
      var functionCalled;
      var eventCalled;

      oboe({
        url:testUrl('echoBackHeadersAsHeaders'),
        headers:{'x-a':'A', 'x-b':'B'}
      }).start(function(statusCode, headers){
        expect(statusCode).toBe(200);
        expect(headers['x-a']).toBe('A');
        functionCalled = true;
      }).on('start', function(statusCode, headers){
        expect(statusCode).toBe(200);
        expect(headers['x-b']).toBe('B');
        eventCalled = true;
      }).done(function() {
        expect(functionCalled).toBe(true);
        expect(eventCalled).toBe(true);
        done();
      });

    });

    it('can listen for nodes nodejs style', function(done) {

      var countGotBack = 0;
      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .on('node', '!.*', function () {
          countGotBack++;
        })
        .done(function() {
          expect(countGotBack).toBe(10);
          done();
        });
    });

    it('can listen for paths nodejs style', function(done) {

      var countGotBack = 0;
      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .on('path', '!.*', function (number) {
          countGotBack++;
        })
        .done(function() {
          expect(countGotBack).toBe(10);
          done();
        });
    });

    it('can listen for done nodejs style', function(done) {
      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .on('done', done);
    });

    it('gets all callbacks and they are in correct order', function (done) {
      var order = [];

      oboe({
        method:'POST',
        url: testUrl('echoBackBody'),
        body: {a:'A', b:'B', c:'C'}
      }).path('!', function() {
        order.push(1);
      }).path('a', function() {
        order.push(2);
      }).node('a', function() {
        order.push(3);
      }).path('b', function() {
        order.push(4);
      }).node('b', function() {
        order.push(5);
      }).path('c', function() {
        order.push(6);
      }).node('c', function() {
        order.push(7);
      }).done(function() {
        order.push(8);
        expect(order).toEqual([1,2,3,4,5,6,7,8]);
        done();
      });

    });

    it('emits error on 404', function(done) {

      var stubCallback = jasmine.createSpy('error callback');

      oboe(testUrl('404json'))
        .fail(function(err) {
          expect(err.statusCode).toBe(404);
          expect(err.jsonBody).toEqual({
            found: "false",
            errorMessage: "was not found"
          });
          done();
        });
    });

    xit('emits error on 404 in nodejs style too', function(done) {

      oboe(testUrl('doesNotExist'))
        .on('fail', function(err) {
          expect(err.statusCode).toBe(404);
          expect(err.jsonBody).toEqual({
            found: "false",
            errorMessage: "was not found"
          });
          done();
        });
    });

    /*
     This isn't reliable enough, too many false negatives for a ci
     it('emits error on unreachable url', function () {

     var stubCallback = jasmine.createSpy('error callback');

     oboe('nowhere.ox.ac.uk:754196/fooz/barz')
     .fail(stubCallback);

     waitsForAndRuns(function () {
     return !!stubCallback.calls.length;
     }, 'the request to fail', 30*1000)

     runs( function() {
     expect(stubCallback).toHaveBeenGivenAnyError();
     });
     })
     */

    it('throws node callback errors in a separate event loop', function(done) {

      var callbackError = new Error('I am a bad callback');

      spyOn(globalContext, 'setTimeout');
      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .node('!.*', function() {
          throw new Error('I am a bad callback');
        })
        .done(function() {
          expect(globalContext.setTimeout.calls.mostRecent().args[0]).toThrow(callbackError);
          done();
        });
    });

    xit('throws done callback errors in a separate event loop', function(done) {

      var callbackError = new Error('I am a bad callback');
      var errorCallback = function() {
        throw callbackError;
      };

      spyOn(globalContext, 'setTimeout');
      oboe(testUrl('static/json/firstTenNaturalNumbers.json'))
        .done(callbackError);

      waitsForAndRuns(function() {
        return globalContext.setTimeout.calls.count() > 0;
      }, function() {
        expect(globalContext.setTimeout.calls.mostRecent().args[0]).toThrowError('I am a bad callback');
        done();
      }, ASYNC_TEST_TIMEOUT);
    });

    it('can load an empty response with 204 status code', function(done) {

      oboe({
        url: testUrl('204noData')
      }).start(function(status){
        expect(status).toEqual(204);
      }).done(function(fullResponse) {
        expect(fullResponse).toEqual({});
        done();
      });

    });

    xit('emits error with incomplete json', function(done) {

      var stubCallback = jasmine.createSpy('error callback');

      oboe(testUrl('static/json/incomplete.json'))
        .fail(function(err) {
          console.log(err);
          expect(err.jsonBody).toEqual({
            found: "false",
            errorMessage: "some error"
          });
          done();
        });
    });

    if( !Platform.isNode ) {
      // only worry about this in the browser

      it( "hasn't put clarinet in the global namespace", function(){
        expect( window.clarinet ).toBeUndefined();
      });
    }

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

    beforeEach(function () {
      jasmine.addMatchers({
        toHaveBeenGivenAnyError: function(){
          return {
            compare: function(actual, expected) {
              var result = {};
              var errorReport = actual.calls.argsFor(0)[0],
                  maybeErr = errorReport.thrown;

              if( typeof maybeErr != 'object' ) {
                return {passed: false};
              }

              if( maybeErr instanceof Error ) {
                return {passed: true};
              }

              // instanceof Error doesn't always work in Safari (tested v6.0.5)
              // because:
              //
              // ((new XMLHttpRequestException()) instanceof Error) == false
              if( window && window.XMLHttpRequestException &&
                  maybeErr instanceof XMLHttpRequestException ) {
                    return {passed: true};
                  }

              // if that didn't work fallback to some duck typing:

              result.pass = (typeof maybeErr.message != 'undefined') &&
                (typeof maybeErr.lineNumber != 'undefined');
              return result;
            }
          };
        },
        toHaveBeenGivenThrowee:function(expectedError){
          return {
            compare: function(actual, expected) {
              var errorReport = actual.calls.getArgsFor(0)[0];
              return {
                passed: errorReport.thrown === expectedError
              };
            }
          };
        },
        toHaveBeenGivenErrorStatusCode:function(expectedCode){
          return {
            compare: function(actual, expected) {
              var errorReport = actual.calls.getArgsFor(0)[0];
              return {
                passed: errorReport.statusCode === expectedCode
              };
            }
          };
        },
        toHaveBeenGivenBodyJson:function(expectedBodyJson){
          return {
            compare: function(actual, expected) {
              var errorReport = actual.calls.getArgsFor(0)[0];
              return {
                passed: JSON.stringify(expectedBodyJson)
                  ===
                  JSON.stringify(errorReport.jsonBody)
              };
            }
          };
        }
      });
    });
  });
})(typeof Platform == 'undefined'? require('../libs/platform.js') : Platform)
