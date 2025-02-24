/* Copyright (c) 2015, Brandon Jones, Colin MacKenzie IV.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

describe("mat4", function() {
    var mat4 = require("../../src/gl-matrix/mat4.js");
    var vec3 = require("../../src/gl-matrix/vec3.js");

    var out, matA, matB, identity, result;

    beforeEach(function() {
        // Attempting to portray a semi-realistic transform matrix
        matA = [1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                1, 2, 3, 1];

        matB = [1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                4, 5, 6, 1];

        out =  [0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0];

        identity = [1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1];
    });

    describe("create", function() {
        beforeEach(function() { result = mat4.create(); });
        it("should return a 16 element array initialized to a 4x4 identity matrix", function() { expect(result).toBeEqualish(identity); });
    });

    describe("clone", function() {
        beforeEach(function() { result = mat4.clone(matA); });
        it("should return a 16 element array initialized to the values in matA", function() { expect(result).toBeEqualish(matA); });
    });

    describe("copy", function() {
        beforeEach(function() { result = mat4.copy(out, matA); });
        it("should place values into out", function() { expect(out).toBeEqualish(matA); });
        it("should return out", function() { expect(result).toBe(out); });
    });

    describe("identity", function() {
        beforeEach(function() { result = mat4.identity(out); });
        it("should place values into out", function() { expect(result).toBeEqualish(identity); });
        it("should return out", function() { expect(result).toBe(out); });
    });

    describe("transpose", function() {
        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.transpose(out, matA); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    1, 0, 0, 1,
                    0, 1, 0, 2,
                    0, 0, 1, 3,
                    0, 0, 0, 1
                ]); 
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.transpose(matA, matA); });
            
            it("should place values into matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 1,
                    0, 1, 0, 2,
                    0, 0, 1, 3,
                    0, 0, 0, 1
                ]); 
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("invert", function() {
        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.invert(out, matA); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -1, -2, -3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.invert(matA, matA); });
            
            it("should place values into matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -1, -2, -3, 1
                ]); 
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("adjoint", function() {
        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.adjoint(out, matA); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -1, -2, -3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.adjoint(matA, matA); });
            
            it("should place values into matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -1, -2, -3, 1
                ]); 
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("determinant", function() {
        beforeEach(function() { result = mat4.determinant(matA); });
        
        it("should return the determinant", function() { expect(result).toEqual(1); });
    });

    describe("multiply", function() {
        it("should have an alias called 'mul'", function() { expect(mat4.mul).toEqual(mat4.multiply); });

        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.multiply(out, matA, matB); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    5, 7, 9, 1
                ]); 
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
            it("should not modify matB", function() {
                expect(matB).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    4, 5, 6, 1
                ]);
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.multiply(matA, matA, matB); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    5, 7, 9, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
            it("should not modify matB", function() {
                expect(matB).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    4, 5, 6, 1
                ]);
            });
        });

        describe("when matB is the output matrix", function() {
            beforeEach(function() { result = mat4.multiply(matB, matA, matB); });
            
            it("should place values into matB", function() { 
                expect(matB).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    5, 7, 9, 1
                ]); 
            });
            it("should return matB", function() { expect(result).toBe(matB); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
        });
    });

    describe("translate", function() {
        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.translate(out, matA, [4, 5, 6]); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    5, 7, 9, 1
                ]); 
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.translate(matA, matA, [4, 5, 6]); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    5, 7, 9, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("scale", function() {
        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.scale(out, matA, [4, 5, 6]); });
            
            it("should place values into out", function() { 
                expect(out).toBeEqualish([
                    4, 0, 0, 0,
                    0, 5, 0, 0,
                    0, 0, 6, 0,
                    1, 2, 3, 1
                ]); 
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() { 
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]); 
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.scale(matA, matA, [4, 5, 6]); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    4, 0, 0, 0,
                    0, 5, 0, 0,
                    0, 0, 6, 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });
    
    describe("rotate", function() {
        var rad = Math.PI * 0.5;
        var axis = [1, 0, 0];

        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.rotate(out, matA, rad, axis); });
            
            it("should place values into out", function() {
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, Math.cos(rad), Math.sin(rad), 0,
                    0, -Math.sin(rad), Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.rotate(matA, matA, rad, axis); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, Math.cos(rad), Math.sin(rad), 0,
                    0, -Math.sin(rad), Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("rotateX", function() {
        var rad = Math.PI * 0.5;

        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.rotateX(out, matA, rad); });
            
            it("should place values into out", function() {
                expect(out).toBeEqualish([
                    1, 0, 0, 0,
                    0, Math.cos(rad), Math.sin(rad), 0,
                    0, -Math.sin(rad), Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.rotateX(matA, matA, rad); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, Math.cos(rad), Math.sin(rad), 0,
                    0, -Math.sin(rad), Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("rotateY", function() {
        var rad = Math.PI * 0.5;

        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.rotateY(out, matA, rad); });
            
            it("should place values into out", function() {
                expect(out).toBeEqualish([
                    Math.cos(rad), 0, -Math.sin(rad), 0,
                    0, 1, 0, 0,
                    Math.sin(rad), 0, Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.rotateY(matA, matA, rad); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    Math.cos(rad), 0, -Math.sin(rad), 0,
                    0, 1, 0, 0,
                    Math.sin(rad), 0, Math.cos(rad), 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    describe("rotateZ", function() {
        var rad = Math.PI * 0.5;

        describe("with a separate output matrix", function() {
            beforeEach(function() { result = mat4.rotateZ(out, matA, rad); });
            
            it("should place values into out", function() {
                expect(out).toBeEqualish([
                    Math.cos(rad), Math.sin(rad), 0, 0,
                    -Math.sin(rad), Math.cos(rad), 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return out", function() { expect(result).toBe(out); });
            it("should not modify matA", function() {
                expect(matA).toBeEqualish([
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
        });

        describe("when matA is the output matrix", function() {
            beforeEach(function() { result = mat4.rotateZ(matA, matA, rad); });
            
            it("should place values into matA", function() {
                expect(matA).toBeEqualish([
                    Math.cos(rad), Math.sin(rad), 0, 0,
                    -Math.sin(rad), Math.cos(rad), 0, 0,
                    0, 0, 1, 0,
                    1, 2, 3, 1
                ]);
            });
            it("should return matA", function() { expect(result).toBe(matA); });
        });
    });

    // TODO: fromRotationTranslation

    describe("frustum", function() {
        beforeEach(function() { result = mat4.frustum(out, -1, 1, -1, 1, -1, 1); });
        it("should place values into out", function() { expect(result).toBeEqualish([
                -1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 0, -1,
                0, 0, 1, 0
            ]); 
        });
        it("should return out", function() { expect(result).toBe(out); });
    });

    describe("perspective", function() {
        var fovy = Math.PI * 0.5;
        beforeEach(function() { result = mat4.perspective(out, fovy, 1, 0, 1); });
        it("should place values into out", function() { expect(result).toBeEqualish([
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, -1, -1,
                0, 0, 0, 0
            ]); 
        });
        it("should return out", function() { expect(result).toBe(out); });

        describe("with nonzero near, 45deg fovy, and realistic aspect ratio", function() {
            beforeEach(function() { result = mat4.perspective(out, 45 * Math.PI / 180.0, 640/480, 0.1, 200); });
            it("should calculate correct matrix", function() { expect(result).toBeEqualish([
                1.81066, 0, 0, 0,
                0, 2.414213, 0, 0,
                0, 0, -1.001, -1,
                0, 0, -0.2001, 0
            ]); });
        });
    });

    describe("ortho", function() {
        beforeEach(function() { result = mat4.ortho(out, -1, 1, -1, 1, -1, 1); });
        it("should place values into out", function() { expect(result).toBeEqualish([
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, -1, 0,
                0, 0, 0, 1
            ]); 
        });
        it("should return out", function() { expect(result).toBe(out); });
    });

    describe("lookAt", function() {
        var eye    = [0, 0, 1];
        var center = [0, 0, -1];
        var up     = [0, 1, 0];
        var view, up, right;

        describe("looking down", function() {
            beforeEach(function() {
                view = [0, -1,  0];
                up   = [0,  0, -1];
                right= [1,  0,  0];
                result = mat4.lookAt(out, [0, 0, 0], view, up);
            });

            it("should transform view into local -Z", function() {
                result = vec3.transformMat4([], view, out);
                expect(result).toBeEqualish([0, 0, -1]);
            });

            it("should transform up into local +Y", function() {
                result = vec3.transformMat4([], up, out);
                expect(result).toBeEqualish([0, 1, 0]);
            });

            it("should transform right into local +X", function() {
                result = vec3.transformMat4([], right, out);
                expect(result).toBeEqualish([1, 0, 0]);
            });

            it("should return out", function() { expect(result).toBe(out); });
        });

        describe("#74", function() {
            beforeEach(function() {
                mat4.lookAt(out, [0,2,0], [0,0.6,0], [0,0,-1]);
            });

            it("should transform a point 'above' into local +Y", function() {
                result = vec3.transformMat4([], [0, 2, -1], out);
                expect(result).toBeEqualish([0, 1, 0]);
            });

            it("should transform a point 'right of' into local +X", function() {
                result = vec3.transformMat4([], [1, 2, 0], out);
                expect(result).toBeEqualish([1, 0, 0]);
            });

            it("should transform a point 'in front of' into local -Z", function() {
                result = vec3.transformMat4([], [0, 1, 0], out);
                expect(result).toBeEqualish([0, 0, -1]);
            });
        });

        beforeEach(function() {
            eye    = [0, 0, 1];
            center = [0, 0, -1];
            up     = [0, 1, 0];
            result = mat4.lookAt(out, eye, center, up);
        });
        it("should place values into out", function() { expect(result).toBeEqualish([
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, -1, 1
            ]); 
        });
        it("should return out", function() { expect(result).toBe(out); });
    });

    describe("str", function() {
        beforeEach(function() { result = mat4.str(matA); });
        
        it("should return a string representation of the matrix", function() { expect(result).toEqual("mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 2, 3, 1)"); });
    });

   describe("frob", function() {
        beforeEach(function() { result = mat4.frob(matA); });
        it("should return the Frobenius Norm of the matrix", function() { expect(result).toEqual( Math.sqrt(Math.pow(1, 2) + Math.pow(1, 2) + Math.pow(1, 2) + Math.pow(1, 2) + Math.pow(1, 2) + Math.pow(2, 2) + Math.pow(3, 2) )); });
   });


});
