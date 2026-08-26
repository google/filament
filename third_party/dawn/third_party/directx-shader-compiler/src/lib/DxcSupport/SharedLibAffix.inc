///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SharedLibAffix.inc                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines shared library prefixes and suffixes for the build platform.      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#cmakedefine CMAKE_SHARED_LIBRARY_PREFIX "@CMAKE_SHARED_LIBRARY_PREFIX@"
#cmakedefine CMAKE_SHARED_LIBRARY_SUFFIX "@CMAKE_SHARED_LIBRARY_SUFFIX@"

#ifndef CMAKE_SHARED_LIBRARY_PREFIX
#define CMAKE_SHARED_LIBRARY_PREFIX
#endif

#ifndef CMAKE_SHARED_LIBRARY_SUFFIX
#define CMAKE_SHARED_LIBRARY_SUFFIX
#endif