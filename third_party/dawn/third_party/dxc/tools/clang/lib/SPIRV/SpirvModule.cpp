//===--- SpirvModule.cpp - SPIR-V Module Implementation ----------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/SpirvModule.h"
#include "clang/SPIRV/SpirvFunction.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

SpirvModule::SpirvModule()
    : capabilities({}), extensions({}), extInstSets({}), memoryModel(nullptr),
      entryPoints({}), executionModes({}), moduleProcesses({}), decorations({}),
      constants({}), undefs({}), variables({}), functions({}),
      debugInstructions({}), perVertexInterp(false) {}

SpirvModule::~SpirvModule() {
  for (auto *cap : capabilities)
    cap->releaseMemory();
  for (auto *ext : extensions)
    ext->releaseMemory();
  for (auto *set : extInstSets)
    set->releaseMemory();
  if (memoryModel)
    memoryModel->releaseMemory();
  for (auto *entry : entryPoints)
    entry->releaseMemory();
  for (auto *exec : executionModes)
    exec->releaseMemory();
  for (auto *str : constStrings)
    str->releaseMemory();
  for (auto *d : sources)
    d->releaseMemory();
  for (auto *mp : moduleProcesses)
    mp->releaseMemory();
  for (auto *decoration : decorations)
    decoration->releaseMemory();
  for (auto *constant : constants)
    constant->releaseMemory();
  for (auto *undef : undefs)
    undef->releaseMemory();
  for (auto *var : variables)
    var->releaseMemory();
  for (auto *di : debugInstructions)
    di->releaseMemory();
  for (auto *f : allFunctions)
    f->~SpirvFunction();
}

