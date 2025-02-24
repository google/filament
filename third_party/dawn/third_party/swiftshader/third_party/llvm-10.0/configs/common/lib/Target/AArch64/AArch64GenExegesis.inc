/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Exegesis Tables                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


static const char* AArch64PfmCounterNames[] = {
  "CPU_CYCLES", // 0
};


static const PfmCountersInfo AArch64DefaultPfmCounters = {
  AArch64PfmCounterNames[0],  // Cycle counter
  nullptr,  // No uops counter.
  nullptr,  // No issue counters.
  0
};

// Sorted (by CpuName) array of pfm counters.
static const CpuAndPfmCounters AArch64CpuPfmCounters[] = {
  { "", &AArch64DefaultPfmCounters },
};

