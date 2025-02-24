var oboe;
var sinon;
var globalContext;
var module = module ? module : undefined;

if(module && module.exports) {
  sinon = require('sinon');
  oboe = require('../../dist/oboe-node.js');
  globalContext = GLOBAL;
} else {
  sinon = window.sinon;
  oboe = window.oboe;
  globalContext = window;
}

/*
 Assertion helpers for testing the interface exposed as window.oboe

 These assertions mostly rely on everything that sits behind there as well (so they aren't
 true unit testing assertions, more of a suite of component testing helpers).

 */
function givenAnOboeInstance(jsonFileName) {

  function OboeAsserter() {

    var asserter = this,
        oboeInstance,

        expectingErrors = false,

        givenErrors = [],

        completeJson, // assigned in the requestCompleteCallback

        spiedCallback; //erk: only one callback stub per Asserter right now :-s

    function requestComplete(completeJsonFromJsonCompleteCall){
      completeJson = completeJsonFromJsonCompleteCall;

      asserter.isComplete = true;
    }

    oboeInstance = oboe( jsonFileName
                       ).done(requestComplete);

    oboeInstance.fail(function(e) {
      // Unless set up to expect them, the test isn't expecting errors.
      // Fail the test on getting an error:
      expect(expectingErrors).toBeTruthy();
    });

    // designed for use with jasmine's waitsFor, ie:
    //    waitsFor(asserter.toComplete())
    this.toComplete = function() {
      return function() {
        return asserter.isComplete;
      }
    }

    this.andWeAreListeningForNodes = function(pattern, callback, scope) {
      spiedCallback = callback ? sinon.spy(callback) : sinon.stub();

      oboeInstance.node(pattern, argumentClone(spiedCallback), scope);
      return this;
    };

    this.andWeAreListeningForPaths = function(pattern, callback, scope) {
      spiedCallback = callback ? sinon.spy(callback) : sinon.stub();

      oboeInstance.path(pattern, argumentClone(spiedCallback), scope);
      return this;
    };

    this.andWeHaveAFaultyCallbackListeningFor = function(pattern) {
      spiedCallback = sinon.stub().throws();

      oboeInstance.path(pattern, argumentClone(spiedCallback));
      return this;
    };

    this.andWeAreExpectingSomeErrors = function() {
      expectingErrors = true;

      spiedCallback = sinon.stub();

      oboeInstance.fail(argumentClone(spiedCallback));
      return this;
    };

    this.andWeAbortTheRequest = function() {
      oboeInstance.abort();
      return this;
    };

    this._whenGivenInput = function(input, close) {
      var jsonSerialisedInput;

      if (typeof input == 'string') {
        jsonSerialisedInput = input;
      } else {
        jsonSerialisedInput = JSON.stringify(input);
      }

      if( !jsonSerialisedInput || jsonSerialisedInput.length == 0 ) {
        throw new Error('Faulty test - input not valid:' + input);
      }

      oboeInstance.emit('data', jsonSerialisedInput );

      if( close ) {
        oboeInstance.emit('end');
      }

      return this;
    };

    this.whenGivenInput = function(input) {
      return this._whenGivenInput(input, true);
    };
    this.whenGivenInputPart = function(input) {
      return this._whenGivenInput(input, false);
    };

    this.whenGivenInputOneCharAtATime = function(input) {
      var jsonSerialisedInput;

      if (typeof input == 'string') {
        jsonSerialisedInput = input;
      } else {
        jsonSerialisedInput = JSON.stringify(input);
      }

      if( !jsonSerialisedInput || jsonSerialisedInput.length == 0 ) {
        throw new Error('Faulty test - input not valid:' + input);
      }

      for( var i = 0; i< jsonSerialisedInput.length; i++) {
        oboeInstance.emit('data', jsonSerialisedInput.charAt(i) );
      }

      return this;
    };

    this.whenInputFinishes = function() {

      oboeInstance.emit('end');

      return this;
    };

    /**
     * Assert any number of conditions were met on the spied callback
     */
    this.thenTheInstance = function( /* ... functions ... */ ){

      if( givenErrors.length > 0 ) {
        throw new Error('error found during previous stages\n' + givenErrors[0].stack);
      }

      for (var i = 0; i < arguments.length; i++) {
        var assertion = arguments[i];
        assertion.testAgainst(spiedCallback, oboeInstance, completeJson);
      }

      return this;
    };

    /** sinon stub is only really used to record arguments given.
     *  However, we want to preserve the arguments given at the time of calling, because they might subsequently
     *  be changed inside the parser so everything gets cloned before going to the stub
     */
    function argumentClone(delegateCallback) {
      return function(){

        function clone(original){
          // Note: window.eval being used here instead of JSON.parse because
          // eval can handle 'undefined' in the string but JSON.parse cannot.
          // This isn't wholy ideal since this means we're relying on JSON.
          // stringify to create invalid JSON. But at least there are no
          // security concerns with this being a test.
          return globalContext.eval( '(' + JSON.stringify( original ) + ')' );
        }
        function toArray(args) {
          return Array.prototype.slice.call(args);
        }

        var cloneArguments = toArray(arguments).map(clone);

        return delegateCallback.apply( this, cloneArguments );
      };
    }
  }
  return new OboeAsserter();
}


