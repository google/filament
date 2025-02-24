/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Exegesis Tables                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


static const char *X86PfmCounterNames[] = {
  "cpu_clk_unhalted", // 0
  "cycles_not_in_halt", // 1
  "dispatched_fpu:pipe0", // 2
  "dispatched_fpu:pipe1", // 3
  "dispatched_fpu_ops:ops_pipe0 + dispatched_fpu_ops:ops_dual_pipe0", // 4
  "dispatched_fpu_ops:ops_pipe1 + dispatched_fpu_ops:ops_dual_pipe1", // 5
  "dispatched_fpu_ops:ops_pipe2 + dispatched_fpu_ops:ops_dual_pipe2", // 6
  "dispatched_fpu_ops:ops_pipe3 + dispatched_fpu_ops:ops_dual_pipe3", // 7
  "div_op_count", // 8
  "fpu_pipe_assignment:total0", // 9
  "fpu_pipe_assignment:total1", // 10
  "fpu_pipe_assignment:total2", // 11
  "fpu_pipe_assignment:total3", // 12
  "ls_dispatch:ld_dispatch", // 13
  "ls_dispatch:ld_st_dispatch + ls_dispatch:ld_dispatch + ls_dispatch:store_dispatch", // 14
  "ls_dispatch:store_dispatch", // 15
  "mem_uop_retired:any_ld + mem_uop_retired:any_st", // 16
  "ops_type_dispatched_from_decoder:fp_disp_retire_mode", // 17
  "ops_type_dispatched_from_decoder:int_disp_retire_mode", // 18
  "retired_ops", // 19
  "retired_uops", // 20
  "rs_uops_dispatched_cycles:port_0", // 21
  "rs_uops_dispatched_cycles:port_1", // 22
  "rs_uops_dispatched_cycles:port_2 + rs_uops_dispatched_cycles:port_3", // 23
  "rs_uops_dispatched_cycles:port_4", // 24
  "rs_uops_dispatched_cycles:port_5", // 25
  "unhalted_core_cycles", // 26
  "uops_dispatched_port:port_0", // 27
  "uops_dispatched_port:port_1", // 28
  "uops_dispatched_port:port_2", // 29
  "uops_dispatched_port:port_2 + uops_dispatched_port:port_3", // 30
  "uops_dispatched_port:port_2_3", // 31
  "uops_dispatched_port:port_3", // 32
  "uops_dispatched_port:port_4", // 33
  "uops_dispatched_port:port_4_9", // 34
  "uops_dispatched_port:port_5", // 35
  "uops_dispatched_port:port_6", // 36
  "uops_dispatched_port:port_7", // 37
  "uops_dispatched_port:port_7_8", // 38
  "uops_executed:port0", // 39
  "uops_executed:port1", // 40
  "uops_executed:port2_core + uops_executed:port3_core", // 41
  "uops_executed:port4_core", // 42
  "uops_executed:port5", // 43
  "uops_executed_port:port_0", // 44
  "uops_executed_port:port_1", // 45
  "uops_executed_port:port_2", // 46
  "uops_executed_port:port_3", // 47
  "uops_executed_port:port_4", // 48
  "uops_executed_port:port_5", // 49
  "uops_executed_port:port_6", // 50
  "uops_executed_port:port_7", // 51
  "uops_issued:any", // 52
  "uops_retired", // 53
  "uops_retired:all", // 54
  "uops_retired:any", // 55
};

