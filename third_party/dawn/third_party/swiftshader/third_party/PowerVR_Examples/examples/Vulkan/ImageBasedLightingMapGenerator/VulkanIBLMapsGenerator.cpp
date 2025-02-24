/*!
\brief A command line tool using Headless (i.e. surface less) Vulkan Graphics to calculate the Irradiance map (
	   used for Diffuse global illumination), and the Pre-filtered Reflection map (used for Specular global
	   illumination).
\file  VulkanIBLMapsGenerator.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRCore/PVRCore.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/PBRUtilsVk.h"
#include <iostream>

/// <summary>Print the supported command line parameters to standard out.</summary>
void printHelp()
{
	std::cout << "  Options:\n" << std::endl;
	std::cout << "     -diffuseSize=[NUMBER]         The size of the output Irradiance map." << std::endl;
	std::cout << "                                   Default: 64 (for a 64x64 map)      Min: 1" << std::endl << std::endl;
	std::cout << "     -diffuseSamples=[NUMBER]      The number of sampling points to use when generating the Irradiance map." << std::endl;
	std::cout << "                                   Default: 10000                      Min: 10" << std::endl;
	std::cout << "                                   Recommended values : Low Thousands. Use higher values for environment maps with high frequency components." << std::endl
			  << std::endl;
	std::cout << "     -skipDiffuse                  Do not create a Diffuse Irradiance map" << std::endl << std::endl;
	std::cout << "     -skipSpecular                 Do not create a specular irradiance map." << std::endl << std::endl;
	std::cout << "     -specularSize=[NUMBER]        The size of the generated Pre-filtered Reflection map. Default: 256 (for a 256x256 map)" << std::endl << std::endl;
	std::cout << "     -specularSamples=[NUMBER]     The number of sampling points to use when generating the Pre-filtered Reflection map." << std::endl;
	std::cout << "                                   Default: 10000                      Min: 1" << std::endl;
	std::cout << "                                   Recommended values : Low-Mid Thousands. Use higher values for environment maps with high frequency components." << std::endl
			  << std::endl;
	std::cout << "     -specularDiscardMips=[NUMBER] The number of specular maps to not generate (i.e. indirectly, the size of the smallest map to generate).points to use when "
				 "generating the Pre-filtered Reflection map."
			  << std::endl;
	std::cout << "                                   Default: 2                          Min: 0    Max: The base-2 logarithm of the size." << std::endl;
	std::cout << "                                   Recommended values : 1-3. If the lowest specular maps are kept, rough but mildly curved surfaces will be suffering by very "
				 "strong artefacts. If a lot of maps are discarded, smoother surfaces will be incorrect due to not having enough resolution between the low roughness mipmaps."
			  << std::endl
			  << std::endl;
	std::cout << "  CAUTION: With the number of samples, more is better but up to a point: Very high value will eventually sharply drop in accuracy due to floating point "
				 "limitations, providing incorrect results."
			  << std::endl
			  << std::endl;
}

int main(int argc, char** argv)
{
	using std::cout;
	using std::cerr;
	try
	{
		pvr::platform::CommandLineParser parser(argc - 1, argv + 1);
		const pvr::CommandLine& cmdLine = parser.getParsedCommandLine();

		cout << "\nImage Based Lighting map calculator. Usage: " << argv[0] << " [input cubemap] [options]\n";

		if (argc == 1)
		{
			printHelp();
			return 1;
		}
		if (cmdLine.hasOption("-h") || cmdLine.hasOption("--help"))
		{
			printHelp();
			exit(0);
		}
		int32_t numSamplesDiffuse = 10000;
		int32_t numSamplesSpecular = 10000;
		int32_t mapSizeDiffuse = 64;
		int32_t mapSizeSpecular = 256;
		int32_t specularDiscardMips = 2;
		bool skipDiffuse = false;
		bool skipSpecular = false;

		cmdLine.getIntOption("-diffuseSamples", numSamplesDiffuse);
		cmdLine.getIntOption("-specularSamples", numSamplesSpecular);
		cmdLine.getIntOption("-diffuseSize", mapSizeDiffuse);
		cmdLine.getIntOption("-specularSize", mapSizeSpecular);
		cmdLine.getIntOption("-specularDiscardMips", specularDiscardMips);
		cmdLine.getBoolOptionSetTrueIfPresent("-skipDiffuse", skipDiffuse);
		cmdLine.getBoolOptionSetTrueIfPresent("-skipSpecular", skipSpecular);

		if (numSamplesDiffuse < 10)
		{
			std::cout << "-diffuseSamples cannot be less than 10";
			return 1;
		}
		if (numSamplesSpecular < 10)
		{
			std::cout << "-specularSamples cannot be less than 10";
			return 1;
		}
		if (mapSizeDiffuse < 1)
		{
			std::cout << "-mapSizeDiffuse cannot be less than 1";
			return 1;
		}

		if (mapSizeSpecular < pow(2, specularDiscardMips))
		{
			std::cout << "-mapSizeSpecular and -specularDiscardMips set incorrectly: Attempting to discard " << specularDiscardMips << " maps while only "
					  << int(log2(mapSizeSpecular)) << " mipmaps exist for map size " << mapSizeSpecular << std::endl;
			return 1;
		}

		std::cout << "Running with parameters: \n"
				  << " Diffuse  map - Size: " << mapSizeDiffuse << "x" << mapSizeDiffuse << "  - Number of Samples: " << numSamplesDiffuse << "\n"
				  << " Specular map - Size: " << mapSizeSpecular << "x" << mapSizeSpecular << "  - Number of Samples: " << numSamplesSpecular << "\n";

		std::unique_ptr<pvr::Stream> inputFile;
		pvr::Texture inputTexture;

		std::string inputName = argv[1];

		try
		{
			inputTexture = pvr::textureLoad(*pvr::FileStream::createFileStream(inputName, "rb"));
		}
		catch (pvr::FileNotFoundError&)
		{
			cerr << "Input file [" << inputName << "] not found.\n";
			return 1;
		}
		catch (pvr::InvalidArgumentError&)
		{
			cerr << "Error: Input image file type not recognized.\n";
			return 1;
		}
		catch (std::runtime_error& err)
		{
			cerr << "Error: " << err.what() << "\n";
			return 1;
		}

		cout << "Setting up Vulkan headless context\n";
		// Create the instance and retrieve the physical device
		pvrvk::Instance instance = pvr::utils::createInstance("IBLMapsGenerator");

		// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(instance);

		pvrvk::PhysicalDevice physicalDevice = instance->getPhysicalDevice(0);

		// Create a Logical device and retrieve a queue supporting graphics
		const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT };
		pvr::utils::QueueAccessInfo queueAccessInfo;
		pvrvk::Device device = pvr::utils::createDeviceAndQueues(physicalDevice, &queuePopulateInfo, 1, &queueAccessInfo);
		pvrvk::Queue queue = device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

		// Create a Command Pool used by the utility functions
		pvrvk::CommandPool pool = device->createCommandPool(pvrvk::CommandPoolCreateInfo(queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

		cout << "Uploading input file\n";

		// Upload the source environment map
		pvrvk::ImageView environmentMap = pvr::utils::uploadImageAndViewSubmit(device, inputTexture, true, pool, queue);
		queue->waitIdle();
		device->waitIdle();

		std::string outputName = pvr::FilePath(inputName).getFilenameNoExtension();
		std::string outputNameIrradiance = outputName + "_Irradiance.pvr";
		std::string outputNamePrefiltered = outputName + "_Prefiltered.pvr";

		pvr::Texture irradianceTexture;
		pvr::Texture preFilteredTexture;

		if (!skipDiffuse)
		{
			cout << "Generating irradiance map [" << outputNameIrradiance << "]...";
			irradianceTexture = pvr::utils::generateIrradianceMap(
				queue, environmentMap, pvr::PixelFormat::RGBA_16161616(), pvr::VariableType::SignedFloat, mapSizeDiffuse, uint32_t(sqrtf(float(numSamplesDiffuse))));
			cout << "DONE!\n";
			pvr::assetWriters::writePVR(irradianceTexture, pvr::FileStream(outputNameIrradiance, "wb"));
		}
		if (!skipSpecular)
		{
			cout << "Generating pre-filtered reflection map [" << outputNamePrefiltered << "]..." << std::flush;
			preFilteredTexture = pvr::utils::generatePreFilteredMapMipmapStyle(
				queue, environmentMap, pvr::PixelFormat::RGBA_16161616(), pvr::VariableType::SignedFloat, mapSizeSpecular, true, 2, numSamplesSpecular);
			cout << "DONE!\n";
			pvr::assetWriters::writePVR(preFilteredTexture, pvr::FileStream(outputNamePrefiltered, "wb"));
		}
	}
	catch (std::runtime_error& err)
	{
		cout << "Error encountered: " << err.what();
	}
	catch (std::exception& err)
	{
		cout << "Error encountered: " << err.what();
	}

	cout << "\n";
	return 0;
}