var wasPassedAnErrorObject = {
  testAgainst: function failIfNotPassedAnError(callback, oboeInstance) {

    if( !callback.args[0][0] instanceof Error ) {
      throw new Error("Callback should have been given an error but was given" + callback.constructor.name);
    }

  }
};


// higher-order function to create assertions. Pass output to Asserter#thenTheInstance.
// test how many matches were found
function foundNMatches(n){
  return {
    testAgainst:
    function(callback, oboeInstance) {
      if( n != callback.callCount ) {
        throw new Error('expected to have been called ' + n + ' times but has been called ' +
                        callback.callCount + ' times. \n' +
                        "all calls were with:" +
                        reportArgumentsToCallback(callback.args)
                       )
      }
    }
  }
}

// To test the json at oboe#json() is as expected.
function hasRootJson(expected){
  return {
    testAgainst:
    function(callback, oboeInstance) {

      expect(oboeInstance.root()).toEqual(expected);
    }
  }
}

// To test the json given as the call .onGet(url, callback(completeJson))
// is correct
function gaveFinalCallbackWithRootJson(expected) {
  return {
    testAgainst:
    function(callback, oboeInstance, completeJson) {
      expect(completeJson).toEqual(expected);
    }
  }
}

var foundOneMatch = foundNMatches(1),
    calledCallbackOnce = foundNMatches(1),
    foundNoMatches = foundNMatches(0);

function wasCalledbackWithContext(callbackScope) {
  return {
    testAgainst:
    function(callbackStub, oboeInstance) {
      if(!callbackStub.calledOn(callbackScope)){

        if( !callbackStub.called ) {
          throw new Error('Expected to be called with context ' + callbackScope + ' but has not been called at all');
        }

        throw new Error('was not called in the expected context. Expected ' + callbackScope + ' but got ' +
                        callbackStub.getCall(0).thisValue);
      }
    }
  };
}

function wasGivenTheOboeAsContext() {
  return {
    testAgainst:
    function(callbackStub, oboeInstance) {
      return wasCalledbackWithContext(oboeInstance).testAgainst(callbackStub, oboeInstance);
    }
  };
}

function lastOf(array){
  return array && array[array.length-1];
}
function penultimateOf(array){
  return array && array[array.length-2];
}
function prepenultimateOf(array){
  return array && array[array.length-3];
}

/**
 * Make a string version of the callback arguments given from oboe
 * @param {[[*]]} callbackArgs
 */
function reportArgumentsToCallback(callbackArgs) {

  return "\n" + callbackArgs.map( function( args, i ){

    var ancestors = args[2];

    return "Call number " + i + " was: \n" +
      "\tnode:         " + JSON.stringify( args[0] ) + "\n" +
      "\tpath:         " + JSON.stringify( args[1] ) + "\n" +
      "\tparent:       " + JSON.stringify( lastOf(ancestors) ) + "\n" +
      "\tgrandparent:  " + JSON.stringify( penultimateOf(ancestors) ) + "\n" +
      "\tancestors:    " + JSON.stringify( ancestors );

  }).join("\n\n");

}

