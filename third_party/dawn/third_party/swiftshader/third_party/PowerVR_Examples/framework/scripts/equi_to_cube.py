#!/usr/bin/env python
import sys
import imageio
from math import pi, sin, cos, tan, atan2, hypot, floor
import numpy
import time
from joblib import Parallel, delayed
import threading
import argparse

FACE_NAMES = {
  0: 'back',
  1: 'left',
  2: 'front',
  3: 'right',
  4: 'top',
  5: 'bottom'
}


FACE_REMAP_ORDER = { # 0->right, 1->left 2-> up 3-> down 4->forward 5 ->backward
  0: 5,
  1: 1,
  2: 4,
  3: 0,
  4: 2,
  5: 3
}

# get x,y,z coords from out image pixels coords
# i,j are pixel coords
# faceIdx is face number
# faceSize is edge length
def outImgToXYZ(i, j, faceIdx, faceSize):
    a = 2.0 * (float(j) + .5) / faceSize
    b = 2.0 * (float(i) + .5) / faceSize

    if faceIdx == 0: # back
        (x,y,z) = (-1.0, 1.0 - a, 1.0 - b)
    elif faceIdx == 1: # left
        (x,y,z) = (a - 1.0, -1.0, 1.0 - b)
    elif faceIdx == 2: # front
        (x,y,z) = (1.0, a - 1.0, 1.0 - b)
    elif faceIdx == 3: # right
        (x,y,z) = (1.0 - a, 1.0, 1.0 - b)
    elif faceIdx == 4: # top
        (x,y,z) = (b - 1.0, a - 1.0, 1.0)
    elif faceIdx == 5: # bottom
        (x,y,z) = (1.0 - b, a - 1.0, -1.0)

    return (x, y, z)

# convert using an inverse transformation
def convertFace(args, imgIn, faceIdx):
    global FACES_OUTPUT
    in_height = imgIn.shape[0]
    in_width = imgIn.shape[1]
    faceSize = in_width / 4
    
    imgOut = numpy.empty((faceSize,faceSize,3), imgIn.dtype)

    assert imgOut.shape[0] == imgOut.shape[1]

    for yOut in xrange(faceSize):
        if yOut % 10 == 0:
            print "FACE: ",FACE_NAMES[faceIdx],"\t ", yOut * 200 / in_height, "%"
        for xOut in xrange(faceSize):
            (x,y,z) = outImgToXYZ(xOut, yOut, faceIdx, faceSize)
            theta = atan2(y,x) # range -pi to pi
            r = hypot(x,y)
            phi = atan2(z,r) # range -pi/2 to pi/2

            # source img coords
            
            uf = 0.5 * in_width * (theta + pi) / pi
            vf = 0.5 * in_width * (pi/2 - phi) / pi

            # Use bilinear interpolation between the four surrounding pixels
            ui = int(floor(uf))  # coord of pixel to bottom left
            vi = int(floor(vf))
            u2 = ui+1       # coords of pixel to top right
            v2 = vi+1
            mu = uf-ui      # fraction of way across pixel
            nu = vf-vi
            
            vi = 0 if vi<0 else in_height-1 if vi > in_height-1 else vi
            v2 = 0 if v2<0 else in_height-1 if v2 > in_height-1 else v2
            
            
            # Pixel values of four corners
            A = imgIn[vi, 0 if ui == in_width else in_width - 1 if ui == -1 else ui]
            B = imgIn[vi, 0 if u2 == in_width else in_width - 1 if u2 == -1 else u2]
            C = imgIn[v2, 0 if ui == in_width else in_width - 1 if ui == -1 else ui]
            D = imgIn[v2, 0 if u2 == in_width else in_width - 1 if u2 == -1 else u2]

            # interpolate
            imgOut[xOut, yOut] = [
              A[0]*(1-mu)*(1-nu) + B[0]*(mu)*(1-nu) + C[0]*(1-mu)*nu+D[0]*mu*nu,
              A[1]*(1-mu)*(1-nu) + B[1]*(mu)*(1-nu) + C[1]*(1-mu)*nu+D[1]*mu*nu,
              A[2]*(1-mu)*(1-nu) + B[2]*(mu)*(1-nu) + C[2]*(1-mu)*nu+D[2]*mu*nu ]
    
    print "FACE: ",FACE_NAMES[faceIdx],"\t 100% - DONE"
    
    if not args.flatten_cubemap:
        components = args.output_file.rsplit('.', 1) if args.output_file else args.input_file.rsplit('.',1)
        filename = components[0] + "_" + str(faceSize) + "x" + str(faceSize) + "_" + FACE_NAMES[faceIdx] + "_" +args.format
        if args.raw:
            filename = filename + ".raw"
            imgOut.tofile(open(filename,"wb"))
        else:
            filename = filename + "." + components[1]
            imageio.imwrite(filename, imgOut)
    else:
            return faceIdx, imgOut.copy()
        
