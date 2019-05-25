// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <iostream>
#include <cassert>
#include <cmath>
#include <signal.h>

#ifdef VTUNE
#include <ittnotify.h>
#endif

#include <OpenImageDenoise/oidn.hpp>

#include "common/timer.h"
#include "image_io.h"
#include "cli.h"

using namespace oidn;

void printUsage()
{
  std::cout << "Open Image Denoise Example" << std::endl;
  std::cout << "Usage: denoise [-ldr ldr_color.pfm] [-srgb] [-hdr hdr_color.pfm]" << std::endl
            << "               [-alb albedo.pfm] [-nrm normal.pfm]" << std::endl
            << "               [-o output.pfm] [-ref reference_output.pfm]" << std::endl
            << "               [-bench ntimes] [-threads n] [-affinity 0|1]" << std::endl;
}

void errorCallback(void* userPtr, oidn::Error error, const char* message)
{
  throw std::runtime_error(message);
}

volatile bool isCancelled = false;

void signalHandler(int signal)
{
  isCancelled = true;
}

bool progressCallback(void* userPtr, double n)
{
  if (isCancelled)
    return false;
  std::cout << "\rDenoising " << int(n * 100.) << "%" << std::flush;
  return true;
}

int main(int argc, char* argv[])
{
  std::string colorFilename, albedoFilename, normalFilename;
  std::string outputFilename, refFilename;
  bool hdr = false;
  bool srgb = false;
  int numBenchmarkRuns = 0;
  int numThreads = -1;
  int setAffinity = -1;

  // Parse the arguments
  if (argc == 1)
  {
    printUsage();
    return 1;
  }

  try
  {
    ArgParser args(argc, argv);
    while (args.hasNext())
    {
      std::string opt = args.getNextOpt();
      if (opt == "ldr")
      {
        colorFilename = args.getNextValue();
        hdr = false;
      }
      else if (opt == "hdr")
      {
        colorFilename = args.getNextValue();
        hdr = true;
      }
      else if (opt == "srgb")
        srgb = true;
      else if (opt == "alb" || opt == "albedo")
        albedoFilename = args.getNextValue();
      else if (opt == "nrm" || opt == "normal")
        normalFilename = args.getNextValue();
      else if (opt == "o" || opt == "out" || opt == "output")
        outputFilename = args.getNextValue();
      else if (opt == "ref" || opt == "reference")
        refFilename = args.getNextValue();
      else if (opt == "bench" || opt == "benchmark")
        numBenchmarkRuns = std::max(args.getNextValueInt(), 0);
      else if (opt == "threads")
        numThreads = args.getNextValueInt();
      else if (opt == "affinity")
        setAffinity = args.getNextValueInt();
      else if (opt == "h" || opt == "help")
      {
        printUsage();
        return 1;
      }
      else
        throw std::invalid_argument("invalid argument");
    }

    if (colorFilename.empty())
      throw std::runtime_error("no color image specified");

    // Load the input image
    ImageBuffer color, albedo, normal;
    ImageBuffer ref;

    std::cout << "Loading input" << std::flush;

    color = loadImage(colorFilename);
    if (color.getChannels() != 3)
      throw std::runtime_error("invalid color image");

    if (!albedoFilename.empty())
    {
      albedo = loadImage(albedoFilename);
      if (albedo.getChannels() != 3 || albedo.getSize() != color.getSize())
        throw std::runtime_error("invalid albedo image");
    }

    if (!normalFilename.empty())
    {
      normal = loadImage(normalFilename);
      if (normal.getChannels() != 3 || normal.getSize() != color.getSize())
        throw std::runtime_error("invalid normal image");
    }

    if (!refFilename.empty())
    {
      ref = loadImage(refFilename);
      if (ref.getChannels() != 3 || ref.getSize() != color.getSize())
        throw std::runtime_error("invalid reference output image");
    }

    const int width  = color.getWidth();
    const int height = color.getHeight();
    std::cout << std::endl << "Resolution: " << width << "x" << height << std::endl;

    // Initialize the output image
    ImageBuffer output(width, height, 3);

    // Initialize the denoising filter
    std::cout << "Initializing" << std::flush;
    Timer timer;

    oidn::DeviceRef device = oidn::newDevice();

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None)
      throw std::runtime_error(errorMessage);
    device.setErrorFunction(errorCallback);

    if (numThreads > 0)
      device.set("numThreads", numThreads);
    if (setAffinity >= 0)
      device.set("setAffinity", bool(setAffinity));
    device.commit();

    oidn::FilterRef filter = device.newFilter("RT");

    filter.setImage("color", color.getData(), oidn::Format::Float3, width, height);
    if (albedo)
      filter.setImage("albedo", albedo.getData(), oidn::Format::Float3, width, height);
    if (normal)
      filter.setImage("normal", normal.getData(), oidn::Format::Float3, width, height);
    filter.setImage("output", output.getData(), oidn::Format::Float3, width, height);

    if (hdr)
      filter.set("hdr", true);
    if (srgb)
      filter.set("srgb", true);

    filter.setProgressMonitorFunction(progressCallback);
    signal(SIGINT, signalHandler);

    filter.commit();

    const double initTime = timer.query();

    const int versionMajor = device.get<int>("versionMajor");
    const int versionMinor = device.get<int>("versionMinor");
    const int versionPatch = device.get<int>("versionPatch");

    std::cout << ": version=" << versionMajor << "." << versionMinor << "." << versionPatch
         << ", msec=" << (1000. * initTime) << std::endl;

    // Denoise the image
    //std::cout << "Denoising" << std::flush;
    timer.reset();

    filter.execute();

    const double denoiseTime = timer.query();
    std::cout << ": msec=" << (1000. * denoiseTime) << std::endl;

    filter.setProgressMonitorFunction(nullptr);
    signal(SIGINT, SIG_DFL);

    if (ref)
    {
      // Verify the output values
      int nerr = 0;
      float maxre = 0;
      for (size_t i = 0; i < output.getDataSize(); ++i)
      {
        float expect = std::max(ref[i], 0.f);
        if (!hdr)
          expect = std::min(expect, 1.f);
        const float actual = output[i];
        float re;
        if (std::abs(expect) < 1e-5 && std::abs(actual) < 1e-5)
          re = 0;
        else if (expect != 0)
          re = std::abs((expect - actual) / expect);
        else
          re = std::abs(expect - actual);
        if (maxre < re) maxre = re;
        if (re > 1e-3)
        {
          //std::cout << "i=" << i << " expect=" << expect << " actual=" << actual << std::endl;
          ++nerr;
        }
      }
      std::cout << "Verified output: nfloats=" << output.getDataSize() << ", nerr=" << nerr << ", maxre=" << maxre << std::endl;

      // Save debug images
      std::cout << "Saving debug images" << std::flush;
      saveImage("denoise_in.ppm",  color);
      saveImage("denoise_out.ppm", output);
      saveImage("denoise_ref.ppm", ref);
      std::cout << std::endl;
    }

    if (!outputFilename.empty())
    {
      // Save output image
      std::cout << "Saving output" << std::flush;
      saveImage(outputFilename, output);
      std::cout << std::endl;
    }

    if (numBenchmarkRuns > 0)
    {
      // Benchmark loop
    #ifdef VTUNE
      __itt_resume();
    #endif

      std::cout << "Benchmarking: " << "ntimes=" << numBenchmarkRuns << std::flush;
      timer.reset();

      for (int i = 0; i < numBenchmarkRuns; ++i)
        filter.execute();

      const double totalTime = timer.query();
      std::cout << ", sec=" << totalTime << ", msec/image=" << (1000.*totalTime / numBenchmarkRuns) << std::endl;

    #ifdef VTUNE
      __itt_pause();
    #endif
    }
  }
  catch (std::exception& e)
  {
    std::cout << std::endl << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
