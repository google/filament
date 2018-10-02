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

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS (1)
#endif
/*
since this code will be used on a wide varity of platforms and configurations
its important to have some sort of sanity check for the amount of threads that can be used.
change this macro to suit your configuration. This will be the maximum amount of threads
that can be created. 
*/
#define MAX_JOBS 1024

#define RUN_MEM_TEST 0

#include "EtcConfig.h"

#include "Etc.h"
#include "EtcFilter.h"

#include "EtcTool.h"
#include "EtcSourceImage.h"
#include "EtcFile.h"
#include "EtcMath.h"
#include "EtcImage.h"
#include "EtcErrorMetric.h"
#include "EtcBlock4x4EncodingBits.h"

#include "EtcAnalysis.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

using namespace Etc;

#if ETC_WINDOWS
const char *ETC_MKDIR_COMMAND = "mkdir";

int strcasecmp(const char *s1, const char *s2)
{
	return _stricmp(s1, s2);
}
#else
const char *ETC_MKDIR_COMMAND = "mkdir -p";
#endif

#if RUN_MEM_TEST
#include "EtcMemTest.h"
#endif

class Commands
{
public:

	static const unsigned int MIN_JOBS = 8;

	Commands(void)
	{
		pstrSourceFilename = nullptr;
		pstrOutputFilename = nullptr;
		format = Image::Format::DEFAULT;
		pstrAnalysisDirectory = nullptr;
		uiComparisons = 0;
		for (unsigned int uiComparison = 0; uiComparison < Analysis::MAX_COMPARISONS; uiComparison++)
		{
			apstrCompareFilename[uiComparison] = nullptr;
		}
		fEffort = ETCCOMP_DEFAULT_EFFORT_LEVEL;
		//Rec. 709 or BT.709...the default
		e_ErrMetric = ErrorMetric::BT709;
		uiJobs = MIN_JOBS;

		//these are ignored if they are < 0
		i_hPixel = -1;
		i_vPixel = -1;
		verboseOutput = false;
		boolNormalizeXYZ = false;
		mipmaps = 1;
		mipFilterFlags = Etc::FILTER_WRAP_NONE;
	}

	bool ProcessCommandLineArguments(int a_iArgs, const char *a_apstrArgs[]);
	void PrintUsageMessage(void);
	static void FixSlashes(char *a_pstr);

	char *pstrSourceFilename;
	char *pstrOutputFilename;

	Image::Format format;
	char *pstrAnalysisDirectory;
	char *formatType;
	unsigned int uiComparisons;
	char *apstrCompareFilename[Analysis::MAX_COMPARISONS];
	float fEffort;
	ErrorMetric e_ErrMetric;
	unsigned int uiJobs;		// for threading
	bool verboseOutput;
	//when both of these are >= 0 then single block mode is on
	int i_hPixel;
	int i_vPixel;
	bool boolNormalizeXYZ;
	int mipmaps;
	unsigned int mipFilterFlags;
};

#include "EtcFileHeader.h"

