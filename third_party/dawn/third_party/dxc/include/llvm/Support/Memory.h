//===- llvm/Support/Memory.h - Memory Support --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the llvm::sys::Memory class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MEMORY_H
#define LLVM_SUPPORT_MEMORY_H

#include "llvm/Support/DataTypes.h"
#include <string>
#include <system_error>

namespace llvm {
namespace sys {

  /// This class encapsulates the notion of a memory block which has an address
  /// and a size. It is used by the Memory class (a friend) as the result of
  /// various memory allocation operations.
  /// @see Memory
  /// @brief Memory block abstraction.
  class MemoryBlock {
  public:
    MemoryBlock() : Address(nullptr), Size(0) { }
    MemoryBlock(void *addr, size_t size) : Address(addr), Size(size) { }
    void *base() const { return Address; }
    size_t size() const { return Size; }
  private:
    void *Address;    ///< Address of first byte of memory area
    size_t Size;      ///< Size, in bytes of the memory area
    friend class Memory;
  };

  /// This class provides various memory handling functions that manipulate
  /// MemoryBlock instances.
  /// @since 1.4
  /// @brief An abstraction for memory operations.
  class Memory {
  public:
    enum ProtectionFlags {
      MF_READ  = 0x1000000,
      MF_WRITE = 0x2000000,
      MF_EXEC  = 0x4000000
    };

    /// This method allocates a block of memory that is suitable for loading
    /// dynamically generated code (e.g. JIT). An attempt to allocate
    /// \p NumBytes bytes of virtual memory is made.
    /// \p NearBlock may point to an existing allocation in which case
    /// an attempt is made to allocate more memory near the existing block.
    /// The actual allocated address is not guaranteed to be near the requested
    /// address.
    /// \p Flags is used to set the initial protection flags for the block
    /// of the memory.
    /// \p EC [out] returns an object describing any error that occurs.
    ///
    /// This method may allocate more than the number of bytes requested.  The
    /// actual number of bytes allocated is indicated in the returned
    /// MemoryBlock.
    ///
    /// The start of the allocated block must be aligned with the
    /// system allocation granularity (64K on Windows, page size on Linux).
    /// If the address following \p NearBlock is not so aligned, it will be
    /// rounded up to the next allocation granularity boundary.
    ///
    /// \r a non-null MemoryBlock if the function was successful, 
    /// otherwise a null MemoryBlock is with \p EC describing the error.
    ///
    /// @brief Allocate mapped memory.
    static MemoryBlock allocateMappedMemory(size_t NumBytes,
                                            const MemoryBlock *const NearBlock,
                                            unsigned Flags,
                                            std::error_code &EC);

    /// This method releases a block of memory that was allocated with the
    /// allocateMappedMemory method. It should not be used to release any
    /// memory block allocated any other way.
    /// \p Block describes the memory to be released.
    ///
    /// \r error_success if the function was successful, or an error_code
    /// describing the failure if an error occurred.
    /// 
    /// @brief Release mapped memory.
    static std::error_code releaseMappedMemory(MemoryBlock &Block);

    /// This method sets the protection flags for a block of memory to the
    /// state specified by /p Flags.  The behavior is not specified if the
    /// memory was not allocated using the allocateMappedMemory method.
    /// \p Block describes the memory block to be protected.
    /// \p Flags specifies the new protection state to be assigned to the block.
    /// \p ErrMsg [out] returns a string describing any error that occurred.
    ///
    /// If \p Flags is MF_WRITE, the actual behavior varies
    /// with the operating system (i.e. MF_READ | MF_WRITE on Windows) and the
    /// target architecture (i.e. MF_WRITE -> MF_READ | MF_WRITE on i386).
    ///
    /// \r error_success if the function was successful, or an error_code
    /// describing the failure if an error occurred.
    ///
    /// @brief Set memory protection state.
    static std::error_code protectMappedMemory(const MemoryBlock &Block,
                                               unsigned Flags);

    /// This method allocates a block of Read/Write/Execute memory that is
    /// suitable for executing dynamically generated code (e.g. JIT). An
    /// attempt to allocate \p NumBytes bytes of virtual memory is made.
    /// \p NearBlock may point to an existing allocation in which case
    /// an attempt is made to allocate more memory near the existing block.
    ///
    /// On success, this returns a non-null memory block, otherwise it returns
    /// a null memory block and fills in *ErrMsg.
    ///
    /// @brief Allocate Read/Write/Execute memory.
    static MemoryBlock AllocateRWX(size_t NumBytes,
                                   const MemoryBlock *NearBlock,
                                   std::string *ErrMsg = nullptr);

    /// This method releases a block of Read/Write/Execute memory that was
    /// allocated with the AllocateRWX method. It should not be used to
    /// release any memory block allocated any other way.
    ///
    /// On success, this returns false, otherwise it returns true and fills
    /// in *ErrMsg.
    /// @brief Release Read/Write/Execute memory.
    static bool ReleaseRWX(MemoryBlock &block, std::string *ErrMsg = nullptr);


    /// InvalidateInstructionCache - Before the JIT can run a block of code
    /// that has been emitted it must invalidate the instruction cache on some
    /// platforms.
    static void InvalidateInstructionCache(const void *Addr, size_t Len);

    /// setExecutable - Before the JIT can run a block of code, it has to be
    /// given read and executable privilege. Return true if it is already r-x
    /// or the system is able to change its previlege.
    static bool setExecutable(MemoryBlock &M, std::string *ErrMsg = nullptr);

    /// setWritable - When adding to a block of code, the JIT may need
    /// to mark a block of code as RW since the protections are on page
    /// boundaries, and the JIT internal allocations are not page aligned.
    static bool setWritable(MemoryBlock &M, std::string *ErrMsg = nullptr);

    /// setRangeExecutable - Mark the page containing a range of addresses
    /// as executable.
    static bool setRangeExecutable(const void *Addr, size_t Size);

    /// setRangeWritable - Mark the page containing a range of addresses
    /// as writable.
    static bool setRangeWritable(const void *Addr, size_t Size);
  };
}
}

#endif
