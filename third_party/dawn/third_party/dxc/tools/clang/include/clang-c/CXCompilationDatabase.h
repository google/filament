/*===-- clang-c/CXCompilationDatabase.h - Compilation database  ---*- C -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header provides a public inferface to use CompilationDatabase without *|
|* the full Clang C++ API.                                                    *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/
#ifndef LLVM_CLANG_C_CXCOMPILATIONDATABASE_H
#define LLVM_CLANG_C_CXCOMPILATIONDATABASE_H

#include "clang-c/Platform.h"
#include "clang-c/CXString.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup COMPILATIONDB CompilationDatabase functions
 * \ingroup CINDEX
 *
 * @{
 */

/**
 * A compilation database holds all information used to compile files in a
 * project. For each file in the database, it can be queried for the working
 * directory or the command line used for the compiler invocation.
 *
 * Must be freed by \c clang_CompilationDatabase_dispose
 */
typedef void * CXCompilationDatabase;

/**
 * \brief Contains the results of a search in the compilation database
 *
 * When searching for the compile command for a file, the compilation db can
 * return several commands, as the file may have been compiled with
 * different options in different places of the project. This choice of compile
 * commands is wrapped in this opaque data structure. It must be freed by
 * \c clang_CompileCommands_dispose.
 */
typedef void * CXCompileCommands;

/**
 * \brief Represents the command line invocation to compile a specific file.
 */
typedef void * CXCompileCommand;

/**
 * \brief Error codes for Compilation Database
 */
typedef enum  {
  /*
   * \brief No error occurred
   */
  CXCompilationDatabase_NoError = 0,

  /*
   * \brief Database can not be loaded
   */
  CXCompilationDatabase_CanNotLoadDatabase = 1

} CXCompilationDatabase_Error;

/**
 * \brief Creates a compilation database from the database found in directory
 * buildDir. For example, CMake can output a compile_commands.json which can
 * be used to build the database.
 *
 * It must be freed by \c clang_CompilationDatabase_dispose.
 */
CINDEX_LINKAGE CXCompilationDatabase
clang_CompilationDatabase_fromDirectory(const char *BuildDir,
                                        CXCompilationDatabase_Error *ErrorCode);

/**
 * \brief Free the given compilation database
 */
CINDEX_LINKAGE void
clang_CompilationDatabase_dispose(CXCompilationDatabase);

/**
 * \brief Find the compile commands used for a file. The compile commands
 * must be freed by \c clang_CompileCommands_dispose.
 */
CINDEX_LINKAGE CXCompileCommands
clang_CompilationDatabase_getCompileCommands(CXCompilationDatabase,
                                             const char *CompleteFileName);

/**
 * \brief Get all the compile commands in the given compilation database.
 */
CINDEX_LINKAGE CXCompileCommands
clang_CompilationDatabase_getAllCompileCommands(CXCompilationDatabase);

/**
 * \brief Free the given CompileCommands
 */
CINDEX_LINKAGE void clang_CompileCommands_dispose(CXCompileCommands);

/**
 * \brief Get the number of CompileCommand we have for a file
 */
CINDEX_LINKAGE unsigned
clang_CompileCommands_getSize(CXCompileCommands);

/**
 * \brief Get the I'th CompileCommand for a file
 *
 * Note : 0 <= i < clang_CompileCommands_getSize(CXCompileCommands)
 */
CINDEX_LINKAGE CXCompileCommand
clang_CompileCommands_getCommand(CXCompileCommands, unsigned I);

/**
 * \brief Get the working directory where the CompileCommand was executed from
 */
CINDEX_LINKAGE CXString
clang_CompileCommand_getDirectory(CXCompileCommand);

/**
 * \brief Get the number of arguments in the compiler invocation.
 *
 */
CINDEX_LINKAGE unsigned
clang_CompileCommand_getNumArgs(CXCompileCommand);

/**
 * \brief Get the I'th argument value in the compiler invocations
 *
 * Invariant :
 *  - argument 0 is the compiler executable
 */
CINDEX_LINKAGE CXString
clang_CompileCommand_getArg(CXCompileCommand, unsigned I);

/**
 * \brief Get the number of source mappings for the compiler invocation.
 */
CINDEX_LINKAGE unsigned
clang_CompileCommand_getNumMappedSources(CXCompileCommand);

/**
 * \brief Get the I'th mapped source path for the compiler invocation.
 */
CINDEX_LINKAGE CXString
clang_CompileCommand_getMappedSourcePath(CXCompileCommand, unsigned I);

/**
 * \brief Get the I'th mapped source content for the compiler invocation.
 */
CINDEX_LINKAGE CXString
clang_CompileCommand_getMappedSourceContent(CXCompileCommand, unsigned I);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif

