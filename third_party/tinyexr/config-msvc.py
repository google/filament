exe = "test_tinyexr.exe"

# "gnu" or "msvc" are provided as predefined toolchain
toolchain = "msvc"

# optional
link_pool_depth = 1

# optional
builddir = {
    "gnu" :  "build"
  , "msvc" :  "build"
  , "clang" :  "build"
    }

# required
includes = {
    "gnu" : [ "-I." ]
  , "msvc" : [ "/I." ]
  , "clang" : [ "-I." ]
    }

# required
defines = {
    "gnu" : [ "-DEXAMPLE=1" ]
  , "msvc" : [ "/DEXAMPLE=1" ]
  , "clang" : [ "-DEXAMPLE=1" ]
    }

# required
cflags = {
    "gnu" : [ "-O2", "-g" ]
  , "msvc" : [ "/O2" ]
  , "clang" : [ "-O2", "-g" ]
    }

# required
cxxflags = {
    "gnu" : [ "-O2", "-g" ]
  , "msvc" : [ "/O2", "/W4" ]
  , "clang" : [ "-O2", "-g", "-fsanitize=address" ]
    }

# required
ldflags = {
    "gnu" : [ ]
  , "msvc" : [ ]
  , "clang" : [ "-fsanitize=address" ]
    }

# optionsl
cxx_files = [ "test_tinyexr.cc" ]
c_files = [ ]

# You can register your own toolchain through register_toolchain function
def register_toolchain(ninja):
    pass

    #ninja.rule('clangcxx', description='CXX $out',
    #    command='$clangcxx -MMD -MF $out.d $clangdefines $clangincludes $clangcxxflags -c $in -o $out',
    #    depfile='$out.d', deps='gcc')
    #ninja.rule('clangcc', description='CC $out',
    #    command='$clangcc -MMD -MF $out.d $clangdefines $clangincludes $clangcflags -c $in -o $out',
    #    depfile='$out.d', deps='gcc')
    #ninja.rule('clanglink', description='LINK $out', pool='link_pool',
    #    command='$clangld $clangldflags -o $out $in $libs')
    #ninja.rule('clangar', description='AR $out', pool='link_pool',
    #    command='$clangar rsc $out $in')
    #ninja.rule('clangstamp', description='STAMP $out', command='touch $out')
    #ninja.newline()

    #ninja.variable('clangcxx', 'clang++')
    #ninja.variable('clangcc', 'clang')
    #ninja.variable('clangld', 'clang++')
    #ninja.variable('clangar', 'ar')
    #ninja.newline()


