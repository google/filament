set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Support
  )

# HLSL Change Begin
# Disable filesystem tests because the HLSL compiler file system abstractions
# don't work the same way LLVM's do.

set(HLSL_IGNORE_SOURCES
    FileOutputBufferTest.cpp
    LockFileManagerTest.cpp
    MemoryTest.cpp
    Path.cpp 
    raw_pwrite_stream_test.cpp
    # HLSL compiler doesn't support building targets...
    TargetRegistry.cpp
    )
# HLSL Change End

add_llvm_unittest(SupportTests
  AlignOfTest.cpp
  AllocatorTest.cpp
  ArrayRecyclerTest.cpp
  BlockFrequencyTest.cpp
  BranchProbabilityTest.cpp
  Casting.cpp
  CommandLineTest.cpp
  CompressionTest.cpp
  ConvertUTFTest.cpp
  DataExtractorTest.cpp
  DwarfTest.cpp
  EndianStreamTest.cpp
  EndianTest.cpp
  ErrorOrTest.cpp
  #FileOutputBufferTest.cpp # HLSL Change
  LEB128Test.cpp
  LineIteratorTest.cpp
  #LockFileManagerTest.cpp # HLSL Change
  MD5Test.cpp
  ManagedStatic.cpp
  MathExtrasTest.cpp
  MemoryBufferTest.cpp
  #MemoryTest.cpp # HLSL Change
  #Path.cpp # HLSL Change
  ProcessTest.cpp
  ProgramTest.cpp
  RegexTest.cpp
  ScaledNumberTest.cpp
  SourceMgrTest.cpp
  SpecialCaseListTest.cpp
  StreamingMemoryObject.cpp
  StringPool.cpp
  SwapByteOrderTest.cpp
  #TargetRegistry.cpp # HLSL Change
  ThreadLocalTest.cpp
  TimeValueTest.cpp
  UnicodeTest.cpp
  YAMLIOTest.cpp
  YAMLParserTest.cpp
  formatted_raw_ostream_test.cpp
  raw_ostream_test.cpp
  #raw_pwrite_stream_test.cpp # HLSL Change
  )

# ManagedStatic.cpp uses <pthread>.
if(LLVM_ENABLE_THREADS AND HAVE_LIBPTHREAD)
  target_link_libraries(SupportTests pthread)
endif()
