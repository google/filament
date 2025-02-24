# Vulkan Mock ICD

This directory contains a mock ICD driver designed for validation layer testing.

## Introduction

The mock ICD is focused on enabling validation testing apart from an actual device. Because the validation layers
sit on top of the ICD and don't depend upon the results of Vulkan rendering, they can be tested without having actual
GPU hardware backing the ICD. The final mock driver will be composed of three main features: a null driver, flexible
device configuration, and entrypoint tracking & verification.

### Null Driver
The intial mock driver features just the null driver capability. This allows all of the validation tests to be run
on a fixed device configuration that is hard-coded into the ICD.

### Entrypoint Tracking & Verification
Entrypoint tracking and verification will be added to the mock layer as a later feature. The idea is that all expected
Vulkan function calls and their parameters can be stored in the ICD and then a separate call can be made to verify that
the exepected calls and parameters actually entered the ICD. This allows verification that the validation layers are
correctly passing calls and their parameters through to the ICD unchanged.

## Using the Mock ICD

To enable the mock ICD, set VK\_ICD\_FILENAMES environment variable to point to your {BUILD_DIR}/icd/VkICD\_mock\_icd.json.

## Plans

The initial mock ICD is just the null driver which can be used to test validation layers on
simulated devices. Here's a rough sequence of tasks planned for the mock driver going forward:
- [X] Get all LVL tests passing on the bare null driver
- [X] Get failing tests passing
- [X] Get skipped tests passing as able
- [ ] Get all LVL tests to run without unexpected errors
- [X] Develop automated test flow using mock ICD (alternative to or replacement for run\_all\_tests.sh)
- [ ] Update LVL tests with device dependencies to target specific device profiles
- [ ] Add entrypoint tracking & verification
- [ ] Initially track expected calls
- [ ] Update some tests to verify expected capability
- [ ] Expand tracking to include parameters

## Beyond Validation Layer Testing

The focus of the mock icd is for validation testing, but the code is available to use and enhance for anyone wishing to apply it for alternative
purposes.
With the following enhancements, the mock driver state available to the app should very closely mimic an actual ICD:
- Update various function return codes
- Simulated synchronization objects
- Simulated query with mock data
- Basic command buffer state tracking to note synch object transitions and query state updates

Beyond that it's certainly possible that the mock icd could be hooked up to a SW renderer and serve as a virtual GPU with complete rendering/compute
capabilities.

## Status

This is a temporary section used for tracking as the mock icd is being developed. Once all tests are passing with the mock, this section can be removed.
Currently 333/333 tests are passing with the mock icd, but many passing tests have unexpected validation errors that need to be cleaned up.

### Failing Tests

NONE

### Passing Tests With Unexpected Errors

- VkLayerTest.RenderPassInUseDestroyedSignaled
- VkLayerTest.RenderPassIncompatible

### Skipped Tests

- VkLayerTest.BindImageInvalidMemoryType
- VkLayerTest.CreatePipelineBadVertexAttributeFormat
- VkLayerTest.MiscBlitImageTests
- VkLayerTest.TemporaryExternalSemaphore
- VkLayerTest.TemporaryExternalFence
- VkLayerTest.InvalidBarriers
- VkLayerTest.CommandQueueFlags
- VkPositiveLayerTest.TwoQueuesEnsureCorrectRetirementWithWorkStolen
- VkPositiveLayerTest.ExternalSemaphore
- VkPositiveLayerTest.ExternalFence
- VkPositiveLayerTest.ExternalMemory
