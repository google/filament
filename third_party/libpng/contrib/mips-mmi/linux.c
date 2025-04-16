/* contrib/mips-mmi/linux.c
 *
 * Copyright (c) 2024 Cosmin Truta
 * Written by guxiwei, 2023
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/auxv.h>

/*
 * parse_r var, r - Helper assembler macro for parsing register names.
 *
 * This converts the register name in $n form provided in \r to the
 * corresponding register number, which is assigned to the variable \var. It is
 * needed to allow explicit encoding of instructions in inline assembly where
 * registers are chosen by the compiler in $n form, allowing us to avoid using
 * fixed register numbers.
 *
 * It also allows newer instructions (not implemented by the assembler) to be
 * transparently implemented using assembler macros, instead of needing separate
 * cases depending on toolchain support.
 *
 * Simple usage example:
 * __asm__ __volatile__("parse_r __rt, %0\n\t"
 *                      ".insn\n\t"
 *                      "# di    %0\n\t"
 *                      ".word   (0x41606000 | (__rt << 16))"
 *                      : "=r" (status);
 */

/* Match an individual register number and assign to \var */
#define _IFC_REG(n)                                \
        ".ifc        \\r, $" #n "\n\t"             \
        "\\var        = " #n "\n\t"                \
        ".endif\n\t"

__asm__(".macro        parse_r var r\n\t"
        "\\var        = -1\n\t"
        _IFC_REG(0)  _IFC_REG(1)  _IFC_REG(2)  _IFC_REG(3)
        _IFC_REG(4)  _IFC_REG(5)  _IFC_REG(6)  _IFC_REG(7)
        _IFC_REG(8)  _IFC_REG(9)  _IFC_REG(10) _IFC_REG(11)
        _IFC_REG(12) _IFC_REG(13) _IFC_REG(14) _IFC_REG(15)
        _IFC_REG(16) _IFC_REG(17) _IFC_REG(18) _IFC_REG(19)
        _IFC_REG(20) _IFC_REG(21) _IFC_REG(22) _IFC_REG(23)
        _IFC_REG(24) _IFC_REG(25) _IFC_REG(26) _IFC_REG(27)
        _IFC_REG(28) _IFC_REG(29) _IFC_REG(30) _IFC_REG(31)
        ".iflt        \\var\n\t"
        ".error        \"Unable to parse register name \\r\"\n\t"
        ".endif\n\t"
        ".endm");

#define HWCAP_LOONGSON_CPUCFG (1 << 14)

static int cpucfg_available(void)
{
    return getauxval(AT_HWCAP) & HWCAP_LOONGSON_CPUCFG;
}

static int strstart(const char *str, const char *pfx, const char **ptr)
{
    while (*pfx && *pfx == *str) {
        pfx++;
        str++;
    }
    if (!*pfx && ptr)
        *ptr = str;
    return !*pfx;
}

/* Most toolchains have no CPUCFG support yet */
static uint32_t read_cpucfg(uint32_t reg)
{
        uint32_t __res;

        __asm__ __volatile__(
                "parse_r __res,%0\n\t"
                "parse_r reg,%1\n\t"
                ".insn \n\t"
                ".word (0xc8080118 | (reg << 21) | (__res << 11))\n\t"
                :"=r"(__res)
                :"r"(reg)
                :
                );
        return __res;
}

#define LOONGSON_CFG1 0x1

#define LOONGSON_CFG1_MMI    (1 << 4)

static int cpu_flags_cpucfg(void)
{
    int flags = 0;
    uint32_t cfg1 = read_cpucfg(LOONGSON_CFG1);

    if (cfg1 & LOONGSON_CFG1_MMI)
        flags = 1;

    return flags;
}

static int cpu_flags_cpuinfo(void)
{
    FILE *f = fopen("/proc/cpuinfo", "r");
    char buf[200];
    int flags = 0;

    if (!f)
        return flags;

    while (fgets(buf, sizeof(buf), f)) {
        /* Legacy kernel may not export MMI in ASEs implemented */
        if (strstart(buf, "cpu model", NULL)) {
            if (strstr(buf, "Loongson-3 "))
                flags = 1;
            break;
        }
        if (strstart(buf, "ASEs implemented", NULL)) {
            if (strstr(buf, " loongson-mmi"))
                flags = 1;
            break;
        }
    }
    fclose(f);
    return flags;
}

static int png_have_mmi()
{
    if (cpucfg_available())
        return cpu_flags_cpucfg();
    else
        return cpu_flags_cpuinfo();
    return 0;
}
