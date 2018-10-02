/*
 * Copyright 2015 The Etc2Comp Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "EtcMemTest.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
using namespace std;

#if defined(_WIN32)
#include <Windows.h>
#include <psapi.h>
// Use to convert bytes to KB
#define DIV 1024

// Specify the width of the field in which to print the numbers. 
// The asterisk in the format specifier "%*I64d" takes an integer 
// argument and uses it to pad and right justify the number.
#define WIDTH 7

size_t GetMemoryUsageAmount()
{
	//MEMORYSTATUS status; // if 64 bit version isnt working, use 32bit version
	//GlobalMemoryStatus(&status);
	/*MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return (size_t)(status.ullAvailPhys / DIV);*/

	PROCESS_MEMORY_COUNTERS_EX pmc;
	DWORD ret = GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	if (ret == 0)
	{
		printf("GetProcessMemoryInfo failed with code %d\n", GetLastError());
	}
	
	return pmc.PrivateUsage;
}

void PrintWindowsMemUsage()
{
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof(statex);

	GlobalMemoryStatusEx(&statex);

	printf("There is  %*ld percent of memory in use.\n",
		WIDTH, statex.dwMemoryLoad);
	printf("There are %*I64d total KB of physical memory.\n",
		WIDTH, statex.ullTotalPhys / DIV);
	printf("There are %*I64d free  KB of physical memory.\n",
		WIDTH, statex.ullAvailPhys / DIV);
	printf("There are %*I64d total KB of paging file.\n",
		WIDTH, statex.ullTotalPageFile / DIV);
	printf("There are %*I64d free  KB of paging file.\n",
		WIDTH, statex.ullAvailPageFile / DIV);
	printf("There are %*I64d total KB of virtual memory.\n",
		WIDTH, statex.ullTotalVirtual / DIV);
	printf("There are %*I64d free  KB of virtual memory.\n",
		WIDTH, statex.ullAvailVirtual / DIV);

	// Show the amount of extended memory available.

	printf("There are %*I64d free  KB of extended memory.\n",
		WIDTH, statex.ullAvailExtendedVirtual / DIV);
}




