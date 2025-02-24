# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(LLVM_SUBZERO_SRC_FILES
    "build/Android/include/llvm/Config/abi-breaking.h"
    "build/Android/include/llvm/Config/config.h"
    "build/Android/include/llvm/Config/llvm-config.h"
    "build/Android/include/llvm/Support/DataTypes.h"
    "build/Fuchsia/include/llvm/Config/abi-breaking.h"
    "build/Fuchsia/include/llvm/Config/config.h"
    "build/Fuchsia/include/llvm/Config/llvm-config.h"
    "build/Fuchsia/include/llvm/Support/DataTypes.h"
    "build/Linux/include/llvm/Config/abi-breaking.h"
    "build/Linux/include/llvm/Config/config.h"
    "build/Linux/include/llvm/Config/llvm-config.h"
    "build/Linux/include/llvm/Support/DataTypes.h"
    "build/MacOS/include/llvm/Config/abi-breaking.h"
    "build/MacOS/include/llvm/Config/config.h"
    "build/MacOS/include/llvm/Config/llvm-config.h"
    "build/MacOS/include/llvm/Support/DataTypes.h"
    "build/Windows/include/llvm/Config/abi-breaking.h"
    "build/Windows/include/llvm/Config/config.h"
    "build/Windows/include/llvm/Config/llvm-config.h"
    "build/Windows/include/llvm/Support/DataTypes.h"
    "include/llvm-c/ErrorHandling.h"
    "include/llvm-c/Support.h"
    "include/llvm-c/Types.h"
    "include/llvm/ADT/APFloat.h"
    "include/llvm/ADT/APInt.h"
    "include/llvm/ADT/ArrayRef.h"
    "include/llvm/ADT/BitVector.h"
    "include/llvm/ADT/DenseMap.h"
    "include/llvm/ADT/DenseMapInfo.h"
    "include/llvm/ADT/edit_distance.h"
    "include/llvm/ADT/EpochTracker.h"
    "include/llvm/ADT/Hashing.h"
    "include/llvm/ADT/ilist_base.h"
    "include/llvm/ADT/ilist_iterator.h"
    "include/llvm/ADT/ilist_node_base.h"
    "include/llvm/ADT/ilist_node_options.h"
    "include/llvm/ADT/ilist_node.h"
    "include/llvm/ADT/ilist.h"
    "include/llvm/ADT/IntrusiveRefCntPtr.h"
    "include/llvm/ADT/iterator_range.h"
    "include/llvm/ADT/iterator.h"
    "include/llvm/ADT/None.h"
    "include/llvm/ADT/Optional.h"
    "include/llvm/ADT/PointerIntPair.h"
    "include/llvm/ADT/PointerUnion.h"
    "include/llvm/ADT/simple_ilist.h"
    "include/llvm/ADT/SmallPtrSet.h"
    "include/llvm/ADT/SmallSet.h"
    "include/llvm/ADT/SmallString.h"
    "include/llvm/ADT/SmallVector.h"
    "include/llvm/ADT/Statistic.h"
    "include/llvm/ADT/STLExtras.h"
    "include/llvm/ADT/StringExtras.h"
    "include/llvm/ADT/StringMap.h"
    "include/llvm/ADT/StringRef.h"
    "include/llvm/ADT/StringSwitch.h"
    "include/llvm/ADT/Twine.h"
    "include/llvm/Demangle/Demangle.h"
    "include/llvm/IR/Argument.h"
    "include/llvm/IR/Attributes.h"
    "include/llvm/IR/BasicBlock.h"
    "include/llvm/IR/CallingConv.h"
    "include/llvm/IR/Constant.h"
    "include/llvm/IR/DebugLoc.h"
    "include/llvm/IR/DerivedTypes.h"
    "include/llvm/IR/Function.h"
    "include/llvm/IR/GlobalObject.h"
    "include/llvm/IR/GlobalValue.h"
    "include/llvm/IR/Instruction.h"
    "include/llvm/IR/Intrinsics.h"
    "include/llvm/IR/LLVMContext.h"
    "include/llvm/IR/Metadata.h"
    "include/llvm/IR/OperandTraits.h"
    "include/llvm/IR/SymbolTableListTraits.h"
    "include/llvm/IR/TrackingMDRef.h"
    "include/llvm/IR/Type.h"
    "include/llvm/IR/Use.h"
    "include/llvm/IR/User.h"
    "include/llvm/IR/Value.h"
    "include/llvm/IRReader/IRReader.h"
    "include/llvm/Support/AlignOf.h"
    "include/llvm/Support/Allocator.h"
    "include/llvm/Support/ARMBuildAttributes.h"
    "include/llvm/Support/Atomic.h"
    "include/llvm/Support/Casting.h"
    "include/llvm/Support/CBindingWrapping.h"
    "include/llvm/Support/Chrono.h"
    "include/llvm/Support/circular_raw_ostream.h"
    "include/llvm/Support/COFF.h"
    "include/llvm/Support/CommandLine.h"
    "include/llvm/Support/Compiler.h"
    "include/llvm/Support/DataStream.h"
    "include/llvm/Support/Debug.h"
    "include/llvm/Support/ELF.h"
    "include/llvm/Support/Endian.h"
    "include/llvm/Support/Errc.h"
    "include/llvm/Support/Errno.h"
    "include/llvm/Support/Error.h"
    "include/llvm/Support/ErrorHandling.h"
    "include/llvm/Support/ErrorOr.h"
    "include/llvm/Support/FileSystem.h"
    "include/llvm/Support/FileUtilities.h"
    "include/llvm/Support/Format.h"
    "include/llvm/Support/FormatCommon.h"
    "include/llvm/Support/FormatProviders.h"
    "include/llvm/Support/FormatVariadic.h"
    "include/llvm/Support/FormatVariadicDetails.h"
    "include/llvm/Support/MachO.h"
    "include/llvm/Support/ManagedStatic.h"
    "include/llvm/Support/MathExtras.h"
    "include/llvm/Support/MD5.h"
    "include/llvm/Support/Memory.h"
    "include/llvm/Support/MemoryBuffer.h"
    "include/llvm/Support/MemoryObject.h"
    "include/llvm/Support/Mutex.h"
    "include/llvm/Support/MutexGuard.h"
    "include/llvm/Support/NativeFormatting.h"
    "include/llvm/Support/Options.h"
    "include/llvm/Support/Path.h"
    "include/llvm/Support/PointerLikeTypeTraits.h"
    "include/llvm/Support/Process.h"
    "include/llvm/Support/Program.h"
    "include/llvm/Support/raw_os_ostream.h"
    "include/llvm/Support/raw_ostream.h"
    "include/llvm/Support/Regex.h"
    "include/llvm/Support/Signals.h"
    "include/llvm/Support/SMLoc.h"
    "include/llvm/Support/SourceMgr.h"
    "include/llvm/Support/StreamingMemoryObject.h"
    "include/llvm/Support/StringSaver.h"
    "include/llvm/Support/SwapByteOrder.h"
    "include/llvm/Support/thread.h"
    "include/llvm/Support/Threading.h"
    "include/llvm/Support/Timer.h"
    "include/llvm/Support/TimeValue.h"
    "include/llvm/Support/type_traits.h"
    "include/llvm/Support/UniqueLock.h"
    "include/llvm/Support/Valgrind.h"
    "include/llvm/Support/WindowsError.h"
    "include/llvm/Support/YAMLParser.h"
    "include/llvm/Support/YAMLTraits.h"
    "lib/Demangle/ItaniumDemangle.cpp"
    "lib/Support/APInt.cpp"
    "lib/Support/Atomic.cpp"
    "lib/Support/circular_raw_ostream.cpp"
    "lib/Support/CommandLine.cpp"
    "lib/Support/Debug.cpp"
    "lib/Support/Errno.cpp"
    "lib/Support/ErrorHandling.cpp"
    "lib/Support/Hashing.cpp"
    "lib/Support/ManagedStatic.cpp"
    "lib/Support/MemoryBuffer.cpp"
    "lib/Support/Mutex.cpp"
    "lib/Support/NativeFormatting.cpp"
    "lib/Support/Path.cpp"
    "lib/Support/Process.cpp"
    "lib/Support/Program.cpp"
    "lib/Support/raw_os_ostream.cpp"
    "lib/Support/raw_ostream.cpp"
    "lib/Support/regcclass.h"
    "lib/Support/regcname.h"
    "lib/Support/regcomp.c"
    "lib/Support/regerror.c"
    "lib/Support/regex_impl.h"
    "lib/Support/Regex.cpp"
    "lib/Support/regex2.h"
    "lib/Support/regexec.c"
    "lib/Support/regfree.c"
    "lib/Support/regstrlcpy.c"
    "lib/Support/regutils.h"
    "lib/Support/Signals.cpp"
    "lib/Support/SmallPtrSet.cpp"
    "lib/Support/SmallVector.cpp"
    "lib/Support/StringExtras.cpp"
    "lib/Support/StringMap.cpp"
    "lib/Support/StringRef.cpp"
    "lib/Support/StringSaver.cpp"
    "lib/Support/Threading.cpp"
    "lib/Support/Timer.cpp"
    "lib/Support/Twine.cpp"
    "lib/Support/Unix/Unix.h"
    "lib/Support/Windows/WindowsSupport.h"
)


