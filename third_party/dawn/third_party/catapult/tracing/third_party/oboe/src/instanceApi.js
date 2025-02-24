/**
 * The instance API is the thing that is returned when oboe() is called.
 * it allows:
 *
 *    - listeners for various events to be added and removed
 *    - the http response header/headers to be read
 */
function instanceApi(oboeBus, contentSource){

  var oboeApi,
      fullyQualifiedNamePattern = /^(node|path):./,
      rootNodeFinishedEvent = oboeBus(ROOT_NODE_FOUND),
      emitNodeDrop = oboeBus(NODE_DROP).emit,
      emitNodeSwap = oboeBus(NODE_SWAP).emit,

      /**
       * Add any kind of listener that the instance api exposes
       */
      addListener = varArgs(function( eventId, parameters ){

        if( oboeApi[eventId] ) {

          // for events added as .on(event, callback), if there is a
          // .event() equivalent with special behaviour , pass through
          // to that:
          apply(parameters, oboeApi[eventId]);
        } else {

          // we have a standard Node.js EventEmitter 2-argument call.
          // The first parameter is the listener.
          var event = oboeBus(eventId),
              listener = parameters[0];

          if( fullyQualifiedNamePattern.test(eventId) ) {

            // allow fully-qualified node/path listeners
            // to be added
            addForgettableCallback(event, listener);
          } else  {

            // the event has no special handling, pass through
            // directly onto the event bus:
            event.on( listener);
          }
        }

        return oboeApi; // chaining
      }),

      /**
       * Remove any kind of listener that the instance api exposes
       */
      removeListener = function( eventId, p2, p3 ){

        if( eventId == 'done' ) {

          rootNodeFinishedEvent.un(p2);

        } else if( eventId == 'node' || eventId == 'path' ) {

          // allow removal of node and path
          oboeBus.un(eventId + ':' + p2, p3);
        } else {

          // we have a standard Node.js EventEmitter 2-argument call.
          // The second parameter is the listener. This may be a call
          // to remove a fully-qualified node/path listener but requires
          // no special handling
          var listener = p2;

          oboeBus(eventId).un(listener);
        }

        return oboeApi; // chaining
      };

  /**
   * Add a callback, wrapped in a try/catch so as to not break the
   * execution of Oboe if an exception is thrown (fail events are
   * fired instead)
   *
   * The callback is used as the listener id so that it can later be
   * removed using .un(callback)
   */
  function addProtectedCallback(eventName, callback) {
    oboeBus(eventName).on(protectedCallback(callback), callback);
    return oboeApi; // chaining
  }

  /**
   * Add a callback where, if .forget() is called during the callback's
   * execution, the callback will be de-registered
   */
  function addForgettableCallback(event, callback, listenerId) {

    // listenerId is optional and if not given, the original
    // callback will be used
    listenerId = listenerId || callback;

    var safeCallback = protectedCallback(callback);

    event.on( function() {

      var discard = false;

      oboeApi.forget = function(){
        discard = true;
      };

      apply( arguments, safeCallback );

      delete oboeApi.forget;

      if( discard ) {
        event.un(listenerId);
      }
    }, listenerId);

    return oboeApi; // chaining
  }

  /**
   *  wrap a callback so that if it throws, Oboe.js doesn't crash but instead
   *  throw the error in another event loop
   */
  function protectedCallback( callback ) {
    return function() {
      try{
        return callback.apply(oboeApi, arguments);
      }catch(e)  {
        setTimeout(function() {
          throw new Error(e.message);
        });
      }
    }
  }

  /**
   * Return the fully qualified event for when a pattern matches
   * either a node or a path
   *
   * @param type {String} either 'node' or 'path'
   */
  function fullyQualifiedPatternMatchEvent(type, pattern) {
    return oboeBus(type + ':' + pattern);
  }

  function wrapCallbackToSwapNodeIfSomethingReturned( callback ) {
    return function() {
      var returnValueFromCallback = callback.apply(this, arguments);

      if( defined(returnValueFromCallback) ) {

        if( returnValueFromCallback == oboe.drop ) {
          emitNodeDrop();
        } else {
          emitNodeSwap(returnValueFromCallback);
        }
      }
    }
  }

  function addSingleNodeOrPathListener(eventId, pattern, callback) {

    var effectiveCallback;

    if( eventId == 'node' ) {
      effectiveCallback = wrapCallbackToSwapNodeIfSomethingReturned(callback);
    } else {
      effectiveCallback = callback;
    }

    addForgettableCallback(
      fullyQualifiedPatternMatchEvent(eventId, pattern),
      effectiveCallback,
      callback
    );
  }

  /**
   * Add several listeners at a time, from a map
   */
  function addMultipleNodeOrPathListeners(eventId, listenerMap) {

    for( var pattern in listenerMap ) {
      addSingleNodeOrPathListener(eventId, pattern, listenerMap[pattern]);
    }
  }

  /**
   * implementation behind .onPath() and .onNode()
   */
  function addNodeOrPathListenerApi( eventId, jsonPathOrListenerMap, callback ){

    if( isString(jsonPathOrListenerMap) ) {
      addSingleNodeOrPathListener(eventId, jsonPathOrListenerMap, callback);

    } else {
      addMultipleNodeOrPathListeners(eventId, jsonPathOrListenerMap);
    }

    return oboeApi; // chaining
  }


  // some interface methods are only filled in after we receive
  // values and are noops before that:
  oboeBus(ROOT_PATH_FOUND).on( function(rootNode) {
    oboeApi.root = functor(rootNode);
  });

  /**
   * When content starts make the headers readable through the
   * instance API
   */
  oboeBus(HTTP_START).on( function(_statusCode, headers) {

    oboeApi.header =  function(name) {
      return name ? headers[name]
        : headers
      ;
    }
  });

  /**
   * Construct and return the public API of the Oboe instance to be
   * returned to the calling application
   */
  return oboeApi = {
    on             : addListener,
    addListener    : addListener,
    removeListener : removeListener,
    emit           : oboeBus.emit,

    node           : partialComplete(addNodeOrPathListenerApi, 'node'),
    path           : partialComplete(addNodeOrPathListenerApi, 'path'),

    done           : partialComplete(addForgettableCallback, rootNodeFinishedEvent),
    start          : partialComplete(addProtectedCallback, HTTP_START ),

    // fail doesn't use protectedCallback because
    // could lead to non-terminating loops
    fail           : oboeBus(FAIL_EVENT).on,

    // public api calling abort fires the ABORTING event
    abort          : oboeBus(ABORTING).emit,

    // initially return nothing for header and root
    header         : noop,
    root           : noop,

    source         : contentSource
  };
}
