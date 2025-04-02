Scripts and makefiles for libpng
--------------------------------

    pnglibconf.h.prebuilt  =>  Configuration settings

    makefile.aix      =>  AIX/gcc makefile
    makefile.amiga    =>  Amiga makefile
    makefile.atari    =>  Atari makefile
    makefile.bc32     =>  Borland C makefile, for Win32
    makefile.beos     =>  BeOS makefile
    makefile.c89      =>  Generic UNIX makefile for C89 (cc -std=c89)
    makefile.clang    =>  Generic clang makefile
    makefile.darwin   =>  Darwin makefile, for macOS (formerly Mac OS X)
    makefile.dec      =>  DEC Alpha UNIX makefile
    makefile.dj2      =>  DJGPP 2 makefile
    makefile.emcc     =>  Emscripten makefile
    makefile.freebsd  =>  FreeBSD makefile
    makefile.gcc      =>  Generic gcc makefile
    makefile.hpgcc    =>  HPUX makefile using gcc
    makefile.hpux     =>  HPUX (10.20 and 11.00) makefile
    makefile.hp64     =>  HPUX (10.20 and 11.00) makefile, 64-bit
    makefile.ibmc     =>  IBM C/C++ version 3.x for Win32 and OS/2 (static lib)
    makefile.intel    =>  Intel C/C++ version 4.0 and later
    makefile.linux    =>  Linux/ELF makefile
                          (gcc, creates shared libpng16.so.16.1.6.*)
    makefile.mips     =>  MIPS makefile
    makefile.msys     =>  MSYS (MinGW) makefile
    makefile.netbsd   =>  NetBSD/cc makefile, makes shared libpng.so
    makefile.openbsd  =>  OpenBSD makefile
    makefile.riscos   =>  Acorn RISCOS makefile
    makefile.sco      =>  SCO OSr5 ELF and Unixware 7 with Native cc
    makefile.sgi      =>  Silicon Graphics IRIX makefile (cc, static lib)
    makefile.sggcc    =>  Silicon Graphics makefile
                          (gcc, creates shared libpng16.so.16.1.6.*)
    makefile.solaris  =>  Solaris 2.X makefile
                          (gcc, creates shared libpng16.so.16.1.6.*)
    makefile.so9      =>  Solaris 9 makefile
                          (gcc, creates shared libpng16.so.16.1.6.*)
    makefile.std      =>  Generic UNIX makefile (cc, static lib)
    makefile.sunos    =>  Sun makefile
    makefile.32sunu   =>  Sun Ultra 32-bit makefile
    makefile.64sunu   =>  Sun Ultra 64-bit makefile
    makefile.vcwin32  =>  makefile for Microsoft Visual C++ 4.0 and later
    makevms.com       =>  VMS build script
    smakefile.ppc     =>  AMIGA smakefile for SAS C V6.58/7.00 PPC compiler
                          (Requires SCOPTIONS, copied from SCOPTIONS.ppc)

Other supporting scripts
------------------------

    README.txt        =>  This file
    descrip.mms       =>  VMS makefile for MMS or MMK
    libpng-config-body.in  =>  used by several makefiles to create libpng-config
    libpng-config-head.in  =>  used by several makefiles to create libpng-config
    libpng.pc.in      =>  Used by several makefiles to create libpng.pc
    macro.lst         =>  Used by GNU Autotools
    pngwin.rc         =>  Used by the visualc71 project
    pngwin.def        =>  Used by makefile.os2
    pngwin.dfn        =>  Used to maintain pngwin.def
    SCOPTIONS.ppc     =>  Used with smakefile.ppc

    checksym.awk      =>  Used for maintaining pnglibconf.h
    dfn.awk           =>  Used for maintaining pnglibconf.h
    options.awk       =>  Used for maintaining pnglibconf.h
    pnglibconf.dfa    =>  Used for maintaining pnglibconf.h
    pnglibconf.mak    =>  Used for maintaining pnglibconf.h
    intprefix.c       =>  Used for symbol versioning
    prefix.c          =>  Used for symbol versioning
    sym.c             =>  Used for symbol versioning
    symbols.c         =>  Used for symbol versioning
    vers.c            =>  Used for symbol versioning

Further information can be found in comments in the individual scripts and
makefiles.
