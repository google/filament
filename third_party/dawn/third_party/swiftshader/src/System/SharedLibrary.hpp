// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SharedLibrary_hpp
#define SharedLibrary_hpp

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <dlfcn.h>
#endif

#include <string>

void *getLibraryHandle(const char *path);
void *loadLibrary(const char *path);
void freeLibrary(void *library);
void *getProcAddress(void *library, const char *name);

template<int n>
void *loadLibrary(const std::string &libraryDirectory, const char *(&names)[n], const char *mustContainSymbol = nullptr)
{
	for(const char *libraryName : names)
	{
		std::string libraryPath = libraryDirectory + libraryName;
		void *library = getLibraryHandle(libraryPath.c_str());

		if(library)
		{
			if(!mustContainSymbol || getProcAddress(library, mustContainSymbol))
			{
				return library;
			}

			freeLibrary(library);
		}
	}

	for(const char *libraryName : names)
	{
		std::string libraryPath = libraryDirectory + libraryName;
		void *library = loadLibrary(libraryPath.c_str());

		if(library)
		{
			if(!mustContainSymbol || getProcAddress(library, mustContainSymbol))
			{
				return library;
			}

			freeLibrary(library);
		}
	}

	return nullptr;
}

#if defined(_WIN32)
inline void *loadLibrary(const char *path)
{
	return (void *)LoadLibrary(path);
}

inline void *getLibraryHandle(const char *path)
{
	HMODULE module = NULL;
	GetModuleHandleEx(0, path, &module);
	return (void *)module;
}

inline void freeLibrary(void *library)
{
	FreeLibrary((HMODULE)library);
}

inline void *getProcAddress(void *library, const char *name)
{
	return (void *)GetProcAddress((HMODULE)library, name);
}
#else
inline void *loadLibrary(const char *path)
{
	return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}

inline void *getLibraryHandle(const char *path)
{
#	ifdef __ANDROID__
	// bionic doesn't support RTLD_NOLOAD before L
	return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#	else
	void *resident = dlopen(path, RTLD_LAZY | RTLD_NOLOAD | RTLD_LOCAL);

	if(resident)
	{
		return dlopen(path, RTLD_LAZY | RTLD_LOCAL);  // Increment reference count
	}

	return nullptr;
#	endif
}

inline void freeLibrary(void *library)
{
	if(library)
	{
		dlclose(library);
	}
}

inline void *getProcAddress(void *library, const char *name)
{
	void *symbol = dlsym(library, name);

	if(!symbol)
	{
		const char *reason = dlerror();  // Silence the error
		(void)reason;
	}

	return symbol;
}
#endif

template<typename FunctionPointer>
inline void getFuncAddress(void *library, const char *functionName, FunctionPointer *out)
{
	*out = reinterpret_cast<FunctionPointer>(getProcAddress(library, functionName));
}

#endif  // SharedLibrary_hpp