bool SpirvModule::invokeVisitor(Visitor *visitor, bool reverseOrder) {
  // Note: It is debatable whether reverse order of visiting the module should
  // reverse everything in this method. For the time being, we just reverse the
  // order of the function visitors, and keeping everything else the same.
  // For example, it is not clear what the value would be of vising the last
  // function first. We can update this methodology if needed.

  if (!visitor->visit(this, Visitor::Phase::Init))
    return false;

  if (reverseOrder) {
    // Reverse order of a SPIR-V module.

    // Our transformations do not cross function bounaries, therefore the order
    // of visiting functions is not important.
    for (auto iter = functions.rbegin(); iter != functions.rend(); ++iter) {
      auto *fn = *iter;
      if (!fn->invokeVisitor(visitor, reverseOrder))
        return false;
    }

    for (auto iter = debugInstructions.rbegin();
         iter != debugInstructions.rend(); ++iter) {
      auto *debugInstruction = *iter;
      if (!debugInstruction->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = variables.rbegin(); iter != variables.rend(); ++iter) {
      auto *var = *iter;
      if (!var->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = constants.rbegin(); iter != constants.rend(); ++iter) {
      auto *constant = *iter;
      if (!constant->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = undefs.rbegin(); iter != undefs.rend(); ++iter) {
      auto *undef = *iter;
      if (!undef->invokeVisitor(visitor))
        return false;
    }

    // Since SetVector doesn't have 'rbegin()' and 'rend()' methods, we use
    // manual indexing.
    for (auto decorIndex = decorations.size(); decorIndex > 0; --decorIndex) {
      auto *decoration = decorations[decorIndex - 1];
      if (!decoration->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = moduleProcesses.rbegin(); iter != moduleProcesses.rend();
         ++iter) {
      auto *moduleProcess = *iter;
      if (!moduleProcess->invokeVisitor(visitor))
        return false;
    }

    if (!sources.empty())
      for (auto iter = sources.rbegin(); iter != sources.rend(); ++iter) {
        auto *source = *iter;
        if (!source->invokeVisitor(visitor))
          return false;
      }

    for (auto iter = constStrings.rbegin(); iter != constStrings.rend();
         ++iter) {
      if (!(*iter)->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = executionModes.rbegin(); iter != executionModes.rend();
         ++iter) {
      auto *execMode = *iter;
      if (!execMode->invokeVisitor(visitor))
        return false;
    }

    for (auto iter = entryPoints.rbegin(); iter != entryPoints.rend(); ++iter) {
      auto *entryPoint = *iter;
      if (!entryPoint->invokeVisitor(visitor))
        return false;
    }

    if (!memoryModel->invokeVisitor(visitor))
      return false;

    for (auto iter = extInstSets.rbegin(); iter != extInstSets.rend(); ++iter) {
      auto *extInstSet = *iter;
      if (!extInstSet->invokeVisitor(visitor))
        return false;
    }

    // Since SetVector doesn't have 'rbegin()' and 'rend()' methods, we use
    // manual indexing.
    for (auto extIndex = extensions.size(); extIndex > 0; --extIndex) {
      auto *extension = extensions[extIndex - 1];
      if (!extension->invokeVisitor(visitor))
        return false;
    }

    // Since SetVector doesn't have 'rbegin()' and 'rend()' methods, we use
    // manual indexing.
    for (auto capIndex = capabilities.size(); capIndex > 0; --capIndex) {
      auto *capability = capabilities[capIndex - 1];
      if (!capability->invokeVisitor(visitor))
        return false;
    }
  }
  // Traverse the regular order of a SPIR-V module.
  else {
    for (auto *cap : capabilities)
      if (!cap->invokeVisitor(visitor))
        return false;

    for (auto ext : extensions)
      if (!ext->invokeVisitor(visitor))
        return false;

    for (auto extInstSet : extInstSets)
      if (!extInstSet->invokeVisitor(visitor))
        return false;

    if (!memoryModel->invokeVisitor(visitor))
      return false;

    for (auto entryPoint : entryPoints)
      if (!entryPoint->invokeVisitor(visitor))
        return false;

    for (auto execMode : executionModes)
      if (!execMode->invokeVisitor(visitor))
        return false;

    for (auto *str : constStrings)
      if (!str->invokeVisitor(visitor))
        return false;

    if (!sources.empty())
      for (auto *source : sources)
        if (!source->invokeVisitor(visitor))
          return false;

    for (auto moduleProcess : moduleProcesses)
      if (!moduleProcess->invokeVisitor(visitor))
        return false;

    for (auto decoration : decorations)
      if (!decoration->invokeVisitor(visitor))
        return false;

    for (auto constant : constants)
      if (!constant->invokeVisitor(visitor))
        return false;

    for (auto undef : undefs)
      if (!undef->invokeVisitor(visitor))
        return false;

    for (auto var : variables)
      if (!var->invokeVisitor(visitor))
        return false;

    for (size_t i = 0; i < debugInstructions.size(); i++)
      if (!debugInstructions[i]->invokeVisitor(visitor))
        return false;

    for (auto fn : functions)
      if (!fn->invokeVisitor(visitor, reverseOrder))
        return false;
  }

  if (!visitor->visit(this, Visitor::Phase::Done))
    return false;

  return true;
}

void SpirvModule::addFunctionToListOfSortedModuleFunctions(SpirvFunction *fn) {
  assert(fn && "cannot add null function to the module");
  functions.push_back(fn);
}

void SpirvModule::addFunction(SpirvFunction *fn) {
  assert(fn && "cannot add null function to the module");
  allFunctions.insert(fn);
}

bool SpirvModule::addCapability(SpirvCapability *cap) {
  assert(cap && "cannot add null capability to the module");
  return capabilities.insert(cap);
}

bool SpirvModule::hasCapability(SpirvCapability &cap) {
  return capabilities.count(&cap) != 0;
}

void SpirvModule::setMemoryModel(SpirvMemoryModel *model) {
  assert(model && "cannot set a null memory model");
  if (memoryModel)
    memoryModel->releaseMemory();
  memoryModel = model;
}

bool SpirvModule::promoteAddressingModel(spv::AddressingModel addrModel) {
  assert(memoryModel && "base memory model must be set first");
  auto getPriority = [](spv::AddressingModel am) -> int {
    switch (am) {
    default:
      assert(false && "unknown addressing model");
      return 0;
    case spv::AddressingModel::Logical:
      return 0;
    case spv::AddressingModel::Physical32:
      return 1;
    case spv::AddressingModel::Physical64:
      return 2;
    case spv::AddressingModel::PhysicalStorageBuffer64:
      return 3;
    }
  };

  int current = getPriority(memoryModel->getAddressingModel());
  int pending = getPriority(addrModel);

  if (pending > current) {
    memoryModel->setAddressingModel(addrModel);
    return true;
  } else {
    return false;
  }
}

void SpirvModule::addEntryPoint(SpirvEntryPoint *ep) {
  assert(ep && "cannot add null as an entry point");
  entryPoints.push_back(ep);
}

SpirvExecutionModeBase *
SpirvModule::findExecutionMode(SpirvFunction *entryPoint,
                               spv::ExecutionMode em) {
  for (SpirvExecutionModeBase *cem : executionModes) {
    if (cem->getEntryPoint() != entryPoint)
      continue;
    if (cem->getExecutionMode() != em)
      continue;
    return cem;
  }
  return nullptr;
}

void SpirvModule::addExecutionMode(SpirvExecutionModeBase *em) {
  assert(em && "cannot add null execution mode");
  executionModes.push_back(em);
}

bool SpirvModule::addExtension(SpirvExtension *ext) {
  assert(ext && "cannot add null extension");
  return extensions.insert(ext);
}

void SpirvModule::addExtInstSet(SpirvExtInstImport *set) {
  assert(set && "cannot add null extended instruction set");
  extInstSets.push_back(set);
}

SpirvExtInstImport *SpirvModule::getExtInstSet(llvm::StringRef name) {
  // We expect very few (usually 1) extended instruction sets to exist in the
  // module, so this is not expensive.
  auto found = std::find_if(extInstSets.begin(), extInstSets.end(),
                            [name](const SpirvExtInstImport *set) {
                              return set->getExtendedInstSetName() == name;
                            });

  if (found != extInstSets.end())
    return *found;

  return nullptr;
}

void SpirvModule::addVariable(SpirvVariable *var) {
  assert(var && "cannot add null variable to the module");
  variables.push_back(var);
}

void SpirvModule::addVariable(SpirvVariable *var, SpirvInstruction *pos) {
  assert(var && "cannot add null variable to the module");
  auto location = std::find(variables.begin(), variables.end(), pos);
  variables.insert(location, var);
}

void SpirvModule::addDecoration(SpirvDecoration *decor) {
  assert(decor && "cannot add null decoration to the module");
  decorations.insert(decor);
}

void SpirvModule::addConstant(SpirvConstant *constant) {
  assert(constant);
  constants.push_back(constant);
}

void SpirvModule::addUndef(SpirvUndef *undef) {
  assert(undef);
  undefs.push_back(undef);
}

void SpirvModule::addString(SpirvString *str) {
  assert(str);
  constStrings.push_back(str);
}

void SpirvModule::addSource(SpirvSource *src) {
  assert(src);
  sources.push_back(src);
}

void SpirvModule::addDebugInfo(SpirvDebugInstruction *info) {
  assert(info);
  debugInstructions.push_back(info);
}

void SpirvModule::addModuleProcessed(SpirvModuleProcessed *p) {
  assert(p);
  moduleProcesses.push_back(p);
}

SpirvDebugCompilationUnit *SpirvModule::getDebugCompilationUnit() {
  SpirvDebugCompilationUnit *unit = debugCompilationUnit;
  assert(unit && "null DebugCompilationUnit");
  return unit;
}

void SpirvModule::setDebugCompilationUnit(SpirvDebugCompilationUnit *unit) {
  assert(unit);
  debugCompilationUnit = unit;
}

} // end namespace spirv
} // end namespace clang