// ----------------------------------------------------------------------------------------------------
//
int main(int argc, const char * argv[])
{

	static const bool USE_C_INTERFACE = false;

	// this code tests for memory leaks
#if RUN_MEM_TEST
	RunMemTest(true, 100);
	printf("an extra line to see how the memory is free'd\n");
	printf("all done!\n");
	exit(0);
#endif

	Commands commands;
	bool boolPrintUsage = commands.ProcessCommandLineArguments(argc, argv);
	if (boolPrintUsage)
	{
		commands.PrintUsageMessage();
		exit(1);
	}

	if (commands.verboseOutput)
	{
		printf("SourceImage: %s\n", commands.pstrSourceFilename);
	}
	SourceImage sourceimage(commands.pstrSourceFilename, commands.i_hPixel, commands.i_vPixel);
	if (commands.boolNormalizeXYZ)
	{
		sourceimage.NormalizeXYZ();
	}

	unsigned int uiSourceWidth = sourceimage.GetWidth();
	unsigned int uiSourceHeight = sourceimage.GetHeight();

	if(commands.mipmaps != 1)
	{
		int iEncodingTime_ms;

		// Calculate the maximum number of possible mipmaps
		{
			int dim = (uiSourceWidth < uiSourceHeight)?uiSourceWidth:uiSourceHeight;
			int maxMips = 0;
			while(dim >= 1)
			{
				maxMips++;
				dim >>= 1;
			}
			if( commands.mipmaps == 0 || commands.mipmaps > maxMips)
			{
				commands.mipmaps = maxMips;
			}
		}

		Etc::RawImage *pMipmapImages = new Etc::RawImage[commands.mipmaps];

		if (commands.verboseOutput)
		{
			printf("Encoding:\n");
			printf("    effort = %.f\n", commands.fEffort);
			printf("  encoding =  %s\n", Image::EncodingFormatToString(commands.format));
			printf("  error metric: %s\n", ErrorMetricToString(commands.e_ErrMetric));
		}
		Etc::EncodeMipmaps((float *)sourceimage.GetPixels(),
			uiSourceWidth, uiSourceHeight,
			commands.format,
			commands.e_ErrMetric,
			commands.fEffort,
			commands.uiJobs,
			MAX_JOBS,
			commands.mipmaps,
			commands.mipFilterFlags,
			pMipmapImages,
			&iEncodingTime_ms);
		if (commands.verboseOutput)
		{
			printf("    encode time = %dms\n", iEncodingTime_ms);
			printf("EncodedImage: %s\n", commands.pstrOutputFilename);
		}
		Etc::File etcfile(commands.pstrOutputFilename, Etc::File::Format::INFER_FROM_FILE_EXTENSION,
			commands.format,
			commands.mipmaps,
			pMipmapImages,
			uiSourceWidth, uiSourceHeight );
		etcfile.Write();

		delete [] pMipmapImages;
	}
	else if (USE_C_INTERFACE)
	{
		unsigned char *paucEncodingBits;
		unsigned int uiEncodingBitsBytes;
		unsigned int uiExtendedWidth;
		unsigned int uiExtendedHeight;
		int iEncodingTime_ms;
		
		if (commands.verboseOutput)
		{
			printf("Encoding:\n");
			printf("    effort = %.f\n", commands.fEffort);
			printf("  encoding =  %s\n", Image::EncodingFormatToString(commands.format));
			printf("  error metric: %s\n", ErrorMetricToString(commands.e_ErrMetric));
		}
		Etc::Encode((float *)sourceimage.GetPixels(),
					uiSourceWidth, uiSourceHeight,
					commands.format,
					commands.e_ErrMetric,
					commands.fEffort,
					commands.uiJobs,
					MAX_JOBS,
					&paucEncodingBits, &uiEncodingBitsBytes,
					&uiExtendedWidth, &uiExtendedHeight,
					&iEncodingTime_ms);
		if (commands.verboseOutput)
		{
			printf("    encode time = %dms\n", iEncodingTime_ms);
			printf("EncodedImage: %s\n", commands.pstrOutputFilename);
		}
		Etc::File etcfile(commands.pstrOutputFilename, Etc::File::Format::INFER_FROM_FILE_EXTENSION,
							commands.format,
							paucEncodingBits, uiEncodingBitsBytes,
							uiSourceWidth, uiSourceHeight,
							uiExtendedWidth, uiExtendedHeight);
		etcfile.Write();
	}
	else
	{
		if (commands.verboseOutput)
		{
			printf("Encoding:\n");
			printf("  effort = %.f%%\n", commands.fEffort);
			printf("  encoding =  %s\n", Image::EncodingFormatToString(commands.format));
			printf("  error metric: %s\n", ErrorMetricToString(commands.e_ErrMetric));
		}
		Etc::Image image((float *)sourceimage.GetPixels(),
							uiSourceWidth, uiSourceHeight,
							commands.e_ErrMetric);
		image.m_bVerboseOutput = commands.verboseOutput;
		Etc::Image::EncodingStatus encStatus = Etc::Image::EncodingStatus::SUCCESS;
		
		encStatus = image.Encode(commands.format, commands.e_ErrMetric, commands.fEffort, commands.uiJobs,MAX_JOBS);
		if (commands.verboseOutput)
		{
			printf("  encode time = %dms\n", image.GetEncodingTimeMs());
			printf("EncodedImage: %s\n", commands.pstrOutputFilename);
			printf("status bitfield: %u\n", encStatus);
		}
		Etc::File etcfile(commands.pstrOutputFilename, Etc::File::Format::INFER_FROM_FILE_EXTENSION,
							commands.format,
							image.GetEncodingBits(), image.GetEncodingBitsBytes(),
							image.GetSourceWidth(), image.GetSourceHeight(),
							image.GetExtendedWidth(), image.GetExtendedHeight());

		etcfile.Write();

		if (commands.pstrAnalysisDirectory)
		{
			if (commands.verboseOutput)
			{
				printf("Analysis: %s\n", commands.pstrAnalysisDirectory);
			}
			Analysis analysis(&image, commands.pstrAnalysisDirectory);

			for (unsigned int uiComparison = 0; uiComparison < commands.uiComparisons; uiComparison++)
			{
				analysis.Compare(commands.apstrCompareFilename[uiComparison], commands.i_hPixel, commands.i_vPixel);
			}
		}

	}

	return 0;
}