if __name__ == "__main__":
    imageio.plugins.freeimage.download()
    parser = argparse.ArgumentParser()
    description='Create a raw CubeMap array from an Equirectangular projection file. You can choose to perform a format conversion, and save the file either as separate images, or either separate raw files or a single data file for PVRTexTool import.'
    parser.add_argument("input_file", help="Input image. Must be an equirectangular projection (sphere projection map with a 2/1 side ratio)")
    parser.add_argument("-o", "--output_file", help="output file name basis. Metadata will still be appended to the filename (for example, face names). If not provided, the input file name and format is used accordingly (the input file is never overwritten). Will determine output format if provided.")
    parser.add_argument("-f", "--format", help="Set output format. Supported values: f16, f32. If you do not specify a format, input format is left unchanged", choices=['f16','f32'])
    parser.add_argument("-r", "--raw", help="Output raw data instead of attempting to store an image file", action="store_true")
    parser.add_argument("-c", "--flatten_cubemap", help="Create a single raw cubemap file for import into PVRTexTool instead of one file per side. Assumes and forces raw.", action="store_true")
    args = parser.parse_args()


    start = time.time()
    imgIn = imageio.imread(args.input_file)

    print "Input file dataType is: ", imgIn.dtype
    if args.format != None:
        if args.format == 'f16':
            format = 'f2'
        elif args.format == 'f32':
            if imgin.dtype.char == 'f2':
                print 'WARNING: Attempting to do widening conversion. Source image data is 16-bit float (f16) so there is no benefit to widen to 32-bit float.'
            format = 'f4'
        else:
            raise "Unknown format"
    else:
        format = imgIn.dtype.char
        if format == 'f2':
            args.format = 'f16'
        else:
            format = 'f4'
            args.format = 'f32'

    if imgIn.dtype.char != format:
        print "Converting to ", args.format, "(",format,")"
    imgIn = imgIn.astype(format)
            
    print (args.output_file.rsplit('.', 1) if args.output_file else args.input_file.rsplit('.',1))
    result = Parallel(n_jobs=8)(delayed(convertFace)(args,imgIn,i) for i in xrange(6))
    
    if args.flatten_cubemap:
        components = args.output_file.rsplit('.', 1) if args.output_file else args.input_file.rsplit('.',1)
        filename = components[0] + "_" + str(imgIn.shape[0]/2) + "_full-cubemap_" +args.format+".raw"
        result_arrays = result.sort(key=lambda x: FACE_REMAP_ORDER[x[0]])
        numpy.concatenate(tuple([x[1] for x in result])).tofile(open(filename,"wb"))
    
    print "Time elapsed: ",(time.time() - start)
    print "Saved processed image as: " + filename
    print '== PVRTexTool Wrap Raw Data settings =='
    print '= Width x Height : %d x %d' % (imgIn.shape[0]/2 , imgIn.shape[0]/2)
    print '= Variable type  : "Signed Floating Point"'
    print '= Colour space   : Linear RGB'
    print '= Faces          : 6'
    print '= MIP-Map levels : 1'
    print '= Array Members  : 1'
    print '= Pixel format - Uncompressed'
    print '= Pixel format - Channel names: R  G  B  [blank]'
    print '= Pixel format - Channel bits : %d %d %d [nothing]' % (16 if format == 'f2' else 32, 16 if format == 'f2' else 32, 16 if format == 'f2' else 32)
    