if(WIN32)
    set(LLVM_SUBZERO_PLATFORM_INCLUDE_DIR "build/Windows/include")
elseif(LINUX)
    set(LLVM_SUBZERO_PLATFORM_INCLUDE_DIR "build/Linux/include")
elseif(APPLE)
    set(LLVM_SUBZERO_PLATFORM_INCLUDE_DIR "build/MacOS/include")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    list(APPEND LLVM_SUBZERO_COMPILE_OPTIONS
        "/wd4141"  # 'inline': used more than once
        "/wd4146"  # unary minus operator applied to unsigned type, result still unsigned
        "/wd4244"  # '=': conversion from '__int64' to 'llvm_regoff_t', possible loss of data
        "/wd4267"  # '=': conversion from 'size_t' to 'sopno', possible loss of data
        "/wd4291"  # 'void *operator new(size_t,const `anonymous-namespace'::NamedBufferAlloc &)': no matching operator delete found; memory will not be freed if initialization throws an exception
        "/wd4334"  # ''operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
        "/wd4996"  # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: new_name.
    )
else()  # Clang or GCC
    list(APPEND LLVM_SUBZERO_COMPILE_OPTIONS
         "-Wno-deprecated-declarations"
    )
endif()

add_library(llvm-subzero STATIC EXCLUDE_FROM_ALL
  ${LLVM_SUBZERO_SRC_FILES}
)

set_target_properties(llvm-subzero PROPERTIES
    POSITION_INDEPENDENT_CODE 1
)

target_include_directories(llvm-subzero
    PUBLIC
        "include"
        ${LLVM_SUBZERO_PLATFORM_INCLUDE_DIR}
)

target_compile_options(llvm-subzero
    PUBLIC
        ${LLVM_SUBZERO_COMPILE_OPTIONS}
)
