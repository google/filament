PngMinus
--------
(copyright Willem van Schaik, 1999-2019)



Some history
------------
Soon after the creation of PNG in 1995, the need was felt for a set of
pnmtopng / pngtopnm utilities. Independently Alexander Lehmann and I
(Willem van Schaik) started such a project. Luckily we discovered this
and merged the two, which later became part of NetPBM, available from
SourceForge.

These two utilities have many, many options and make use of most of the
features of PNG, like gamma, alpha, sbit, text-chunks, etc. This makes
the utilities quite complex and by now not anymore very maintainable.
When we wrote these programs, libpng was still in an early stage.
Therefore, lots of the functionality that we put in our software can now
be done using transform-functions in libpng.

Finally, to compile these programs, you need to have installed and
compiled three libraries: libpng, zlib and netpbm. Especially the latter
makes the whole setup a bit bulky. But that's unavoidable given the many
features of pnmtopng.


What now (1999)
---------------
At this moment libpng is in a very stable state and can do much of the
work done in pnmtopng. Also, pnmtopng needs to be upgraded to the new
interface of libpng. Hence, it is time for a rewrite from the ground up
of pnmtopng and pngtopnm. This will happen in the near future (stay
tuned). The new package will get a different name to distinguish it from
the old one: PngPlus.

To experiment a bit with the new interface of libpng, I started off with
a small prototype that contains only the basic functionality. It doesn't
have any of the options to read or write special chunks and it will do
no gamma correction. But this makes it also a simple program that is
quite easy to understand and can serve well as a template for other
software developments. By now there are of course a couple of programs,
like Greg Roelofs' rpng/wpng, that can be used just as good.


Can and can not
---------------
As this is the small brother of the future PngPlus, I called this fellow
PngMinus. Because I started this development in good-old Turbo-C, I
avoided the use the netpbm library, which requires DOS extenders. Again,
another reason to call it PngMinus (minus netpbm :-). So, part of the
program are some elementary routines to read / write pgm- and ppm-files.
It does not handle B&W pbm-files, but instead you could do pgm with bit-
depth 1.

The downside of this approach is that you can not use them on images
that require blocks of memory bigger than 64k (the DOS version). For
larger images you will get an out-of-memory error.

As said before, PngMinus doesn't correct for gamma. When reading
png-files you can do this just as well by piping the output of png2pnm
to pnmgamma, one of the standard PbmPlus tools. This same scenario will
most probably also be followed in the full-blown future PngPlus, with
the addition of course of the possibility to create gamma-chunks when
writing png-files.

On the other hand it supports alpha-channels. When reading a png-image
you can write the alpha-channel into a pgm-file. And when creating an
RGB+A png-image, you just combine a ppm-file with a corresponding
pgm-file containing the alpha-channel. When reading, transparency chunks
are converted into an alpha-channel and from there on treated the same
way.

Finally you can opt for writing ascii or binary pgm- and ppm-files. When
the bit-depth is 16, the format will always be ascii.


Using it
--------
To distinguish them from pnmtopng and PngPlus, the utilities are named
png2pnm and pnm2png (2 instead of to). The input- and output-files can
be given as parameters or through redirection. Therefore the programs
can be part of a pipe.

To list the options type "png2pnm -h" or "pnm2png -h".


Just like Scandinavian furniture
--------------------------------
You have to put it together yourself. I developed the software on MS-DOS
with Turbo-C 3.0 and RedHat Linux 4.2 with gcc. In both cases I used
libpng-1.0.4 and zlib-1.1.3. By now (2019) it is twenty years later and
more current versions are OK.

The makefile assumes that the libpng libraries can be found in ../.. and
libz in ../../../zlib. But you can change this to for example ../libpng
and ../zlib. The makefile creates two versions of each program, one with
static library support and the other using shared libraries.

If you create a ../pngsuite directory and then store the basn####.png
files from PngSuite (http://www.schaik.com/pngsuite/) in there, you can
test the proper functioning of PngMinus by running pngminus.sh.


Warranty
-------
Please, remember that this was just a small experiment to learn a few
things. It will have many unforeseen features <vbg> ... who said bugs? Use
it when you are in need for something simple or when you want a starting
point for developing your own stuff.


The end
-------
Willem van Schaik
mailto:willem at schaik dot com
http://www.schaik.com/png/

Oct 1999, Jan 2019

