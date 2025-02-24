===============
DeferredShading
===============

This command-line example demonstrates how various different algorithms and sizes perform for Single Precision General Matrix Multiplication (SGEMM)

API
---
* OpenCL

Description
-----------	
This command-line example demonstrates how various different algorithms and sizes perform for Single Precision General Matrix Multiplication (SGEMM), which is matrix multiplication performed on matrices of arbitrarily large sizes

The example has implemented a few more or less well known algorithms, and tries various strategies for local memory use for caching.

The example is Command-line and its parameters can be displayed by running it with the -h parameter, i.e. "OpenCLMatrixMultiplication -h"

Running the example without parameters will result in a demo mode that compares a lot of different algorithms and workgroup sizes, and shows the results.

The most generic and generally best algorithm is performing the multiplication tile by tile and accumulating the results.