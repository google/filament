/*===------- llvm/Config/llvm-config.h - llvm configuration -------*- C -*-===*/
/*                                                                            */
/* Part of the LLVM Project, under the Apache License v2.0 with LLVM          */
/* Exceptions.                                                                */
/* See https://llvm.org/LICENSE.txt for license information.                  */
/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

/* This file enumerates variables from the LLVM configuration so that they
   can be in exported headers and won't override package specific directives.
   This is a C header that can be included in the llvm-c headers. */

#ifndef LLVM_CONFIG_H
#define LLVM_CONFIG_H

#if !defined(__i386__) && defined(_M_IX86)
#define __i386__ 1
#endif

#if !defined(__x86_64__) && (defined(_M_AMD64) || defined (_M_X64))
#define __x86_64__ 1
#endif

#define LLVM_CONFIG_H

/* Define if LLVM_ENABLE_DUMP is enabled */
/* #undef LLVM_ENABLE_DUMP */

/* Target triple LLVM will generate code for by default */
#if defined(__x86_64__)
#define LLVM_DEFAULT_TARGET_TRIPLE "x86_64-unknown-fuchsia"
#elif defined(__aarch64__)
#define LLVM_DEFAULT_TARGET_TRIPLE "aarch64-unknown-fuchsia"
#else
#error "unknown architecture"
#endif

/* Define if threads enabled */
#define LLVM_ENABLE_THREADS 1

/* Has gcc/MSVC atomic intrinsics */
#define LLVM_HAS_ATOMICS 1

/* Host triple LLVM will be executed on */
#if defined(__x86_64__)
#define LLVM_HOST_TRIPLE "x86_64-unknown-fuchsia"
#elif defined(__aarch64__)
#define LLVM_HOST_TRIPLE "aarch64-unknown-fuchsia"
#else
#error "unknown architecture"
#endif

/* LLVM architecture name for the native architecture, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_ARCH AArch64
#elif defined(__arm__)
#define LLVM_NATIVE_ARCH ARM
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_ARCH X86
#elif defined(__mips__)
#define LLVM_NATIVE_ARCH Mips
#elif defined(__powerpc64__)
#define LLVM_NATIVE_ARCH PowerPC
#else
#error "unknown architecture"
#endif

/* LLVM name for the native AsmParser init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_ASMPARSER LLVMInitializeAArch64AsmParser
#elif defined(__arm__)
#define LLVM_NATIVE_ASMPARSER LLVMInitializeARMAsmParser
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_ASMPARSER LLVMInitializeX86AsmParser
#elif defined(__mips__)
#define LLVM_NATIVE_ASMPARSER LLVMInitializeMipsAsmParser
#elif defined(__powerpc64__)
#define LLVM_NATIVE_ASMPARSER LLVMInitializePowerPCAsmParser
#else
#error "unknown architecture"
#endif

/* LLVM name for the native AsmPrinter init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeAArch64AsmPrinter
#elif defined(__arm__)
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeARMAsmPrinter
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeX86AsmPrinter
#elif defined(__mips__)
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeMipsAsmPrinter
#elif defined(__powerpc64__)
#define LLVM_NATIVE_ASMPRINTER LLVMInitializePowerPCAsmPrinter
#else
#error "unknown architecture"
#endif

/* LLVM name for the native Disassembler init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeAArch64Disassembler
#elif defined(__arm__)
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeARMDisassembler
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeX86Disassembler
#elif defined(__mips__)
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeMipsDisassembler
#elif defined(__powerpc64__)
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializePowerPCDisassembler
#else
#error "unknown architecture"
#endif

/* LLVM name for the native Target init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_TARGET LLVMInitializeAArch64Target
#elif defined(__arm__)
#define LLVM_NATIVE_TARGET LLVMInitializeARMTarget
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_TARGET LLVMInitializeX86Target
#elif defined(__mips__)
#define LLVM_NATIVE_TARGET LLVMInitializeMipsTarget
#elif defined(__powerpc64__)
#define LLVM_NATIVE_TARGET LLVMInitializePowerPCTarget
#else
#error "unknown architecture"
#endif

/* LLVM name for the native TargetInfo init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_TARGETINFO LLVMInitializeAArch64TargetInfo
#elif defined(__arm__)
#define LLVM_NATIVE_TARGETINFO LLVMInitializeARMTargetInfo
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_TARGETINFO LLVMInitializeX86TargetInfo
#elif defined(__mips__)
#define LLVM_NATIVE_TARGETINFO LLVMInitializeMipsTargetInfo
#elif defined(__powerpc64__)
#define LLVM_NATIVE_TARGETINFO LLVMInitializePowerPCTargetInfo
#else
#error "unknown architecture"
#endif

/* LLVM name for the native target MC init function, if available */
#if defined(__aarch64__)
#define LLVM_NATIVE_TARGETMC LLVMInitializeAArch64TargetMC
#elif defined(__arm__)
#define LLVM_NATIVE_TARGETMC LLVMInitializeARMTargetMC
#elif defined(__i386__) || defined(__x86_64__)
#define LLVM_NATIVE_TARGETMC LLVMInitializeX86TargetMC
#elif defined(__mips__)
#define LLVM_NATIVE_TARGETMC LLVMInitializeMipsTargetMC
#elif defined(__powerpc64__)
#define LLVM_NATIVE_TARGETMC LLVMInitializePowerPCTargetMC
#else
#error "unknown architecture"
#endif

/* Define if this is Unixish platform */
#define LLVM_ON_UNIX 1

/* Define if we have the Intel JIT API runtime support library */
#define LLVM_USE_INTEL_JITEVENTS 0

/* Define if we have the oprofile JIT-support library */
#define LLVM_USE_OPROFILE 0

/* Define if we have the perf JIT-support library */
#define LLVM_USE_PERF 0

/* Major version of the LLVM API */
#define LLVM_VERSION_MAJOR 10

/* Minor version of the LLVM API */
#define LLVM_VERSION_MINOR 0

/* Patch version of the LLVM API */
#define LLVM_VERSION_PATCH 0

/* LLVM version string */
#define LLVM_VERSION_STRING "10.0.0"

/* Whether LLVM records statistics for use with GetStatistics(),
 * PrintStatistics() or PrintStatisticsJSON()
 */
#define LLVM_FORCE_ENABLE_STATS 0

#endif