static const PfmCountersInfo::IssueCounter X86PfmIssueCounters[] = {
  { X86PfmCounterNames[4], "PdFPU0"},
  { X86PfmCounterNames[5], "PdFPU1"},
  { X86PfmCounterNames[6], "PdFPU2"},
  { X86PfmCounterNames[7], "PdFPU3"},
  { X86PfmCounterNames[4], "SrFPU0"},
  { X86PfmCounterNames[5], "SrFPU1"},
  { X86PfmCounterNames[6], "SrFPU2"},
  { X86PfmCounterNames[44], "BWPort0"},
  { X86PfmCounterNames[45], "BWPort1"},
  { X86PfmCounterNames[46], "BWPort2"},
  { X86PfmCounterNames[47], "BWPort3"},
  { X86PfmCounterNames[48], "BWPort4"},
  { X86PfmCounterNames[49], "BWPort5"},
  { X86PfmCounterNames[50], "BWPort6"},
  { X86PfmCounterNames[51], "BWPort7"},
  { X86PfmCounterNames[2], "BtFPU0"},
  { X86PfmCounterNames[3], "BtFPU1"},
  { X86PfmCounterNames[2], "JFPU0"},
  { X86PfmCounterNames[3], "JFPU1"},
  { X86PfmCounterNames[21], "SBPort0"},
  { X86PfmCounterNames[22], "SBPort1"},
  { X86PfmCounterNames[23], "SBPort23"},
  { X86PfmCounterNames[24], "SBPort4"},
  { X86PfmCounterNames[25], "SBPort5"},
  { X86PfmCounterNames[44], "HWPort0"},
  { X86PfmCounterNames[45], "HWPort1"},
  { X86PfmCounterNames[46], "HWPort2"},
  { X86PfmCounterNames[47], "HWPort3"},
  { X86PfmCounterNames[48], "HWPort4"},
  { X86PfmCounterNames[49], "HWPort5"},
  { X86PfmCounterNames[50], "HWPort6"},
  { X86PfmCounterNames[51], "HWPort7"},
  { X86PfmCounterNames[27], "ICXPort0"},
  { X86PfmCounterNames[28], "ICXPort1"},
  { X86PfmCounterNames[31], "ICXPort23"},
  { X86PfmCounterNames[34], "ICXPort49"},
  { X86PfmCounterNames[35], "ICXPort5"},
  { X86PfmCounterNames[36], "ICXPort6"},
  { X86PfmCounterNames[38], "ICXPort78"},
  { X86PfmCounterNames[39], "SBPort0"},
  { X86PfmCounterNames[40], "SBPort1"},
  { X86PfmCounterNames[41], "SBPort23"},
  { X86PfmCounterNames[42], "SBPort4"},
  { X86PfmCounterNames[43], "SBPort5"},
  { X86PfmCounterNames[16], "SLM_MEC_RSV"},
  { X86PfmCounterNames[27], "SBPort0"},
  { X86PfmCounterNames[28], "SBPort1"},
  { X86PfmCounterNames[30], "SBPort23"},
  { X86PfmCounterNames[33], "SBPort4"},
  { X86PfmCounterNames[35], "SBPort5"},
  { X86PfmCounterNames[27], "SKLPort0"},
  { X86PfmCounterNames[28], "SKLPort1"},
  { X86PfmCounterNames[29], "SKLPort2"},
  { X86PfmCounterNames[32], "SKLPort3"},
  { X86PfmCounterNames[33], "SKLPort4"},
  { X86PfmCounterNames[35], "SKLPort5"},
  { X86PfmCounterNames[36], "SKLPort6"},
  { X86PfmCounterNames[37], "SKLPort7"},
  { X86PfmCounterNames[27], "SKXPort0"},
  { X86PfmCounterNames[28], "SKXPort1"},
  { X86PfmCounterNames[29], "SKXPort2"},
  { X86PfmCounterNames[32], "SKXPort3"},
  { X86PfmCounterNames[33], "SKXPort4"},
  { X86PfmCounterNames[35], "SKXPort5"},
  { X86PfmCounterNames[36], "SKXPort6"},
  { X86PfmCounterNames[37], "SKXPort7"},
  { X86PfmCounterNames[9], "ZnFPU0"},
  { X86PfmCounterNames[10], "ZnFPU1"},
  { X86PfmCounterNames[11], "ZnFPU2"},
  { X86PfmCounterNames[12], "ZnFPU3"},
  { X86PfmCounterNames[14], "ZnAGU"},
  { X86PfmCounterNames[8], "ZnDivider"},
  { X86PfmCounterNames[14], "Zn2AGU"},
  { X86PfmCounterNames[8], "Zn2Divider"},
  { X86PfmCounterNames[18], "Zn3Int"},
  { X86PfmCounterNames[17], "Zn3FPU"},
  { X86PfmCounterNames[13], "Zn3Load"},
  { X86PfmCounterNames[15], "Zn3Store"},
  { X86PfmCounterNames[8], "Zn3Divider"},
};

static const PfmCountersInfo X86AtomPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[55],  // Uops counter
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86BdVer2PfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 0, 4 // Issue counters.
};

static const PfmCountersInfo X86BdVer3PfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 4, 3 // Issue counters.
};

static const PfmCountersInfo X86BroadwellPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 7, 8 // Issue counters.
};

static const PfmCountersInfo X86BtVer1PfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 15, 2 // Issue counters.
};

static const PfmCountersInfo X86BtVer2PfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 17, 2 // Issue counters.
};

static const PfmCountersInfo X86Core2PfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[55],  // Uops counter
  X86PfmIssueCounters + 19, 5 // Issue counters.
};

static const PfmCountersInfo X86CorePfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[55],  // Uops counter
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86DefaultAMDPfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86DefaultPfmCounters = {
  nullptr,  // No cycle counter.
  nullptr,  // No uops counter.
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86HaswellPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 24, 8 // Issue counters.
};

static const PfmCountersInfo X86IceLakePfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 32, 7 // Issue counters.
};

static const PfmCountersInfo X86KnightPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[54],  // Uops counter
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86NehalemPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[55],  // Uops counter
  X86PfmIssueCounters + 39, 5 // Issue counters.
};

static const PfmCountersInfo X86PentiumPfmCounters = {
  X86PfmCounterNames[0],  // Cycle counter
  X86PfmCounterNames[53],  // Uops counter
  nullptr,  // No issue counters.
  0
};