// ----------------------------------------------------------------------------------------------------
// return true if usage message should be printed
//
bool Commands::ProcessCommandLineArguments(int a_iArgs, const char *a_apstrArgs[])
{
	static const bool DEBUG_PRINT = false;

	if (a_iArgs == 1)
	{
		printf("Error: missing arguments\n");
		return true;
	}

	for (int iArg = 1; iArg < a_iArgs; iArg++)
    {
		if (DEBUG_PRINT)
		{
			printf("%s: %u %s\n", a_apstrArgs[0], iArg, a_apstrArgs[iArg]);
		}

		if (strcmp(a_apstrArgs[iArg], "-analyze") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing folder parameter for -analyze\n");
				return true;
			}
			else
			{
				pstrAnalysisDirectory = new char[strlen(a_apstrArgs[iArg]) + 1];
				strcpy(pstrAnalysisDirectory, a_apstrArgs[iArg]);
				FixSlashes(pstrAnalysisDirectory);
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-argfile") == 0)
		{
			static const unsigned int MAX_LINE_CHARS = 1000;
			static const unsigned int MAX_ARGFILE_ARGS = 100;

			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing file parameter for -argfile\n");
				return true;
			}
			else
			{
				FILE *pfile = fopen(a_apstrArgs[iArg], "rt");
				if (pfile == nullptr)
				{
					printf("Error: couldn't open argfile (%s)\n", a_apstrArgs[iArg]);
					return true;
				}

				char **apstrArgs = new char *[MAX_ARGFILE_ARGS];
				assert(apstrArgs);

				// add null executable name
				apstrArgs[0] = const_cast<char *>("");
				int iArgs = 1;

				// read in tokens
				{
					char *pcTokens = new char[MAX_LINE_CHARS + 1];
					assert(pcTokens);
					char *pcToken = nullptr;

					// read in each line
					while (fgets(pcTokens, MAX_LINE_CHARS, pfile))
					{
						// skip over lines with '#' in first char
						if (pcTokens[0] == '#')
						{
							continue;
						}

						// abort remainder of argfile with '%' in first char
						if (pcTokens[0] == '%')
						{
							break;
						}
						pcToken = strtok(pcTokens, " \n\r");

						if (pcToken != nullptr)
						{
							apstrArgs[iArgs] = new char[strlen(pcToken) + 1];
							strcpy(apstrArgs[iArgs], pcToken);
							iArgs++;
						}

						while (pcToken != nullptr)
						{
							pcToken = strtok(nullptr, " \n");
							if (pcToken != nullptr)
							{
								apstrArgs[iArgs] = new char[strlen(pcToken) + 1];
								strcpy(apstrArgs[iArgs], pcToken);
								iArgs++;
							}
						}
					}

					delete[] pcTokens;
				}

				fclose(pfile);

				bool boolErrors = ProcessCommandLineArguments(iArgs, const_cast<const char **>(apstrArgs));

				for (iArg = 1; iArg < iArgs; iArg++)
				{
					delete[] apstrArgs[iArg];
				}
				delete[] apstrArgs;

				if (boolErrors)
				{
					return true;
				}
			}
		}
		//used for debugging...select a single block to encode
		//supply the horiz and very pos of the block
		else if (strcmp(a_apstrArgs[iArg], "-blockAtHV") == 0)
		{
			++iArg;

			//make sure we have two more args after -block
			if (iArg + 1 >= (a_iArgs))
			{
				printf("Error: missing horiz and vert position of pixel for single block mode \n");
				return true;
			}
			i_hPixel = atoi(a_apstrArgs[iArg]);
			++iArg;
			i_vPixel = atoi(a_apstrArgs[iArg]);
		}
		else if (strcmp(a_apstrArgs[iArg], "-compare") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing comprison_image parameter for -compare\n");
				return true;
			}
			else
			{
				if (uiComparisons >= Analysis::MAX_COMPARISONS)
				{
					printf("Error: too many comparisons\n");
					return true;
				}

				char **ppstrCompareFilename = &apstrCompareFilename[uiComparisons++];

				*ppstrCompareFilename = new char[strlen(a_apstrArgs[iArg]) + 1];
				strcpy(*ppstrCompareFilename, a_apstrArgs[iArg]);
				FixSlashes(*ppstrCompareFilename);
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-effort") == 0)
		{
			++iArg;

            if (iArg >= (a_iArgs))
            {
				printf("Error: missing amount parameter for -effort\n");
				return true;
			}
            else
            {
                float f;
                int iScans = sscanf(a_apstrArgs[iArg], "%f", &f);

                if (iScans != 1)
                {
					printf("Error: couldn't parse amount for -effort (%s)\n", a_apstrArgs[iArg]);
					return true;
				}
                else
                {
                    fEffort = f;
                }
            }
        }
		else if (strcmp(a_apstrArgs[iArg], "-errormetric") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing error metric type %s\n", a_apstrArgs[iArg]);
				return true;
			}
			else
			{
				if (strcmp(a_apstrArgs[iArg], "rgba") == 0)
				{
					e_ErrMetric = ErrorMetric::RGBA;
				}
				else if (strcmp(a_apstrArgs[iArg], "rgbx") == 0)
				{
					e_ErrMetric = ErrorMetric::RGBX;
				}
				else if (strcmp(a_apstrArgs[iArg], "rec709") == 0)
				{
					e_ErrMetric = ErrorMetric::REC709;
				}
				else if (strcmp(a_apstrArgs[iArg], "numeric") == 0)
				{
					e_ErrMetric = ErrorMetric::NUMERIC;
				}
				else if (strcmp(a_apstrArgs[iArg], "normalxyz") == 0 ||
					strcmp(a_apstrArgs[iArg], "normalXYZ") == 0)
				{
					e_ErrMetric = ErrorMetric::NORMALXYZ;
				}
				else
				{
					printf("unrecognized error metric (%s), using numeric\n", a_apstrArgs[iArg]);
					e_ErrMetric = ErrorMetric::NUMERIC;
				}
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-format") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing etc_format parameter for -format\n");
				return true;
			}
			else
			{
				formatType = new char[strlen(a_apstrArgs[iArg])+1];
				strcpy(formatType,a_apstrArgs[iArg]);
				if (strcmp(a_apstrArgs[iArg], "ETC1") == 0)
				{
					format = Image::Format::ETC1;
				}
				else if (strcmp(a_apstrArgs[iArg], "RGB8") == 0)
				{
					format = Image::Format::RGB8;
				}
				else if (strcmp(a_apstrArgs[iArg], "SRGB8") == 0)
				{
					format = Image::Format::SRGB8;
				}
				else if (strcmp(a_apstrArgs[iArg], "RGBA8") == 0)
				{
					format = Image::Format::RGBA8;
				}
				else if (strcmp(a_apstrArgs[iArg], "SRGBA8") == 0)
				{
					format = Image::Format::SRGBA8;
				}
				else if (strcmp(a_apstrArgs[iArg], "R11") == 0)
				{
					format = Image::Format::R11;
				}
				else if (strcmp(a_apstrArgs[iArg], "SIGNED_R11") == 0)
				{
					format = Image::Format::SIGNED_R11;
				}
				else if (strcmp(a_apstrArgs[iArg], "RG11") == 0)
				{
					format = Image::Format::RG11;
				}
				else if (strcmp(a_apstrArgs[iArg], "SIGNED_RG11") == 0)
				{
					format = Image::Format::SIGNED_RG11;
				}
				else if (strcmp(a_apstrArgs[iArg], "RGB8A1") == 0)
				{
					format = Image::Format::RGB8A1;
				}
				else if (strcmp(a_apstrArgs[iArg], "SRGB8A1") == 0)
				{
					format = Image::Format::SRGB8A1;
				}
				else
				{
					printf("Error: unknown etc_format parameter for -format\n");
					format = Image::Format::UNKNOWN;
					return true;
				}
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-help") == 0)
		{
			return true;
		}
		else if (strcmp(a_apstrArgs[iArg], "-j") == 0 ||
				 strcmp(a_apstrArgs[iArg], "-jobs") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing job count for %s\n", a_apstrArgs[iArg]);
				return true;
			}
			else
			{
				unsigned int ui;
				int iScans = sscanf(a_apstrArgs[iArg], "%u", &ui);

				if (iScans != 1)
				{
					printf("Error: couldn't parse job count for %s (%s)\n", a_apstrArgs[iArg-1], a_apstrArgs[iArg]);
					return true;
				}
				else
				{
					if (ui < MIN_JOBS)
					{
						ui = MIN_JOBS;
					}

					uiJobs = ui;
				}
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-normalizexyz") == 0 ||
				 strcmp(a_apstrArgs[iArg], "-normalizeXYZ") == 0)
		{
			boolNormalizeXYZ = true;
		}
		else if (strcmp(a_apstrArgs[iArg], "-output") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing encoded_image parameter for -output\n");
				return true;
			}
			else
			{
				pstrOutputFilename = new char[strlen(a_apstrArgs[iArg]) + 1];
				strcpy(pstrOutputFilename, a_apstrArgs[iArg]);
				//take the output file name and extract the directory path so we can create the directory if nescacary
				char *ptrOutputDir = nullptr;

				FixSlashes(pstrOutputFilename);
				for (int c = (int)strlen(pstrOutputFilename); c > 0; c--)
				{
					//find the last slash, to get the name of the directory
					if (pstrOutputFilename[c] == ETC_PATH_SLASH)
					{
						c++;
						ptrOutputDir = new char[c];
						strncpy(ptrOutputDir, pstrOutputFilename, c);
						ptrOutputDir[c] = '\0';
						CreateNewDir(ptrOutputDir);
						break;
					}
				}

				if (ptrOutputDir == nullptr)
				{
					printf("couldnt find a place to put converted images\n");
					exit(1);
				}
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-verbose") == 0 ||
			strcmp(a_apstrArgs[iArg], "-v") == 0)
		{
			verboseOutput = true;
		}
		else if (strcmp(a_apstrArgs[iArg], "-mipmaps") == 0 ||
			strcmp(a_apstrArgs[iArg], "-m") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing mipmap number parameter for -mipmaps\n");
				return true;
			}
			else
			{
				unsigned int ui;
				int result = sscanf(a_apstrArgs[iArg], "%u", &ui);
				if (result == 1)
				{
					mipmaps = ui;
				}
				else
				{
					printf("Error: -mipmaps argument needs to be a number\n");
					return true;
				}
			}
		}
		else if (strcmp(a_apstrArgs[iArg], "-mipwrap") == 0 ||
			strcmp(a_apstrArgs[iArg], "-w") == 0)
		{
			++iArg;

			if (iArg >= (a_iArgs))
			{
				printf("Error: missing parameter for -mipwrap\n");
				return true;
			}
			else
			{
				if ( 0 == strcmp(a_apstrArgs[iArg], "x") )
				{
					mipFilterFlags = Etc::FILTER_WRAP_X;
				}
				else if (0 == strcmp(a_apstrArgs[iArg], "y"))
				{
					mipFilterFlags = Etc::FILTER_WRAP_Y;
				}
				else if (0 == strcmp(a_apstrArgs[iArg], "xy"))
				{
					mipFilterFlags = Etc::FILTER_WRAP_X | Etc::FILTER_WRAP_Y;
				}
			}
		}
		else if (a_apstrArgs[iArg][0] == '-')
        {
			printf("Error: unknown option (%s)\n", a_apstrArgs[iArg]);
			return true;
		}
		else if (a_apstrArgs[iArg][0] == '\r')
        {
			continue;
		}
        else
        {
			if (pstrSourceFilename != nullptr)
			{
				printf("Error: only support one source_image (%s)\n", a_apstrArgs[iArg]);
				return true;
			}

			pstrSourceFilename = new char[strlen(a_apstrArgs[iArg])+1];
			strcpy(pstrSourceFilename, a_apstrArgs[iArg]);
        }
    }

	if (pstrSourceFilename == nullptr)
	{
		printf("Error: missing source_image\n");
		return true;
	}

	if (pstrOutputFilename == nullptr)
	{
		printf("Error: missing -output encoded_image\n");
		return true;
	}

	if (uiComparisons > 0 && pstrAnalysisDirectory == nullptr)
	{
		printf("Error: -compare is only valid with -analyze\n");
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------
//
void Commands::FixSlashes(char *a_pstr)
{
	while (*a_pstr)
	{
		if (*a_pstr == ETC_BAD_PATH_SLASH)
		{
			*a_pstr = ETC_PATH_SLASH;
		}

		a_pstr++;
	}
}

// ----------------------------------------------------------------------------------------------------
// print usage message and exit
//
void Commands::PrintUsageMessage(void)
{
	printf("Usage: etctool.exe source_image [options ...] -output <output_file>\n");
	printf("Options:\n");
	printf("    -analyze <analysis_folder>\n");
	printf("    -argfile <arg_file>           additional command line arguments\n");
	printf("    -blockAtHV <H V>              encodes a single block that contains the\n");
	printf("                                  pixel specified by the H V coordinates\n");
	printf("    -compare <comparison_image>   compares source_image to comparison_image\n");
	printf("    -effort <amount>              number between 0 and 100\n");
	printf("    -errormetric <error_metric>   specify the error metric, the options are\n");
	printf("                                  rgba, rgbx, rec709, numeric and normalxyz\n");
	printf("    -format <etc_format>          ETC1, RGB8, SRGB8, RGBA8, SRGB8, RGB8A1,\n");
	printf("                                  SRGB8A1 or R11\n");
	printf("    -help                         prints this message\n");
	printf("    -jobs or -j <thread_count>    specifies the number of threads (default=1)\n");
	printf("    -normalizexyz                 normalize RGB to have a length of 1\n");
	printf("    -verbose or -v                shows status information during the encoding\n");
	printf("                                  process\n");
	printf("    -mipmaps or -m <mip_count>    sets the maximum number of mipaps to generate (default=1)\n");
	printf("    -mipwrap or -w <x|y|xy>       sets the mipmap filter wrap mode (default=clamp)\n");
	printf("\n");

	exit(1);
}

	// ----------------------------------------------------------------------------------------------------
	//
	void CreateNewDir(const char *path)
	{
		char strCommand[300];

#if ETC_WINDOWS
		sprintf_s(strCommand, "if not exist %s %s %s", path, ETC_MKDIR_COMMAND, path);
#else
		sprintf(strCommand, "%s %s", ETC_MKDIR_COMMAND, path);
#endif
		int iResult = system(strCommand);
		if (iResult != 0)
		{
			printf("Error: couldn't create directory (%s)\n", path);
			exit(0);
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
