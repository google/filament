// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_LLVMReactorDebugInfo_hpp
#define rr_LLVMReactorDebugInfo_hpp

#include "Reactor.hpp"
#include "ReactorDebugInfo.hpp"

#ifdef ENABLE_RR_DEBUG_INFO

#	include <memory>
#	include <unordered_map>
#	include <unordered_set>
#	include <vector>

// Forward declarations
namespace llvm {

class BasicBlock;
class ConstantFolder;
class DIBuilder;
class DICompileUnit;
class DIFile;
class DILocation;
class DIScope;
class DISubprogram;
class DIType;
class Function;
class Instruction;
class IRBuilderDefaultInserter;
class JITEventListener;
class LLVMContext;
class LoadedObjectInfo;
class Module;
class Type;
class Value;

namespace object {
class ObjectFile;
}

template<typename T, typename Inserter>
class IRBuilder;

}  // namespace llvm

namespace rr {

class Type;
class Value;

// DebugInfo generates LLVM DebugInfo IR from the C++ source that calls
// into Reactor functions. See docs/ReactorDebugInfo.mk for more information.
class DebugInfo
{
public:
	using IRBuilder = llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>;

	DebugInfo(IRBuilder *builder,
	          llvm::LLVMContext *context,
	          llvm::Module *module,
	          llvm::Function *function);

	~DebugInfo();

	// Finalize debug info generation. Must be called before the LLVM module
	// is built.
	void Finalize();

	// Updates the current source location.
	void EmitLocation();

	// Binds the value to its symbol in the source file.
	// See docs/ReactorDebugInfo.mk for more information.
	void EmitVariable(Value *value);

	// Forcefully flush the binding of the last variable name.
	// Used for binding the initializer of `For` loops.
	void Flush();

	// NotifyObjectEmitted informs any attached debuggers of the JIT'd
	// object.
	static void NotifyObjectEmitted(uint64_t key, const llvm::object::ObjectFile &obj, const llvm::LoadedObjectInfo &l);

	// NotifyFreeingObject informs any attached debuggers that the JIT'd
	// object is now invalid.
	static void NotifyFreeingObject(uint64_t key);

private:
	struct Token
	{
		enum Kind
		{
			Identifier,
			Return
		};
		Kind kind;
		std::string identifier;
	};

	using LineTokens = std::unordered_map<unsigned int, Token>;

	struct Pending
	{
		std::string name;
		Location location;
		llvm::DILocation *diLocation = nullptr;
		llvm::Value *value = nullptr;
		llvm::Instruction *insertAfter = nullptr;
		llvm::BasicBlock *block = nullptr;
		llvm::DIScope *scope = nullptr;
		bool addNopOnNextLine = false;
	};

	struct Scope
	{
		Location location;
		llvm::DIScope *di;
		std::unordered_set<std::string> symbols;
		Pending pending;
	};

	void registerBasicTypes();

	void emitPending(Scope &scope, IRBuilder *builder);

	// Returns the source location of the non-Reactor calling function.
	Location getCallerLocation() const;

	// Returns the backtrace for the callstack, starting at the first
	// non-Reactor file. If limit is non-zero, then a maximum of limit
	// frames will be returned.
	Backtrace getCallerBacktrace(size_t limit = 0) const;

	llvm::DILocation *getLocation(const Backtrace &backtrace, size_t i);

	llvm::DIType *getOrCreateType(llvm::Type *type);
	llvm::DIFile *getOrCreateFile(const char *path);
	const LineTokens *getOrParseFileTokens(const char *path);

	// Synchronizes diScope with the current backtrace.
	void syncScope(const Backtrace &backtrace);

	IRBuilder *builder;
	llvm::LLVMContext *context;
	llvm::Module *module;
	llvm::Function *function;

	std::unique_ptr<llvm::DIBuilder> diBuilder;
	llvm::DICompileUnit *diCU;
	llvm::DISubprogram *diSubprogram;
	llvm::DILocation *diRootLocation;
	std::vector<Scope> diScope;
	std::unordered_map<std::string, llvm::DIFile *> diFiles;
	std::unordered_map<llvm::Type *, llvm::DIType *> diTypes;
	std::unordered_map<std::string, std::unique_ptr<LineTokens>> fileTokens;
	std::vector<const void *> pushed;
};

}  // namespace rr

#endif  // ENABLE_RR_DEBUG_INFO

#endif  // rr_LLVMReactorDebugInfo_hpp
