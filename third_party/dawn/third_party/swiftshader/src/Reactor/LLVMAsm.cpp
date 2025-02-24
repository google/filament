// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "LLVMAsm.hpp"

#ifdef ENABLE_RR_EMIT_ASM_FILE

#	include "Debug.hpp"
#	include "llvm/IR/LegacyPassManager.h"
#	include "llvm/Support/FileSystem.h"
#	include <fstream>
#	include <iomanip>
#	include <regex>
#	include <sstream>

namespace rr {
namespace AsmFile {

std::string generateFilename(const std::string &emitDir, std::string routineName)
{
	// Names from gtests sometimes have invalid file name characters
	std::replace(routineName.begin(), routineName.end(), '/', '_');

	static size_t counter = 0;
	std::stringstream f;
	f << emitDir << "reactor_jit_llvm_" << std::setfill('0') << std::setw(4) << counter++ << "_" << routineName << ".asm";
	return f.str();
}

bool emitAsmFile(const std::string &filename, llvm::orc::JITTargetMachineBuilder builder, llvm::Module &module)
{
	auto targetMachine = builder.createTargetMachine();
	if(!targetMachine)
		return false;

	auto fileType = llvm::CGFT_AssemblyFile;
	std::error_code EC;
	llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
	ASSERT(!EC);
	llvm::legacy::PassManager pm;
	auto &options = targetMachine.get()->Options.MCOptions;
	options.ShowMCEncoding = true;
	options.AsmVerbose = true;
	targetMachine.get()->addPassesToEmitFile(pm, dest, nullptr, fileType);
	pm.run(module);
	return true;
}

void fixupAsmFile(const std::string &filename, std::vector<const void *> addresses)
{
	// Read input asm file into memory so we can overwrite it. This also allows us to merge multiline
	// comments into a single line for easier parsing below.
	std::vector<std::string> lines;
	{
		std::ifstream fin(filename);
		std::string line;
		while(std::getline(fin, line))
		{
			auto firstChar = [&] {
				auto index = line.find_first_not_of(" \t");
				if(index == std::string::npos)
					return '\n';
				return line[index];
			};

			if(!lines.empty() && firstChar() == '#')
			{
				lines.back() += line;
			}
			else
			{
				lines.push_back(line);
			}
		}
	}

	std::ofstream fout(filename);

	// Output function table
	fout << "\nFunction Addresses:\n";
	for(size_t i = 0; i < addresses.size(); i++)
	{
		fout << "f" << i << ": " << addresses[i] << "\n";
	}
	fout << "\n";

	size_t functionIndex = ~0;
	size_t instructionAddress = 0;

	for(auto &line : lines)
	{
		size_t pos{};

		if(line.find("# -- Begin function") != std::string::npos)
		{
			++functionIndex;

			if(functionIndex < addresses.size())
			{
				instructionAddress = (size_t)addresses[functionIndex];
			}
			else
			{
				// For coroutines, more functions are compiled than the top-level three.
				// For now, just output 0-based instructions.
				instructionAddress = 0;
			}
		}

		// Handle alignment directives by aligning the instruction address. When lowered, these actually
		// map to a nops to pad to the next aligned address.
		pos = line.find(".p2align");
		if(pos != std::string::npos)
		{
			// This assumes GNU asm format (https://sourceware.org/binutils/docs/as/P2align.html#P2align)
			static std::regex reAlign(R"(.*\.p2align.*([0-9]+).*)");
			std::smatch matches;
			auto found = std::regex_search(line, matches, reAlign);
			ASSERT(found);
			auto alignPow2 = std::stoi(matches[1]);
			auto align = 1 << alignPow2;
			instructionAddress = (instructionAddress + align - 1) & ~(align - 1);
		}

		// Detect instruction lines and prepend the location (address)
		pos = line.find("encoding: [");
		if(pos != std::string::npos)
		{
			// Determine offset of next instruction (size of this instruction in bytes)
			// e.g. # encoding: [0x48,0x89,0x4c,0x24,0x40]
			// Count number of commas in the array + 1
			auto endPos = line.find("]", pos);
			auto instructionSize = 1 + std::count_if(line.begin() + pos, line.begin() + endPos, [](char c) { return c == ','; });

			// Prepend current location to instruction line
			std::stringstream location;
			location << "[0x" << std::uppercase << std::hex << instructionAddress << "] ";
			line = location.str() + line;

			instructionAddress += instructionSize;
		}

		fout << line + "\n";
	}
}

}  // namespace AsmFile
}  // namespace rr

#endif  // ENABLE_RR_EMIT_ASM_FILE