// higher-level function to create assertions which will be used by the asserter.
function matched(obj) {

  return {
    testAgainst: function assertMatchedRightObject( callbackStub ) {

      if(!callbackStub.calledWith(obj)) {

        var objectPassedToCall = function(callArgs){return callArgs[0]};

        throw new Error( "was not called with the object " +  JSON.stringify(obj) + "\n" +
                         "objects that I got are:" +
                         JSON.stringify(callbackStub.args.map(objectPassedToCall) ) + "\n" +
                         "all calls were with:" +
                         reportArgumentsToCallback(callbackStub.args));

      }
    }

    ,  atPath: function assertAtRightPath(path) {
      var oldAssertion = this.testAgainst;

      this.testAgainst = function( callbackStub ){
        oldAssertion.apply(this, arguments);

        if(!callbackStub.calledWithMatch(sinon.match.any, path)) {
          throw new Error( "was not called with the path " +  JSON.stringify(path) + "\n" +
                           "paths that I have are:\n" +
                           callbackStub.args.map(function(callArgs){
                             return "\t" + JSON.stringify(callArgs[1]) + "\n";
                           }) + "\n" +
                           "all calls were with:" +
                           reportArgumentsToCallback(callbackStub.args));
        }
      };

      return this;
    }

    ,  withParent: function( expectedParent ) {
      var oldAssertion = this.testAgainst;

      this.testAgainst = function( callbackStub ){
        oldAssertion.apply(this, arguments);

        var parentMatcher = sinon.match(function (array) {

          var foundParent = penultimateOf(array);

          // since this is a matcher, we can't ues expect().toEqual()
          // because then the test would fail on the first non-match
          // under jasmine. Using stringify is slightly brittle and
          // if this breaks we need to work out how to plug into Jasmine's
          // inner equals(a,b) function

          return JSON.stringify(foundParent) == JSON.stringify(expectedParent)

        }, "had the right parent");

        if(!callbackStub.calledWithMatch(obj, sinon.match.any, parentMatcher)) {
          throw new Error( "was not called with the object" + JSON.stringify(obj) +
                           " and parent object " +  JSON.stringify(expectedParent) +
                           "all calls were with:" +
                           reportArgumentsToCallback(callbackStub.args));
        }
      };

      return this;
    }

    ,  withGrandparent: function( expectedGrandparent ) {
      var oldAssertion = this.testAgainst;

      this.testAgainst = function( callbackStub ){
        oldAssertion.apply(this, arguments);

        var grandparentMatcher = sinon.match(function (array) {

          // since this is a matcher, we can't ues expect().toEqual()
          // because then the test would fail on the first non-match
          // under jasmine. Using stringify is slightly brittle and
          // if this breaks we need to work out how to plug into Jasmine's
          // inner equals(a,b) function

          var foundGrandparent = prepenultimateOf(array);

          return JSON.stringify(foundGrandparent) == JSON.stringify(expectedGrandparent);

        }, "had the right grandparent");

        if(!callbackStub.calledWithMatch(obj, sinon.match.any, grandparentMatcher)) {
          throw new Error( "was not called with the object" + JSON.stringify(obj) +
                           " and garndparent object " +  JSON.stringify(expectedGrandparent) +
                           "all calls were with:" +
                           reportArgumentsToCallback(callbackStub.args));
        }
      };

      return this;
    }

    ,  atRootOfJson: function assertAtRootOfJson() {
      this.atPath([]);
      return this;
    }
  };
}

// Export if node
if (module && module.exports) {
  module.exports = {
    givenAnOboeInstance: givenAnOboeInstance,
    matched: matched,
    hasRootJson: hasRootJson,
    wasGivenTheOboeAsContext: wasGivenTheOboeAsContext,
    foundOneMatch: foundOneMatch,
    gaveFinalCallbackWithRootJson: gaveFinalCallbackWithRootJson,
    calledCallbackOnce: calledCallbackOnce,
    foundNoMatches: foundNoMatches,
    foundNMatches: foundNMatches,
    wasPassedAnErrorObject: wasPassedAnErrorObject
  };
}
