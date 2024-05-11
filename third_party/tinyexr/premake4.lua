newoption {
   trigger = "with-zfp",
   description = "Build with ZFP support."
}

sources = {
   "tinyexr.h",
   "test_tinyexr.cc",
   }

zfp_sources = {
   "./deps/ZFP/src/*.c"
}

-- premake4.lua
solution "TinyEXRSolution"
   configurations { "Release", "Debug" }

   if (os.is("windows")) then
      platforms { "x32", "x64" }
   else
      platforms { "native", "x32", "x64" }
   end

   if _OPTIONS["with-zfp"] then
      includedirs { "./deps/ZFP/inc" }
      defines { "TINYEXR_USE_ZFP=1" }
      files { zfp_sources }
   end

   -- A project defines one build target
   project "tinyexrtest"
      kind "ConsoleApp"
      language "C++"
      files { sources }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetname "test_tinyexr_debug"

      configuration "Release"
         -- defines { "NDEBUG" } -- -NDEBUG
         flags { "Symbols", "Optimize" }
         targetname "test_tinyexr"
