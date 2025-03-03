/* This generated file is for internal use. Do not include it from headers. */

#ifdef CONFIG_H
#error config.h can only be included once
#else
#define CONFIG_H

/* Bug report URL. */
#define BUG_REPORT_URL "${BUG_REPORT_URL}"

/* Default OpenMP runtime used by -fopenmp. */
#define CLANG_DEFAULT_OPENMP_RUNTIME "${CLANG_DEFAULT_OPENMP_RUNTIME}"

/* Multilib suffix for libdir. */
#define CLANG_LIBDIR_SUFFIX "${CLANG_LIBDIR_SUFFIX}"

/* Relative directory for resource files */
#define CLANG_RESOURCE_DIR "${CLANG_RESOURCE_DIR}"

/* Directories clang will search for headers */
#define C_INCLUDE_DIRS "${C_INCLUDE_DIRS}"

/* Default <path> to all compiler invocations for --sysroot=<path>. */
#define DEFAULT_SYSROOT "${DEFAULT_SYSROOT}"

/* Directory where gcc is installed. */
#define GCC_INSTALL_PREFIX "${GCC_INSTALL_PREFIX}"

/* Define if we have libxml2 */
#cmakedefine CLANG_HAVE_LIBXML ${CLANG_HAVE_LIBXML}

/* The LLVM product name and version */
#define BACKEND_PACKAGE_STRING "${BACKEND_PACKAGE_STRING}"

/* Linker version detected at compile time. */
#cmakedefine HOST_LINK_VERSION "${HOST_LINK_VERSION}"

#endif
