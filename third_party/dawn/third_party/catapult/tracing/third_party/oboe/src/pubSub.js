/**
 * pubSub is a curried interface for listening to and emitting
 * events.
 *
 * If we get a bus:
 *
 *    var bus = pubSub();
 *
 * We can listen to event 'foo' like:
 *
 *    bus('foo').on(myCallback)
 *
 * And emit event foo like:
 *
 *    bus('foo').emit()
 *
 * or, with a parameter:
 *
 *    bus('foo').emit('bar')
 *
 * All functions can be cached and don't need to be
 * bound. Ie:
 *
 *    var fooEmitter = bus('foo').emit
 *    fooEmitter('bar');  // emit an event
 *    fooEmitter('baz');  // emit another
 *
 * There's also an uncurried[1] shortcut for .emit and .on:
 *
 *    bus.on('foo', callback)
 *    bus.emit('foo', 'bar')
 *
 * [1]: http://zvon.org/other/haskell/Outputprelude/uncurry_f.html
 */
function pubSub(){

   var singles = {},
       newListener = newSingle('newListener'),
       removeListener = newSingle('removeListener');

   function newSingle(eventName) {
      return singles[eventName] = singleEventPubSub(
         eventName,
         newListener,
         removeListener
      );
   }

   /** pubSub instances are functions */
   function pubSubInstance( eventName ){

      return singles[eventName] || newSingle( eventName );
   }

   // add convenience EventEmitter-style uncurried form of 'emit' and 'on'
   ['emit', 'on', 'un'].forEach(function(methodName){

      pubSubInstance[methodName] = varArgs(function(eventName, parameters){
         apply( parameters, pubSubInstance( eventName )[methodName]);
      });
   });

   return pubSubInstance;
}
