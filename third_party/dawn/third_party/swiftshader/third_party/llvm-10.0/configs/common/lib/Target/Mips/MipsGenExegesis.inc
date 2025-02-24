/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Exegesis Tables                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


static const char* MipsPfmCounterNames[] = {
  "CYCLES", // 0
};


static const PfmCountersInfo MipsDefaultPfmCounters = {
  MipsPfmCounterNames[0],  // Cycle counter
  nullptr,  // No uops counter.
  nullptr,  // No issue counters.
  0
};

// Sorted (by CpuName) array of pfm counters.
static const CpuAndPfmCounters MipsCpuPfmCounters[] = {
  { "", &MipsDefaultPfmCounters },
};

