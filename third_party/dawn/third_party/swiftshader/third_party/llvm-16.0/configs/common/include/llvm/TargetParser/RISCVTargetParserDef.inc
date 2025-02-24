#ifndef PROC
#define PROC(ENUM, NAME, DEFAULT_MARCH)
#endif

PROC(INVALID, {"invalid"}, {""})
PROC(GENERIC_RV32, {"generic-rv32"}, {"rv32i2p0"})
PROC(GENERIC_RV64, {"generic-rv64"}, {"rv64i2p0"})
PROC(ROCKET_RV32, {"rocket-rv32"}, {"rv32i2p0"})
PROC(ROCKET_RV64, {"rocket-rv64"}, {"rv64i2p0"})
PROC(SIFIVE_E20, {"sifive-e20"}, {"rv32i2p0_m2p0_c2p0"})
PROC(SIFIVE_E21, {"sifive-e21"}, {"rv32i2p0_m2p0_a2p0_c2p0"})
PROC(SIFIVE_E24, {"sifive-e24"}, {"rv32i2p0_m2p0_a2p0_f2p0_c2p0"})
PROC(SIFIVE_E31, {"sifive-e31"}, {"rv32i2p0_m2p0_a2p0_c2p0"})
PROC(SIFIVE_E34, {"sifive-e34"}, {"rv32i2p0_m2p0_a2p0_f2p0_c2p0"})
PROC(SIFIVE_E76, {"sifive-e76"}, {"rv32i2p0_m2p0_a2p0_f2p0_c2p0"})
PROC(SIFIVE_S21, {"sifive-s21"}, {"rv64i2p0_m2p0_a2p0_c2p0"})
PROC(SIFIVE_S51, {"sifive-s51"}, {"rv64i2p0_m2p0_a2p0_c2p0"})
PROC(SIFIVE_S54, {"sifive-s54"}, {"rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"})
PROC(SIFIVE_S76, {"sifive-s76"}, {"rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"})
PROC(SIFIVE_U54, {"sifive-u54"}, {"rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"})
PROC(SIFIVE_U74, {"sifive-u74"}, {"rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"})
PROC(SYNTACORE_SCR1_BASE, {"syntacore-scr1-base"}, {"rv32i2p0_c2p0"})
PROC(SYNTACORE_SCR1_MAX, {"syntacore-scr1-max"}, {"rv32i2p0_m2p0_c2p0"})

#undef PROC

#ifndef TUNE_PROC
#define TUNE_PROC(ENUM, NAME)
#endif

TUNE_PROC(GENERIC, "generic")
TUNE_PROC(ROCKET, "rocket")
TUNE_PROC(SIFIVE_7, "sifive-7-series")

#undef TUNE_PROC
