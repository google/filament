// This file is the concatenation of many js files.
// See http://github.com/jimhigson/oboe.js for the raw source

// having a local undefined, window, Object etc allows slightly better minification:
(function  (window, Object, Array, Error, JSON, undefined ) {

   // ---contents--- //

   if ( typeof define === "function" && define.amd ) {
      define( "oboe", [], function () { return oboe; } );
   } else if (typeof exports === 'object') {
      module.exports = oboe;
   } else {
      window.oboe = oboe;
   }
})((function(){
   // Access to the window object throws an exception in HTML5 web workers so
   // point it to "self" if it runs in a web worker
      try {
         return window;
      } catch (e) {
         return self;
      }
   }()), Object, Array, Error, JSON);
