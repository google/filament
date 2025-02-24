import imageio
import numpy
import sys

im = imageio.imread(sys.argv[1])

if len(sys.argv) > 2:
    if sys.argv[2] == "f16":
        print "Reformatting to f16"
        im = im.astype("f2")

im.tofile(open(sys.argv[1] + ".raw","wb"))