static const PfmCountersInfo X86SLMPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[55],  // Uops counter
  X86PfmIssueCounters + 44, 1 // Issue counters.
};

static const PfmCountersInfo X86SandyBridgePfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 45, 5 // Issue counters.
};

static const PfmCountersInfo X86SkylakeClientPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 50, 8 // Issue counters.
};

static const PfmCountersInfo X86SkylakeServerPfmCounters = {
  X86PfmCounterNames[26],  // Cycle counter
  X86PfmCounterNames[52],  // Uops counter
  X86PfmIssueCounters + 58, 8 // Issue counters.
};

static const PfmCountersInfo X86ZnVer1PfmCounters = {
  X86PfmCounterNames[1],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 66, 6 // Issue counters.
};

static const PfmCountersInfo X86ZnVer2PfmCounters = {
  X86PfmCounterNames[1],  // Cycle counter
  X86PfmCounterNames[20],  // Uops counter
  X86PfmIssueCounters + 72, 2 // Issue counters.
};

static const PfmCountersInfo X86ZnVer3PfmCounters = {
  X86PfmCounterNames[1],  // Cycle counter
  X86PfmCounterNames[19],  // Uops counter
  X86PfmIssueCounters + 74, 5 // Issue counters.
};

// Sorted (by CpuName) array of pfm counters.
static const CpuAndPfmCounters X86CpuPfmCounters[] = {
  { "", &X86DefaultPfmCounters },
  { "amdfam10", &X86DefaultAMDPfmCounters },
  { "athlon", &X86DefaultAMDPfmCounters },
  { "athlon-4", &X86DefaultAMDPfmCounters },
  { "athlon-fx", &X86DefaultAMDPfmCounters },
  { "athlon-mp", &X86DefaultAMDPfmCounters },
  { "athlon-tbird", &X86DefaultAMDPfmCounters },
  { "athlon-xp", &X86DefaultAMDPfmCounters },
  { "athlon64", &X86DefaultAMDPfmCounters },
  { "athlon64-sse3", &X86DefaultAMDPfmCounters },
  { "atom", &X86AtomPfmCounters },
  { "barcelona", &X86DefaultAMDPfmCounters },
  { "bdver1", &X86BdVer2PfmCounters },
  { "bdver2", &X86BdVer2PfmCounters },
  { "bdver3", &X86BdVer3PfmCounters },
  { "bdver4", &X86BdVer3PfmCounters },
  { "bonnell", &X86AtomPfmCounters },
  { "broadwell", &X86BroadwellPfmCounters },
  { "btver1", &X86BtVer1PfmCounters },
  { "btver2", &X86BtVer2PfmCounters },
  { "cannonlake", &X86SkylakeServerPfmCounters },
  { "cascadelake", &X86SkylakeServerPfmCounters },
  { "core2", &X86Core2PfmCounters },
  { "corei7", &X86NehalemPfmCounters },
  { "goldmont", &X86SLMPfmCounters },
  { "goldmont-plus", &X86SLMPfmCounters },
  { "haswell", &X86HaswellPfmCounters },
  { "icelake-client", &X86IceLakePfmCounters },
  { "icelake-server", &X86IceLakePfmCounters },
  { "ivybridge", &X86SandyBridgePfmCounters },
  { "k8", &X86DefaultAMDPfmCounters },
  { "k8-sse3", &X86DefaultAMDPfmCounters },
  { "knl", &X86KnightPfmCounters },
  { "knm", &X86KnightPfmCounters },
  { "nehalem", &X86NehalemPfmCounters },
  { "opteron", &X86DefaultAMDPfmCounters },
  { "opteron-sse3", &X86DefaultAMDPfmCounters },
  { "penryn", &X86Core2PfmCounters },
  { "pentium-m", &X86PentiumPfmCounters },
  { "pentium2", &X86PentiumPfmCounters },
  { "pentium3", &X86PentiumPfmCounters },
  { "pentium3m", &X86PentiumPfmCounters },
  { "pentiumpro", &X86PentiumPfmCounters },
  { "prescott", &X86CorePfmCounters },
  { "rocketlake", &X86IceLakePfmCounters },
  { "sandybridge", &X86SandyBridgePfmCounters },
  { "silvermont", &X86SLMPfmCounters },
  { "skylake", &X86SkylakeClientPfmCounters },
  { "skylake-avx512", &X86SkylakeServerPfmCounters },
  { "tigerlake", &X86IceLakePfmCounters },
  { "tremont", &X86SLMPfmCounters },
  { "westmere", &X86NehalemPfmCounters },
  { "yonah", &X86CorePfmCounters },
  { "znver1", &X86ZnVer1PfmCounters },
  { "znver2", &X86ZnVer2PfmCounters },
  { "znver3", &X86ZnVer3PfmCounters },
  { "znver4", &X86ZnVer3PfmCounters },
};

