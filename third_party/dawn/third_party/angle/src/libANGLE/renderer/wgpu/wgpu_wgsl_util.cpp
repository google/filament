//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_wgsl_util.h"

#include <sstream>

#include "common/PackedEnums.h"
#include "common/PackedGLEnums_autogen.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"

namespace rx
{
namespace webgpu
{

namespace
{
const bool kOutputReplacements = false;

std::string WgslReplaceLocationMarkers(const std::string &shaderSource,
                                       std::map<std::string, int> varNameToLocation)
{
    const char *marker    = "@location(@@@@@@) ";
    const char *endOfName = " : ";

    std::string newSource;
    newSource.reserve(shaderSource.size());

    size_t currPos = 0;
    while (true)
    {
        size_t nextMarker = shaderSource.find(marker, currPos, strlen(marker));
        if (nextMarker == std::string::npos)
        {
            // Copy the rest of the shader and end the loop.
            newSource.append(shaderSource, currPos);
            break;
        }
        else
        {
            // Copy up to the next marker
            newSource.append(shaderSource, currPos, nextMarker - currPos);

            // Extract name from something like `@location(@@@@@@) NAME : TYPE`.
            size_t startOfNamePos = nextMarker + strlen(marker);
            size_t endOfNamePos   = shaderSource.find(endOfName, startOfNamePos, strlen(endOfName));
            std::string name(shaderSource.c_str() + startOfNamePos, endOfNamePos - startOfNamePos);

            // Use the shader variable's name to get the assigned location
            auto locationIter = varNameToLocation.find(name);
            if (locationIter == varNameToLocation.end())
            {
                ASSERT(false);
                return "";
            }

            // TODO(anglebug.com/42267100): if the GLSL input is a matrix there should be multiple
            // WGSL input variables (multiple vectors representing the columns of the matrix).
            int location = locationIter->second;
            std::ostringstream locationReplacementStream;
            locationReplacementStream << "@location(" << location << ") " << name;

            if (kOutputReplacements)
            {
                std::cout << "Replace \"" << marker << name << "\" with \""
                          << locationReplacementStream.str() << "\"" << std::endl;
            }

            // Append the new `@location(N) name` and then continue from the ` : type`.
            newSource.append(locationReplacementStream.str());
            currPos = endOfNamePos;
        }
    }
    return newSource;
}

}  // namespace

template <typename T>
std::string WgslAssignLocations(const std::string &shaderSource,
                                const std::vector<T> shaderVars,
                                const gl::ProgramMergedVaryings &mergedVaryings,
                                gl::ShaderType shaderType)
{
    std::map<std::string, int> varNameToLocation;
    for (const T &shaderVar : shaderVars)
    {
        if (shaderVar.isBuiltIn())
        {
            continue;
        }
        varNameToLocation[shaderVar.name] = shaderVar.getLocation();
    }

    int currLocMarker = 0;
    for (const gl::ProgramVaryingRef &linkedVarying : mergedVaryings)
    {
        gl::ShaderBitSet supportedShaderStages =
            gl::ShaderBitSet({gl::ShaderType::Vertex, gl::ShaderType::Fragment});
        ASSERT(linkedVarying.frontShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.frontShaderStage));
        ASSERT(linkedVarying.backShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.backShaderStage));
        if (!linkedVarying.frontShader && !linkedVarying.backShader)
        {
            continue;
        }
        const sh::ShaderVariable *shaderVar = shaderType == gl::ShaderType::Vertex
                                                  ? linkedVarying.frontShader
                                                  : linkedVarying.backShader;
        if (shaderVar)
        {
            if (shaderVar->isBuiltIn())
            {
                continue;
            }
            ASSERT(varNameToLocation.find(shaderVar->name) == varNameToLocation.end());
            varNameToLocation[shaderVar->name] = currLocMarker++;
        }
        else
        {
            const sh::ShaderVariable *otherShaderVar = shaderType == gl::ShaderType::Vertex
                                                           ? linkedVarying.backShader
                                                           : linkedVarying.frontShader;
            if (!otherShaderVar->isBuiltIn())
            {
                // Increment `currLockMarker` to keep locations in sync with the WGSL source
                // generated for the other shader stage, which will also have incremented
                // `currLocMarker` when seeing this variable.
                currLocMarker++;
            }
        }
    }

    return WgslReplaceLocationMarkers(shaderSource, varNameToLocation);
}

template std::string WgslAssignLocations<gl::ProgramInput>(
    const std::string &shaderSource,
    const std::vector<gl::ProgramInput> shaderVars,
    const gl::ProgramMergedVaryings &mergedVaryings,
    gl::ShaderType shaderType);

template std::string WgslAssignLocations<gl::ProgramOutput>(
    const std::string &shaderSource,
    const std::vector<gl::ProgramOutput> shaderVars,
    const gl::ProgramMergedVaryings &mergedVaryings,
    gl::ShaderType shaderType);

}  // namespace webgpu
}  // namespace rx