int RunMemTest(bool verboseOutput, size_t numTestIterations)
{
	WIN32_FIND_DATA ffd;
	srand(static_cast<unsigned int>(time(NULL)));
	HANDLE hFind = INVALID_HANDLE_VALUE;

	
	vector<string> allImages;

	string imagesDir = "D:\\source\\etc2Sourcetree\\TestImages\\";
	string outputDir = "C:\\Users\\BSI\\Desktop\\etc2comp\\";

	hFind = FindFirstFile((imagesDir+"*").c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf ("image dir doesnt exist: %s\n", imagesDir.c_str());
		return -1;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			string filename = ffd.cFileName;
			size_t found = filename.find_last_of(".");
			string ext = filename.substr(found + 1);
			if(ext != "png")
				continue;

			allImages.push_back(filename);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	size_t oldMemSize = 0;
	size_t curMemSize = 0;
	size_t fileIndex = 0;
	int encodingType = 2;//0 = c , 1= both c and c++ encoding, 2 = c++ encoding
	bool encodeRandomImage = true;
	for (size_t i = 0; i < numTestIterations; i++)
	{
		encodingType = rand()%3;

		printf("--------------iteration %zu-----------\n",i+1);
		int randomNumber = 1 + rand() % 11;
		//Image::Format format = (Image::Format)randomNumber;
		Image::Format format = Image::Format::RGBA8;

		randomNumber = rand() % 4;
		ErrorMetric e_ErrMetric = (ErrorMetric)randomNumber;

		randomNumber = rand() % 100;
		float curEffort = (float)randomNumber;

		int jobs = 1 + rand() % 8;
		long memDiff = 0;

		if (verboseOutput)
		{
			printf("    effort = %.f\n", curEffort);
			printf("  encoding =  %s\n", Image::EncodingFormatToString(format));
			printf("  error metric: %s\n", ErrorMetricToString(e_ErrMetric));
			printf("jobs: %d\n", jobs);
		}

		oldMemSize = curMemSize;
		curMemSize = GetMemoryUsageAmount();
		memDiff = (long)curMemSize - (long)oldMemSize;
		printf("mem diff: %ukb\n", memDiff);
		
		string curFile = imagesDir + allImages[fileIndex];
		string outFile = outputDir + allImages[fileIndex] +".ktx";
		printf("file: %s\n", curFile.c_str());

		if (!encodeRandomImage)
		{
			if (fileIndex >= allImages.size() - 1)
				fileIndex = 0;
			else
				fileIndex++;
		}
		else
		{
			fileIndex = rand()%allImages.size();
		}
		SourceImage sourceimage(curFile.c_str(), -1, -1);

		unsigned int uiSourceWidth = sourceimage.GetWidth();
		unsigned int uiSourceHeight = sourceimage.GetHeight();
		//char *imgonnaleak = new char[1000];

		unsigned char *paucEncodingBits;
		unsigned int uiEncodingBitsBytes;
		unsigned int uiExtendedWidth;
		unsigned int uiExtendedHeight;
		int iEncodingTime_ms;
		//////////////C INTERFACE FIRST//////////////////////
		if (encodingType < 2)
		{
			if (verboseOutput)
			{
				printf("start C interface Encoding:\n");
			}

			Etc::Encode((float *)sourceimage.GetPixels(),
				uiSourceWidth, uiSourceHeight,
				format,
				e_ErrMetric,
				curEffort,
				jobs,
				MAX_JOBS,
				&paucEncodingBits, &uiEncodingBitsBytes,
				&uiExtendedWidth, &uiExtendedHeight,
				&iEncodingTime_ms, verboseOutput);
			if (verboseOutput)
			{
				printf("    encode time = %dms\n", iEncodingTime_ms);
				printf("EncodedImage: %s\n", outFile.c_str());
			}
			Etc::File C_interfaceEtcfile(outFile.c_str(), Etc::File::Format::INFER_FROM_FILE_EXTENSION,
				format,
				paucEncodingBits, uiEncodingBitsBytes,
				uiSourceWidth, uiSourceHeight,
				uiExtendedWidth, uiExtendedHeight);
			C_interfaceEtcfile.Write();

			oldMemSize = curMemSize;
			curMemSize = GetMemoryUsageAmount();
			memDiff = (long)curMemSize - (long)oldMemSize;
			printf("mem diff again: %ukb\n", memDiff);
		}
		//////////////C++ INTERFACE ///////////////////////////
		if (encodingType > 0)
		{
			if (verboseOutput)
			{
				printf("start C++ Encoding:\n");
			}
			Etc::Image image((float *)sourceimage.GetPixels(),
				uiSourceWidth, uiSourceHeight,
				e_ErrMetric);
			image.m_bVerboseOutput = verboseOutput;

			Etc::Image::EncodingStatus encStatus = Etc::Image::EncodingStatus::SUCCESS;

			encStatus = image.Encode(format, e_ErrMetric, curEffort, jobs, MAX_JOBS);
			if (verboseOutput)
			{
				printf("  encode time = %dms\n", image.GetEncodingTimeMs());
				printf("EncodedImage: %s\n", outFile.c_str());
				printf("status bitfield: %u\n", encStatus);
			}
			Etc::File etcfile(outFile.c_str(), Etc::File::Format::INFER_FROM_FILE_EXTENSION,
				format,
				image.GetEncodingBits(), image.GetEncodingBitsBytes(),
				image.GetSourceWidth(), image.GetSourceHeight(),
				image.GetExtendedWidth(), image.GetExtendedHeight());

			etcfile.Write();
		}
		
	}

	return 0;
}

#endif
