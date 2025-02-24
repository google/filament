//===--- SignaturePackingUtil.cpp - Util functions impl ----------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SignaturePackingUtil.h"

#include <algorithm>

namespace clang {
namespace spirv {

namespace {

const uint32_t kNumComponentsInFullyUsedLocation = 4;

/// A class for managing stage input/output packed locations to avoid duplicate
/// uses of the same location and component.
class PackedLocationAndComponentSet {
public:
  PackedLocationAndComponentSet(SpirvBuilder &spirvBuilder,
                                llvm::function_ref<uint32_t(uint32_t)> nextLocs)
      : spvBuilder(spirvBuilder), assignLocs(nextLocs) {}

  bool assignLocAndComponent(const StageVar *var) {
    if (tryReuseLocations(var)) {
      return true;
    }
    return assignNewLocations(var);
  }

private:
  // When the stage variable |var| needs M components in N locations, checks
  // whether used N continuous locations have M unused slots or not. If there
  // are N continuous locations that have M unused slots, uses the locations
  // and components to pack |var|.
  //
  // For example, a stage variable `float3 foo[2]` needs 3 components in 2
  // locations. Assuming that we already assigned the following locations and
  // components to other stage variables:
  //
  //              Used components   /   nextUnusedComponent
  //  Location 0: 0 / 1                 2
  //  Location 1: 0 / 1 / 2             3
  //  Location 2: 0 / 1                 2
  //  Location 3: 0                     1
  //  Location 4: 0                     1
  //  Location 5: 0 / 1 / 2 / 3         4 (full)
  //
  //  we can assign Location 3 and Component 1 to `float3 foo[2]` because
  //  Location 3 and 4 have 3 unused Component slots (1, 2, 3).
  bool tryReuseLocations(const StageVar *var) {
    auto requiredLocsAndComponents = var->getLocationAndComponentCount();
    for (size_t startLoc = 0; startLoc < nextUnusedComponent.size();
         startLoc++) {
      uint32_t firstUnusedComponent = 0;
      // Check whether |requiredLocsAndComponents.location| locations starting
      // from |startLoc| have |requiredLocsAndComponents.component| unused
      // components or not. Note that if the number of required slots and used
      // slots is greater than 4, we cannot use that location because the
      // maximum number of available components for a location is 4.
      for (uint32_t i = 0; i < requiredLocsAndComponents.location; ++i) {
        if (startLoc + i >= nextUnusedComponent.size() ||
            nextUnusedComponent[startLoc + i] +
                    requiredLocsAndComponents.component >
                kNumComponentsInFullyUsedLocation) {
          firstUnusedComponent = kNumComponentsInFullyUsedLocation;
          break;
        }
        firstUnusedComponent =
            std::max(firstUnusedComponent, nextUnusedComponent[startLoc + i]);
      }
      if (firstUnusedComponent != kNumComponentsInFullyUsedLocation) {
        // Based on Vulkan spec "15.1.5. Component Assignment", a scalar or
        // two-component 64-bit data type must not specify a Component
        // decoration of 1 or 3.
        if (requiredLocsAndComponents.componentAlignment) {
          reuseLocations(var, startLoc, 2);
        } else {
          reuseLocations(var, startLoc, firstUnusedComponent);
        }
        return true;
      }
    }
    return false;
  }

  // Creates OpDecorate instructions for |var| with Location |startLoc| and
  // Component |componentStart|. Marks used component slots.
  void reuseLocations(const StageVar *var, uint32_t startLoc,
                      uint32_t componentStart) {
    auto requiredLocsAndComponents = var->getLocationAndComponentCount();
    spvBuilder.decorateLocation(var->getSpirvInstr(), assignedLocs[startLoc]);
    spvBuilder.decorateComponent(var->getSpirvInstr(), componentStart);

    for (uint32_t i = 0; i < requiredLocsAndComponents.location; ++i) {
      nextUnusedComponent[startLoc + i] =
          componentStart + requiredLocsAndComponents.component;
    }
  }

  // Pack signature of |var| into new unified stage variables.
  bool assignNewLocations(const StageVar *var) {
    auto requiredLocsAndComponents = var->getLocationAndComponentCount();
    uint32_t loc = assignLocs(requiredLocsAndComponents.location);
    spvBuilder.decorateLocation(var->getSpirvInstr(), loc);

    uint32_t componentCount = requiredLocsAndComponents.component;
    for (uint32_t i = 0; i < requiredLocsAndComponents.location; ++i) {
      assignedLocs.push_back(loc + i);
      nextUnusedComponent.push_back(componentCount);
    }
    return true;
  }

private:
  SpirvBuilder &spvBuilder;
  ///< A function to assign a new location number.
  llvm::function_ref<uint32_t(uint32_t)> assignLocs;
  ///< A vector of assigned locations.
  llvm::SmallVector<uint32_t, 8> assignedLocs;
  ///< A vector to keep the starting unused component number in each assigned
  ///< location.
  llvm::SmallVector<uint32_t, 8> nextUnusedComponent;
};

} // anonymous namespace

bool packSignatureInternal(
    const std::vector<const StageVar *> &vars,
    llvm::function_ref<bool(const StageVar *)> assignLocAndComponent,
    bool forInput, bool forPCF) {
  for (const auto *var : vars) {
    auto sigPointKind = var->getSigPoint()->GetKind();
    // HS has two types of outputs, one from the shader itself and another from
    // patch control function. They have HSCPOut and PCOut SigPointKind,
    // respectively. Since we do not know which one comes first at this moment,
    // we handle PCOut first. Likewise, DS has DSIn and DSCPIn as its inputs. We
    // handle DSIn first.
    if (forPCF) {
      if (sigPointKind != hlsl::SigPoint::Kind::PCOut &&
          sigPointKind != hlsl::SigPoint::Kind::DSIn) {
        continue;
      }
    } else {
      if (sigPointKind == hlsl::SigPoint::Kind::PCOut ||
          sigPointKind == hlsl::SigPoint::Kind::DSIn) {
        continue;
      }
    }

    if (!assignLocAndComponent(var)) {
      return false;
    }
  }
  return true;
}

bool packSignature(SpirvBuilder &spvBuilder,
                   const std::vector<const StageVar *> &vars,
                   llvm::function_ref<uint32_t(uint32_t)> nextLocs,
                   bool forInput) {
  PackedLocationAndComponentSet packedLocSet(spvBuilder, nextLocs);
  auto assignLocationAndComponent = [&packedLocSet](const StageVar *var) {
    return packedLocSet.assignLocAndComponent(var);
  };
  return packSignatureInternal(vars, assignLocationAndComponent, forInput,
                               true) &&
         packSignatureInternal(vars, assignLocationAndComponent, forInput,
                               false);
}

} // end namespace spirv
} // end namespace clang
