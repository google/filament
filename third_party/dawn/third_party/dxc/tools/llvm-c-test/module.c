/*===-- module.c - tool for testing libLLVM and llvm-c API ----------------===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file implements the --module-dump, --module-list-functions and        *|
|* --module-list-globals commands in llvm-c-test.                             *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#include "llvm-c-test.h"
#include "llvm-c/BitReader.h"
#include "llvm-c/Core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LLVMModuleRef load_module(void) {
  LLVMMemoryBufferRef MB;
  LLVMModuleRef M;
  char *msg = NULL;

  if (LLVMCreateMemoryBufferWithSTDIN(&MB, &msg)) {
    fprintf(stderr, "Error reading file: %s\n", msg);
    exit(1);
  }

  if (LLVMParseBitcode(MB, &M, &msg)) {
    fprintf(stderr, "Error parsing bitcode: %s\n", msg);
    LLVMDisposeMemoryBuffer(MB);
    exit(1);
  }

  LLVMDisposeMemoryBuffer(MB);
  return M;
}

int module_dump(void) {
  LLVMModuleRef M = load_module();

  char *irstr = LLVMPrintModuleToString(M);
  puts(irstr);
  LLVMDisposeMessage(irstr);

  LLVMDisposeModule(M);

  return 0;
}

int module_list_functions(void) {
  LLVMModuleRef M = load_module();
  LLVMValueRef f;

  f = LLVMGetFirstFunction(M);
  while (f) {
    if (LLVMIsDeclaration(f)) {
      printf("FunctionDeclaration: %s\n", LLVMGetValueName(f));
    } else {
      LLVMBasicBlockRef bb;
      LLVMValueRef isn;
      unsigned nisn = 0;
      unsigned nbb = 0;

      printf("FunctionDefinition: %s [#bb=%u]\n", LLVMGetValueName(f),
             LLVMCountBasicBlocks(f));

      for (bb = LLVMGetFirstBasicBlock(f); bb;
           bb = LLVMGetNextBasicBlock(bb)) {
        nbb++;
        for (isn = LLVMGetFirstInstruction(bb); isn;
             isn = LLVMGetNextInstruction(isn)) {
          nisn++;
          if (LLVMIsACallInst(isn)) {
            LLVMValueRef callee =
                LLVMGetOperand(isn, LLVMGetNumOperands(isn) - 1);
            printf(" calls: %s\n", LLVMGetValueName(callee));
          }
        }
      }
      printf(" #isn: %u\n", nisn);
      printf(" #bb: %u\n\n", nbb);
    }
    f = LLVMGetNextFunction(f);
  }

  LLVMDisposeModule(M);

  return 0;
}

int module_list_globals(void) {
  LLVMModuleRef M = load_module();
  LLVMValueRef g;

  g = LLVMGetFirstGlobal(M);
  while (g) {
    LLVMTypeRef T = LLVMTypeOf(g);
    char *s = LLVMPrintTypeToString(T);

    printf("Global%s: %s %s\n",
           LLVMIsDeclaration(g) ? "Declaration" : "Definition",
           LLVMGetValueName(g), s);

    LLVMDisposeMessage(s);

    g = LLVMGetNextGlobal(g);
  }

  LLVMDisposeModule(M);

  return 0;
}
