var HELPER_MATCHERS = (function() {
  var EPSILON = 0.00001;
  
  return {
    /*
      Returns true if `actual` has the same length as `expected`, and
      if each element of both arrays is within 0.000001 of each other.
      This is a way to check for "equal enough" conditions, as a way
      of working around floating point imprecision.
    */
    toBeEqualish: function(expected) {
      if (typeof(this.actual) == 'number')
        return Math.abs(this.actual - expected) < EPSILON;

      if (this.actual.length != expected.length) return false;
      for (var i = 0; i < this.actual.length; i++) {
        if (isNaN(this.actual[i]) !== isNaN(expected[i]))
          return false;
        if (Math.abs(this.actual[i] - expected[i]) >= EPSILON)
          return false;
      }
      return true;
    }
  };
})();

beforeEach(function() {
  this.addMatchers(HELPER_MATCHERS);
});

if (typeof(global) != 'undefined')
  global.HELPER_MATCHERS = HELPER_MATCHERS;
