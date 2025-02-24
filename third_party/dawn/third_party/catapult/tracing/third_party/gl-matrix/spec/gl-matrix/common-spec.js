/*
* common.js unit test
*/

describe("glMatrix", function(){
  var result;

  var glMatrix = require("../../src/gl-matrix/common.js");

  describe("toRadian", function(){
    beforeEach(function(){ result = glMatrix.toRadian(180); });
    it("should return a value of 3.141592654(Math.PI)", function(){ expect(result).toBeEqualish(Math.PI); });
  });
});
