/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Target Register Enum Values                                                *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_ENUM
#undef GET_REGINFO_ENUM

namespace llvm {

class MCRegisterClass;
extern const MCRegisterClass AArch64MCRegisterClasses[];

namespace AArch64 {
enum {
  NoRegister,
  FFR = 1,
  FP = 2,
  LR = 3,
  NZCV = 4,
  SP = 5,
  WSP = 6,
  WZR = 7,
  XZR = 8,
  B0 = 9,
  B1 = 10,
  B2 = 11,
  B3 = 12,
  B4 = 13,
  B5 = 14,
  B6 = 15,
  B7 = 16,
  B8 = 17,
  B9 = 18,
  B10 = 19,
  B11 = 20,
  B12 = 21,
  B13 = 22,
  B14 = 23,
  B15 = 24,
  B16 = 25,
  B17 = 26,
  B18 = 27,
  B19 = 28,
  B20 = 29,
  B21 = 30,
  B22 = 31,
  B23 = 32,
  B24 = 33,
  B25 = 34,
  B26 = 35,
  B27 = 36,
  B28 = 37,
  B29 = 38,
  B30 = 39,
  B31 = 40,
  D0 = 41,
  D1 = 42,
  D2 = 43,
  D3 = 44,
  D4 = 45,
  D5 = 46,
  D6 = 47,
  D7 = 48,
  D8 = 49,
  D9 = 50,
  D10 = 51,
  D11 = 52,
  D12 = 53,
  D13 = 54,
  D14 = 55,
  D15 = 56,
  D16 = 57,
  D17 = 58,
  D18 = 59,
  D19 = 60,
  D20 = 61,
  D21 = 62,
  D22 = 63,
  D23 = 64,
  D24 = 65,
  D25 = 66,
  D26 = 67,
  D27 = 68,
  D28 = 69,
  D29 = 70,
  D30 = 71,
  D31 = 72,
  H0 = 73,
  H1 = 74,
  H2 = 75,
  H3 = 76,
  H4 = 77,
  H5 = 78,
  H6 = 79,
  H7 = 80,
  H8 = 81,
  H9 = 82,
  H10 = 83,
  H11 = 84,
  H12 = 85,
  H13 = 86,
  H14 = 87,
  H15 = 88,
  H16 = 89,
  H17 = 90,
  H18 = 91,
  H19 = 92,
  H20 = 93,
  H21 = 94,
  H22 = 95,
  H23 = 96,
  H24 = 97,
  H25 = 98,
  H26 = 99,
  H27 = 100,
  H28 = 101,
  H29 = 102,
  H30 = 103,
  H31 = 104,
  P0 = 105,
  P1 = 106,
  P2 = 107,
  P3 = 108,
  P4 = 109,
  P5 = 110,
  P6 = 111,
  P7 = 112,
  P8 = 113,
  P9 = 114,
  P10 = 115,
  P11 = 116,
  P12 = 117,
  P13 = 118,
  P14 = 119,
  P15 = 120,
  Q0 = 121,
  Q1 = 122,
  Q2 = 123,
  Q3 = 124,
  Q4 = 125,
  Q5 = 126,
  Q6 = 127,
  Q7 = 128,
  Q8 = 129,
  Q9 = 130,
  Q10 = 131,
  Q11 = 132,
  Q12 = 133,
  Q13 = 134,
  Q14 = 135,
  Q15 = 136,
  Q16 = 137,
  Q17 = 138,
  Q18 = 139,
  Q19 = 140,
  Q20 = 141,
  Q21 = 142,
  Q22 = 143,
  Q23 = 144,
  Q24 = 145,
  Q25 = 146,
  Q26 = 147,
  Q27 = 148,
  Q28 = 149,
  Q29 = 150,
  Q30 = 151,
  Q31 = 152,
  S0 = 153,
  S1 = 154,
  S2 = 155,
  S3 = 156,
  S4 = 157,
  S5 = 158,
  S6 = 159,
  S7 = 160,
  S8 = 161,
  S9 = 162,
  S10 = 163,
  S11 = 164,
  S12 = 165,
  S13 = 166,
  S14 = 167,
  S15 = 168,
  S16 = 169,
  S17 = 170,
  S18 = 171,
  S19 = 172,
  S20 = 173,
  S21 = 174,
  S22 = 175,
  S23 = 176,
  S24 = 177,
  S25 = 178,
  S26 = 179,
  S27 = 180,
  S28 = 181,
  S29 = 182,
  S30 = 183,
  S31 = 184,
  W0 = 185,
  W1 = 186,
  W2 = 187,
  W3 = 188,
  W4 = 189,
  W5 = 190,
  W6 = 191,
  W7 = 192,
  W8 = 193,
  W9 = 194,
  W10 = 195,
  W11 = 196,
  W12 = 197,
  W13 = 198,
  W14 = 199,
  W15 = 200,
  W16 = 201,
  W17 = 202,
  W18 = 203,
  W19 = 204,
  W20 = 205,
  W21 = 206,
  W22 = 207,
  W23 = 208,
  W24 = 209,
  W25 = 210,
  W26 = 211,
  W27 = 212,
  W28 = 213,
  W29 = 214,
  W30 = 215,
  X0 = 216,
  X1 = 217,
  X2 = 218,
  X3 = 219,
  X4 = 220,
  X5 = 221,
  X6 = 222,
  X7 = 223,
  X8 = 224,
  X9 = 225,
  X10 = 226,
  X11 = 227,
  X12 = 228,
  X13 = 229,
  X14 = 230,
  X15 = 231,
  X16 = 232,
  X17 = 233,
  X18 = 234,
  X19 = 235,
  X20 = 236,
  X21 = 237,
  X22 = 238,
  X23 = 239,
  X24 = 240,
  X25 = 241,
  X26 = 242,
  X27 = 243,
  X28 = 244,
  Z0 = 245,
  Z1 = 246,
  Z2 = 247,
  Z3 = 248,
  Z4 = 249,
  Z5 = 250,
  Z6 = 251,
  Z7 = 252,
  Z8 = 253,
  Z9 = 254,
  Z10 = 255,
  Z11 = 256,
  Z12 = 257,
  Z13 = 258,
  Z14 = 259,
  Z15 = 260,
  Z16 = 261,
  Z17 = 262,
  Z18 = 263,
  Z19 = 264,
  Z20 = 265,
  Z21 = 266,
  Z22 = 267,
  Z23 = 268,
  Z24 = 269,
  Z25 = 270,
  Z26 = 271,
  Z27 = 272,
  Z28 = 273,
  Z29 = 274,
  Z30 = 275,
  Z31 = 276,
  Z0_HI = 277,
  Z1_HI = 278,
  Z2_HI = 279,
  Z3_HI = 280,
  Z4_HI = 281,
  Z5_HI = 282,
  Z6_HI = 283,
  Z7_HI = 284,
  Z8_HI = 285,
  Z9_HI = 286,
  Z10_HI = 287,
  Z11_HI = 288,
  Z12_HI = 289,
  Z13_HI = 290,
  Z14_HI = 291,
  Z15_HI = 292,
  Z16_HI = 293,
  Z17_HI = 294,
  Z18_HI = 295,
  Z19_HI = 296,
  Z20_HI = 297,
  Z21_HI = 298,
  Z22_HI = 299,
  Z23_HI = 300,
  Z24_HI = 301,
  Z25_HI = 302,
  Z26_HI = 303,
  Z27_HI = 304,
  Z28_HI = 305,
  Z29_HI = 306,
  Z30_HI = 307,
  Z31_HI = 308,
  D0_D1 = 309,
  D1_D2 = 310,
  D2_D3 = 311,
  D3_D4 = 312,
  D4_D5 = 313,
  D5_D6 = 314,
  D6_D7 = 315,
  D7_D8 = 316,
  D8_D9 = 317,
  D9_D10 = 318,
  D10_D11 = 319,
  D11_D12 = 320,
  D12_D13 = 321,
  D13_D14 = 322,
  D14_D15 = 323,
  D15_D16 = 324,
  D16_D17 = 325,
  D17_D18 = 326,
  D18_D19 = 327,
  D19_D20 = 328,
  D20_D21 = 329,
  D21_D22 = 330,
  D22_D23 = 331,
  D23_D24 = 332,
  D24_D25 = 333,
  D25_D26 = 334,
  D26_D27 = 335,
  D27_D28 = 336,
  D28_D29 = 337,
  D29_D30 = 338,
  D30_D31 = 339,
  D31_D0 = 340,
  D0_D1_D2_D3 = 341,
  D1_D2_D3_D4 = 342,
  D2_D3_D4_D5 = 343,
  D3_D4_D5_D6 = 344,
  D4_D5_D6_D7 = 345,
  D5_D6_D7_D8 = 346,
  D6_D7_D8_D9 = 347,
  D7_D8_D9_D10 = 348,
  D8_D9_D10_D11 = 349,
  D9_D10_D11_D12 = 350,
  D10_D11_D12_D13 = 351,
  D11_D12_D13_D14 = 352,
  D12_D13_D14_D15 = 353,
  D13_D14_D15_D16 = 354,
  D14_D15_D16_D17 = 355,
  D15_D16_D17_D18 = 356,
  D16_D17_D18_D19 = 357,
  D17_D18_D19_D20 = 358,
  D18_D19_D20_D21 = 359,
  D19_D20_D21_D22 = 360,
  D20_D21_D22_D23 = 361,
  D21_D22_D23_D24 = 362,
  D22_D23_D24_D25 = 363,
  D23_D24_D25_D26 = 364,
  D24_D25_D26_D27 = 365,
  D25_D26_D27_D28 = 366,
  D26_D27_D28_D29 = 367,
  D27_D28_D29_D30 = 368,
  D28_D29_D30_D31 = 369,
  D29_D30_D31_D0 = 370,
  D30_D31_D0_D1 = 371,
  D31_D0_D1_D2 = 372,
  D0_D1_D2 = 373,
  D1_D2_D3 = 374,
  D2_D3_D4 = 375,
  D3_D4_D5 = 376,
  D4_D5_D6 = 377,
  D5_D6_D7 = 378,
  D6_D7_D8 = 379,
  D7_D8_D9 = 380,
  D8_D9_D10 = 381,
  D9_D10_D11 = 382,
  D10_D11_D12 = 383,
  D11_D12_D13 = 384,
  D12_D13_D14 = 385,
  D13_D14_D15 = 386,
  D14_D15_D16 = 387,
  D15_D16_D17 = 388,
  D16_D17_D18 = 389,
  D17_D18_D19 = 390,
  D18_D19_D20 = 391,
  D19_D20_D21 = 392,
  D20_D21_D22 = 393,
  D21_D22_D23 = 394,
  D22_D23_D24 = 395,
  D23_D24_D25 = 396,
  D24_D25_D26 = 397,
  D25_D26_D27 = 398,
  D26_D27_D28 = 399,
  D27_D28_D29 = 400,
  D28_D29_D30 = 401,
  D29_D30_D31 = 402,
  D30_D31_D0 = 403,
  D31_D0_D1 = 404,
  Q0_Q1 = 405,
  Q1_Q2 = 406,
  Q2_Q3 = 407,
  Q3_Q4 = 408,
  Q4_Q5 = 409,
  Q5_Q6 = 410,
  Q6_Q7 = 411,
  Q7_Q8 = 412,
  Q8_Q9 = 413,
  Q9_Q10 = 414,
  Q10_Q11 = 415,
  Q11_Q12 = 416,
  Q12_Q13 = 417,
  Q13_Q14 = 418,
  Q14_Q15 = 419,
  Q15_Q16 = 420,
  Q16_Q17 = 421,
  Q17_Q18 = 422,
  Q18_Q19 = 423,
  Q19_Q20 = 424,
  Q20_Q21 = 425,
  Q21_Q22 = 426,
  Q22_Q23 = 427,
  Q23_Q24 = 428,
  Q24_Q25 = 429,
  Q25_Q26 = 430,
  Q26_Q27 = 431,
  Q27_Q28 = 432,
  Q28_Q29 = 433,
  Q29_Q30 = 434,
  Q30_Q31 = 435,
  Q31_Q0 = 436,
  Q0_Q1_Q2_Q3 = 437,
  Q1_Q2_Q3_Q4 = 438,
  Q2_Q3_Q4_Q5 = 439,
  Q3_Q4_Q5_Q6 = 440,
  Q4_Q5_Q6_Q7 = 441,
  Q5_Q6_Q7_Q8 = 442,
  Q6_Q7_Q8_Q9 = 443,
  Q7_Q8_Q9_Q10 = 444,
  Q8_Q9_Q10_Q11 = 445,
  Q9_Q10_Q11_Q12 = 446,
  Q10_Q11_Q12_Q13 = 447,
  Q11_Q12_Q13_Q14 = 448,
  Q12_Q13_Q14_Q15 = 449,
  Q13_Q14_Q15_Q16 = 450,
  Q14_Q15_Q16_Q17 = 451,
  Q15_Q16_Q17_Q18 = 452,
  Q16_Q17_Q18_Q19 = 453,
  Q17_Q18_Q19_Q20 = 454,
  Q18_Q19_Q20_Q21 = 455,
  Q19_Q20_Q21_Q22 = 456,
  Q20_Q21_Q22_Q23 = 457,
  Q21_Q22_Q23_Q24 = 458,
  Q22_Q23_Q24_Q25 = 459,
  Q23_Q24_Q25_Q26 = 460,
  Q24_Q25_Q26_Q27 = 461,
  Q25_Q26_Q27_Q28 = 462,
  Q26_Q27_Q28_Q29 = 463,
  Q27_Q28_Q29_Q30 = 464,
  Q28_Q29_Q30_Q31 = 465,
  Q29_Q30_Q31_Q0 = 466,
  Q30_Q31_Q0_Q1 = 467,
  Q31_Q0_Q1_Q2 = 468,
  Q0_Q1_Q2 = 469,
  Q1_Q2_Q3 = 470,
  Q2_Q3_Q4 = 471,
  Q3_Q4_Q5 = 472,
  Q4_Q5_Q6 = 473,
  Q5_Q6_Q7 = 474,
  Q6_Q7_Q8 = 475,
  Q7_Q8_Q9 = 476,
  Q8_Q9_Q10 = 477,
  Q9_Q10_Q11 = 478,
  Q10_Q11_Q12 = 479,
  Q11_Q12_Q13 = 480,
  Q12_Q13_Q14 = 481,
  Q13_Q14_Q15 = 482,
  Q14_Q15_Q16 = 483,
  Q15_Q16_Q17 = 484,
  Q16_Q17_Q18 = 485,
  Q17_Q18_Q19 = 486,
  Q18_Q19_Q20 = 487,
  Q19_Q20_Q21 = 488,
  Q20_Q21_Q22 = 489,
  Q21_Q22_Q23 = 490,
  Q22_Q23_Q24 = 491,
  Q23_Q24_Q25 = 492,
  Q24_Q25_Q26 = 493,
  Q25_Q26_Q27 = 494,
  Q26_Q27_Q28 = 495,
  Q27_Q28_Q29 = 496,
  Q28_Q29_Q30 = 497,
  Q29_Q30_Q31 = 498,
  Q30_Q31_Q0 = 499,
  Q31_Q0_Q1 = 500,
  W30_WZR = 501,
  W0_W1 = 502,
  W2_W3 = 503,
  W4_W5 = 504,
  W6_W7 = 505,
  W8_W9 = 506,
  W10_W11 = 507,
  W12_W13 = 508,
  W14_W15 = 509,
  W16_W17 = 510,
  W18_W19 = 511,
  W20_W21 = 512,
  W22_W23 = 513,
  W24_W25 = 514,
  W26_W27 = 515,
  W28_W29 = 516,
  LR_XZR = 517,
  X28_FP = 518,
  X0_X1 = 519,
  X2_X3 = 520,
  X4_X5 = 521,
  X6_X7 = 522,
  X8_X9 = 523,
  X10_X11 = 524,
  X12_X13 = 525,
  X14_X15 = 526,
  X16_X17 = 527,
  X18_X19 = 528,
  X20_X21 = 529,
  X22_X23 = 530,
  X24_X25 = 531,
  X26_X27 = 532,
  Z0_Z1 = 533,
  Z1_Z2 = 534,
  Z2_Z3 = 535,
  Z3_Z4 = 536,
  Z4_Z5 = 537,
  Z5_Z6 = 538,
  Z6_Z7 = 539,
  Z7_Z8 = 540,
  Z8_Z9 = 541,
  Z9_Z10 = 542,
  Z10_Z11 = 543,
  Z11_Z12 = 544,
  Z12_Z13 = 545,
  Z13_Z14 = 546,
  Z14_Z15 = 547,
  Z15_Z16 = 548,
  Z16_Z17 = 549,
  Z17_Z18 = 550,
  Z18_Z19 = 551,
  Z19_Z20 = 552,
  Z20_Z21 = 553,
  Z21_Z22 = 554,
  Z22_Z23 = 555,
  Z23_Z24 = 556,
  Z24_Z25 = 557,
  Z25_Z26 = 558,
  Z26_Z27 = 559,
  Z27_Z28 = 560,
  Z28_Z29 = 561,
  Z29_Z30 = 562,
  Z30_Z31 = 563,
  Z31_Z0 = 564,
  Z0_Z1_Z2_Z3 = 565,
  Z1_Z2_Z3_Z4 = 566,
  Z2_Z3_Z4_Z5 = 567,
  Z3_Z4_Z5_Z6 = 568,
  Z4_Z5_Z6_Z7 = 569,
  Z5_Z6_Z7_Z8 = 570,
  Z6_Z7_Z8_Z9 = 571,
  Z7_Z8_Z9_Z10 = 572,
  Z8_Z9_Z10_Z11 = 573,
  Z9_Z10_Z11_Z12 = 574,
  Z10_Z11_Z12_Z13 = 575,
  Z11_Z12_Z13_Z14 = 576,
  Z12_Z13_Z14_Z15 = 577,
  Z13_Z14_Z15_Z16 = 578,
  Z14_Z15_Z16_Z17 = 579,
  Z15_Z16_Z17_Z18 = 580,
  Z16_Z17_Z18_Z19 = 581,
  Z17_Z18_Z19_Z20 = 582,
  Z18_Z19_Z20_Z21 = 583,
  Z19_Z20_Z21_Z22 = 584,
  Z20_Z21_Z22_Z23 = 585,
  Z21_Z22_Z23_Z24 = 586,
  Z22_Z23_Z24_Z25 = 587,
  Z23_Z24_Z25_Z26 = 588,
  Z24_Z25_Z26_Z27 = 589,
  Z25_Z26_Z27_Z28 = 590,
  Z26_Z27_Z28_Z29 = 591,
  Z27_Z28_Z29_Z30 = 592,
  Z28_Z29_Z30_Z31 = 593,
  Z29_Z30_Z31_Z0 = 594,
  Z30_Z31_Z0_Z1 = 595,
  Z31_Z0_Z1_Z2 = 596,
  Z0_Z1_Z2 = 597,
  Z1_Z2_Z3 = 598,
  Z2_Z3_Z4 = 599,
  Z3_Z4_Z5 = 600,
  Z4_Z5_Z6 = 601,
  Z5_Z6_Z7 = 602,
  Z6_Z7_Z8 = 603,
  Z7_Z8_Z9 = 604,
  Z8_Z9_Z10 = 605,
  Z9_Z10_Z11 = 606,
  Z10_Z11_Z12 = 607,
  Z11_Z12_Z13 = 608,
  Z12_Z13_Z14 = 609,
  Z13_Z14_Z15 = 610,
  Z14_Z15_Z16 = 611,
  Z15_Z16_Z17 = 612,
  Z16_Z17_Z18 = 613,
  Z17_Z18_Z19 = 614,
  Z18_Z19_Z20 = 615,
  Z19_Z20_Z21 = 616,
  Z20_Z21_Z22 = 617,
  Z21_Z22_Z23 = 618,
  Z22_Z23_Z24 = 619,
  Z23_Z24_Z25 = 620,
  Z24_Z25_Z26 = 621,
  Z25_Z26_Z27 = 622,
  Z26_Z27_Z28 = 623,
  Z27_Z28_Z29 = 624,
  Z28_Z29_Z30 = 625,
  Z29_Z30_Z31 = 626,
  Z30_Z31_Z0 = 627,
  Z31_Z0_Z1 = 628,
  NUM_TARGET_REGS 	// 629
};
} // end namespace AArch64

// Register classes

namespace AArch64 {
enum {
  FPR8RegClassID = 0,
  FPR16RegClassID = 1,
  PPRRegClassID = 2,
  PPR_3bRegClassID = 3,
  GPR32allRegClassID = 4,
  FPR32RegClassID = 5,
  GPR32RegClassID = 6,
  GPR32spRegClassID = 7,
  GPR32commonRegClassID = 8,
  GPR32argRegClassID = 9,
  CCRRegClassID = 10,
  GPR32sponlyRegClassID = 11,
  WSeqPairsClassRegClassID = 12,
  WSeqPairsClass_with_subo32_in_GPR32commonRegClassID = 13,
  WSeqPairsClass_with_sube32_in_GPR32argRegClassID = 14,
  GPR64allRegClassID = 15,
  FPR64RegClassID = 16,
  GPR64RegClassID = 17,
  GPR64spRegClassID = 18,
  GPR64commonRegClassID = 19,
  GPR64noipRegClassID = 20,
  GPR64common_and_GPR64noipRegClassID = 21,
  tcGPR64RegClassID = 22,
  GPR64noip_and_tcGPR64RegClassID = 23,
  GPR64argRegClassID = 24,
  rtcGPR64RegClassID = 25,
  GPR64sponlyRegClassID = 26,
  DDRegClassID = 27,
  XSeqPairsClassRegClassID = 28,
  XSeqPairsClass_with_subo64_in_GPR64commonRegClassID = 29,
  XSeqPairsClass_with_subo64_in_GPR64noipRegClassID = 30,
  XSeqPairsClass_with_sube64_in_GPR64noipRegClassID = 31,
  XSeqPairsClass_with_sube64_in_tcGPR64RegClassID = 32,
  XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClassID = 33,
  XSeqPairsClass_with_subo64_in_tcGPR64RegClassID = 34,
  XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClassID = 35,
  XSeqPairsClass_with_sub_32_in_GPR32argRegClassID = 36,
  XSeqPairsClass_with_sube64_in_rtcGPR64RegClassID = 37,
  FPR128RegClassID = 38,
  ZPRRegClassID = 39,
  FPR128_loRegClassID = 40,
  ZPR_4bRegClassID = 41,
  ZPR_3bRegClassID = 42,
  DDDRegClassID = 43,
  DDDDRegClassID = 44,
  QQRegClassID = 45,
  ZPR2RegClassID = 46,
  QQ_with_qsub0_in_FPR128_loRegClassID = 47,
  QQ_with_qsub1_in_FPR128_loRegClassID = 48,
  ZPR2_with_zsub1_in_ZPR_4bRegClassID = 49,
  ZPR2_with_zsub_in_FPR128_loRegClassID = 50,
  QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClassID = 51,
  ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClassID = 52,
  ZPR2_with_zsub0_in_ZPR_3bRegClassID = 53,
  ZPR2_with_zsub1_in_ZPR_3bRegClassID = 54,
  ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClassID = 55,
  QQQRegClassID = 56,
  ZPR3RegClassID = 57,
  QQQ_with_qsub0_in_FPR128_loRegClassID = 58,
  QQQ_with_qsub1_in_FPR128_loRegClassID = 59,
  QQQ_with_qsub2_in_FPR128_loRegClassID = 60,
  ZPR3_with_zsub1_in_ZPR_4bRegClassID = 61,
  ZPR3_with_zsub2_in_ZPR_4bRegClassID = 62,
  ZPR3_with_zsub_in_FPR128_loRegClassID = 63,
  QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClassID = 64,
  QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID = 65,
  ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID = 66,
  ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClassID = 67,
  QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID = 68,
  ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID = 69,
  ZPR3_with_zsub0_in_ZPR_3bRegClassID = 70,
  ZPR3_with_zsub1_in_ZPR_3bRegClassID = 71,
  ZPR3_with_zsub2_in_ZPR_3bRegClassID = 72,
  ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID = 73,
  ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClassID = 74,
  ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID = 75,
  QQQQRegClassID = 76,
  ZPR4RegClassID = 77,
  QQQQ_with_qsub0_in_FPR128_loRegClassID = 78,
  QQQQ_with_qsub1_in_FPR128_loRegClassID = 79,
  QQQQ_with_qsub2_in_FPR128_loRegClassID = 80,
  QQQQ_with_qsub3_in_FPR128_loRegClassID = 81,
  ZPR4_with_zsub1_in_ZPR_4bRegClassID = 82,
  ZPR4_with_zsub2_in_ZPR_4bRegClassID = 83,
  ZPR4_with_zsub3_in_ZPR_4bRegClassID = 84,
  ZPR4_with_zsub_in_FPR128_loRegClassID = 85,
  QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClassID = 86,
  QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID = 87,
  QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID = 88,
  ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID = 89,
  ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID = 90,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClassID = 91,
  QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID = 92,
  QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID = 93,
  ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID = 94,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID = 95,
  QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID = 96,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID = 97,
  ZPR4_with_zsub0_in_ZPR_3bRegClassID = 98,
  ZPR4_with_zsub1_in_ZPR_3bRegClassID = 99,
  ZPR4_with_zsub2_in_ZPR_3bRegClassID = 100,
  ZPR4_with_zsub3_in_ZPR_3bRegClassID = 101,
  ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID = 102,
  ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID = 103,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClassID = 104,
  ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID = 105,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID = 106,
  ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID = 107,

  };
} // end namespace AArch64


// Register alternate name indices

namespace AArch64 {
enum {
  NoRegAltName,	// 0
  vlist1,	// 1
  vreg,	// 2
  NUM_TARGET_REG_ALT_NAMES = 3
};
} // end namespace AArch64


// Subregister indices

namespace AArch64 {
enum {
  NoSubRegister,
  bsub,	// 1
  dsub,	// 2
  dsub0,	// 3
  dsub1,	// 4
  dsub2,	// 5
  dsub3,	// 6
  hsub,	// 7
  qhisub,	// 8
  qsub,	// 9
  qsub0,	// 10
  qsub1,	// 11
  qsub2,	// 12
  qsub3,	// 13
  ssub,	// 14
  sub_32,	// 15
  sube32,	// 16
  sube64,	// 17
  subo32,	// 18
  subo64,	// 19
  zsub,	// 20
  zsub0,	// 21
  zsub1,	// 22
  zsub2,	// 23
  zsub3,	// 24
  zsub_hi,	// 25
  dsub1_then_bsub,	// 26
  dsub1_then_hsub,	// 27
  dsub1_then_ssub,	// 28
  dsub3_then_bsub,	// 29
  dsub3_then_hsub,	// 30
  dsub3_then_ssub,	// 31
  dsub2_then_bsub,	// 32
  dsub2_then_hsub,	// 33
  dsub2_then_ssub,	// 34
  qsub1_then_bsub,	// 35
  qsub1_then_dsub,	// 36
  qsub1_then_hsub,	// 37
  qsub1_then_ssub,	// 38
  qsub3_then_bsub,	// 39
  qsub3_then_dsub,	// 40
  qsub3_then_hsub,	// 41
  qsub3_then_ssub,	// 42
  qsub2_then_bsub,	// 43
  qsub2_then_dsub,	// 44
  qsub2_then_hsub,	// 45
  qsub2_then_ssub,	// 46
  subo64_then_sub_32,	// 47
  zsub1_then_bsub,	// 48
  zsub1_then_dsub,	// 49
  zsub1_then_hsub,	// 50
  zsub1_then_ssub,	// 51
  zsub1_then_zsub,	// 52
  zsub1_then_zsub_hi,	// 53
  zsub3_then_bsub,	// 54
  zsub3_then_dsub,	// 55
  zsub3_then_hsub,	// 56
  zsub3_then_ssub,	// 57
  zsub3_then_zsub,	// 58
  zsub3_then_zsub_hi,	// 59
  zsub2_then_bsub,	// 60
  zsub2_then_dsub,	// 61
  zsub2_then_hsub,	// 62
  zsub2_then_ssub,	// 63
  zsub2_then_zsub,	// 64
  zsub2_then_zsub_hi,	// 65
  dsub0_dsub1,	// 66
  dsub0_dsub1_dsub2,	// 67
  dsub1_dsub2,	// 68
  dsub1_dsub2_dsub3,	// 69
  dsub2_dsub3,	// 70
  dsub_qsub1_then_dsub,	// 71
  dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub,	// 72
  dsub_qsub1_then_dsub_qsub2_then_dsub,	// 73
  qsub0_qsub1,	// 74
  qsub0_qsub1_qsub2,	// 75
  qsub1_qsub2,	// 76
  qsub1_qsub2_qsub3,	// 77
  qsub2_qsub3,	// 78
  qsub1_then_dsub_qsub2_then_dsub,	// 79
  qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub,	// 80
  qsub2_then_dsub_qsub3_then_dsub,	// 81
  sub_32_subo64_then_sub_32,	// 82
  dsub_zsub1_then_dsub,	// 83
  zsub_zsub1_then_zsub,	// 84
  dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub,	// 85
  dsub_zsub1_then_dsub_zsub2_then_dsub,	// 86
  zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub,	// 87
  zsub_zsub1_then_zsub_zsub2_then_zsub,	// 88
  zsub0_zsub1,	// 89
  zsub0_zsub1_zsub2,	// 90
  zsub1_zsub2,	// 91
  zsub1_zsub2_zsub3,	// 92
  zsub2_zsub3,	// 93
  zsub1_then_dsub_zsub2_then_dsub,	// 94
  zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub,	// 95
  zsub1_then_zsub_zsub2_then_zsub,	// 96
  zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub,	// 97
  zsub2_then_dsub_zsub3_then_dsub,	// 98
  zsub2_then_zsub_zsub3_then_zsub,	// 99
  NUM_TARGET_SUBREGS
};
} // end namespace AArch64

} // end namespace llvm

#endif // GET_REGINFO_ENUM

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* MC Register Information                                                    *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_MC_DESC
#undef GET_REGINFO_MC_DESC

namespace llvm {

extern const MCPhysReg AArch64RegDiffLists[] = {
  /* 0 */ 64977, 1, 1, 1, 74, 1, 1, 1, 0,
  /* 9 */ 65105, 1, 1, 1, 0,
  /* 14 */ 65201, 1, 1, 1, 0,
  /* 19 */ 6, 29, 1, 1, 0,
  /* 24 */ 6, 29, 1, 1, 46, 29, 1, 1, 0,
  /* 33 */ 64945, 1, 1, 75, 1, 1, 0,
  /* 40 */ 65073, 1, 1, 0,
  /* 44 */ 65169, 1, 1, 0,
  /* 48 */ 6, 1, 29, 1, 0,
  /* 53 */ 6, 1, 29, 1, 46, 1, 29, 1, 0,
  /* 62 */ 6, 30, 1, 0,
  /* 66 */ 6, 30, 1, 46, 30, 1, 0,
  /* 73 */ 65009, 1, 76, 1, 0,
  /* 78 */ 65204, 112, 65456, 65472, 33, 112, 65456, 65472, 33, 112, 65456, 65472, 298, 1, 0,
  /* 93 */ 65204, 112, 65456, 65472, 33, 112, 65456, 65472, 1, 112, 65456, 65472, 330, 1, 0,
  /* 108 */ 64552, 1, 0,
  /* 111 */ 64586, 1, 0,
  /* 114 */ 65137, 1, 0,
  /* 117 */ 65219, 1, 0,
  /* 120 */ 65220, 1, 0,
  /* 123 */ 65221, 1, 0,
  /* 126 */ 65222, 1, 0,
  /* 129 */ 65223, 1, 0,
  /* 132 */ 65224, 1, 0,
  /* 135 */ 65225, 1, 0,
  /* 138 */ 65226, 1, 0,
  /* 141 */ 65227, 1, 0,
  /* 144 */ 65228, 1, 0,
  /* 147 */ 65229, 1, 0,
  /* 150 */ 65230, 1, 0,
  /* 153 */ 65231, 1, 0,
  /* 156 */ 65232, 1, 0,
  /* 159 */ 65233, 1, 0,
  /* 162 */ 64, 80, 65424, 80, 124, 63, 1, 62, 65503, 34, 65503, 34, 65503, 1, 63, 1, 62, 65503, 34, 65503, 34, 65503, 1, 95, 1, 62, 65503, 34, 65503, 34, 65503, 1, 0,
  /* 195 */ 124, 159, 1, 62, 65503, 34, 65503, 34, 65503, 1, 95, 1, 62, 65503, 34, 65503, 34, 65503, 1, 0,
  /* 215 */ 65504, 287, 1, 62, 65503, 34, 65503, 34, 65503, 1, 0,
  /* 226 */ 64, 80, 65424, 80, 124, 64, 31, 33, 65504, 62, 65503, 34, 65503, 1, 33, 31, 33, 65504, 62, 65503, 34, 65503, 1, 65, 31, 33, 65504, 62, 65503, 34, 65503, 1, 0,
  /* 259 */ 124, 160, 31, 33, 65504, 62, 65503, 34, 65503, 1, 65, 31, 33, 65504, 62, 65503, 34, 65503, 1, 0,
  /* 279 */ 65504, 288, 31, 33, 65504, 62, 65503, 34, 65503, 1, 0,
  /* 290 */ 63, 65503, 34, 65503, 1, 64, 63, 65503, 34, 65503, 1, 96, 63, 65503, 34, 65503, 1, 0,
  /* 308 */ 64, 80, 65424, 80, 124, 63, 1, 63, 1, 65503, 1, 62, 65503, 1, 33, 1, 63, 1, 65503, 1, 62, 65503, 1, 65, 1, 63, 1, 65503, 1, 62, 65503, 1, 0,
  /* 341 */ 124, 159, 1, 63, 1, 65503, 1, 62, 65503, 1, 65, 1, 63, 1, 65503, 1, 62, 65503, 1, 0,
  /* 361 */ 65504, 287, 1, 63, 1, 65503, 1, 62, 65503, 1, 0,
  /* 372 */ 64, 65504, 63, 65503, 1, 33, 64, 65504, 63, 65503, 1, 65, 64, 65504, 63, 65503, 1, 0,
  /* 390 */ 65503, 1, 128, 65503, 1, 160, 65503, 1, 0,
  /* 399 */ 31, 272, 2, 0,
  /* 403 */ 65324, 514, 2, 0,
  /* 407 */ 2, 3, 0,
  /* 410 */ 65021, 3, 0,
  /* 413 */ 4, 0,
  /* 415 */ 5, 0,
  /* 417 */ 1, 493, 16, 0,
  /* 421 */ 65324, 498, 16, 0,
  /* 425 */ 31, 272, 17, 0,
  /* 429 */ 31, 273, 17, 0,
  /* 433 */ 31, 274, 17, 0,
  /* 437 */ 31, 275, 17, 0,
  /* 441 */ 31, 276, 17, 0,
  /* 445 */ 31, 277, 17, 0,
  /* 449 */ 31, 278, 17, 0,
  /* 453 */ 31, 279, 17, 0,
  /* 457 */ 31, 280, 17, 0,
  /* 461 */ 31, 281, 17, 0,
  /* 465 */ 31, 282, 17, 0,
  /* 469 */ 31, 283, 17, 0,
  /* 473 */ 31, 284, 17, 0,
  /* 477 */ 31, 285, 17, 0,
  /* 481 */ 31, 286, 17, 0,
  /* 485 */ 6, 1, 1, 29, 0,
  /* 490 */ 6, 1, 1, 29, 46, 1, 1, 29, 0,
  /* 499 */ 64, 80, 65424, 80, 124, 63, 1, 62, 1, 65503, 34, 65503, 1, 29, 34, 1, 62, 1, 65503, 34, 65503, 1, 29, 66, 1, 62, 1, 65503, 34, 65503, 1, 29, 0,
  /* 532 */ 124, 159, 1, 62, 1, 65503, 34, 65503, 1, 29, 66, 1, 62, 1, 65503, 34, 65503, 1, 29, 0,
  /* 552 */ 65504, 287, 1, 62, 1, 65503, 34, 65503, 1, 29, 0,
  /* 563 */ 6, 1, 30, 0,
  /* 567 */ 6, 1, 30, 46, 1, 30, 0,
  /* 574 */ 63, 1, 65503, 1, 30, 34, 63, 1, 65503, 1, 30, 66, 63, 1, 65503, 1, 30, 0,
  /* 592 */ 6, 31, 0,
  /* 595 */ 6, 31, 46, 31, 0,
  /* 600 */ 65504, 31, 97, 65504, 31, 129, 65504, 31, 0,
  /* 609 */ 65297, 77, 0,
  /* 612 */ 1, 81, 0,
  /* 615 */ 65021, 81, 0,
  /* 618 */ 65248, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 31, 96, 0,
  /* 635 */ 65248, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 63, 96, 0,
  /* 652 */ 65184, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 30, 96, 65504, 96, 64, 1, 65312, 96, 0,
  /* 682 */ 65184, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 62, 96, 65504, 96, 64, 1, 65312, 96, 0,
  /* 712 */ 65184, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 62, 96, 65504, 96, 64, 65505, 65312, 96, 0,
  /* 742 */ 65216, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 61, 96, 65472, 32, 64, 32, 64, 64, 65473, 64, 65441, 65343, 64, 32, 64, 65345, 96, 0,
  /* 788 */ 65216, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 61, 96, 65472, 32, 64, 32, 64, 64, 65441, 64, 65473, 65311, 64, 32, 64, 65377, 96, 0,
  /* 834 */ 65216, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 29, 96, 65472, 32, 64, 32, 64, 64, 65473, 64, 65473, 65311, 64, 32, 64, 65377, 96, 0,
  /* 880 */ 65216, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65505, 65412, 65456, 112, 65456, 65472, 268, 65473, 65412, 65456, 112, 65456, 65472, 268, 61, 96, 65472, 32, 64, 32, 64, 64, 65473, 64, 65473, 65311, 64, 32, 64, 65377, 96, 0,
  /* 926 */ 96, 128, 0,
  /* 929 */ 212, 0,
  /* 931 */ 65412, 65456, 112, 65456, 65472, 268, 0,
  /* 938 */ 274, 0,
  /* 940 */ 289, 0,
  /* 942 */ 290, 0,
  /* 944 */ 291, 0,
  /* 946 */ 292, 0,
  /* 948 */ 293, 0,
  /* 950 */ 294, 0,
  /* 952 */ 295, 0,
  /* 954 */ 296, 0,
  /* 956 */ 297, 0,
  /* 958 */ 298, 0,
  /* 960 */ 65252, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 299, 0,
  /* 972 */ 300, 0,
  /* 974 */ 301, 0,
  /* 976 */ 65262, 65505, 65325, 212, 302, 0,
  /* 982 */ 65246, 65505, 32, 65505, 303, 0,
  /* 988 */ 65245, 65505, 32, 65505, 304, 0,
  /* 994 */ 65244, 65505, 32, 65505, 305, 0,
  /* 1000 */ 65243, 65505, 32, 65505, 306, 0,
  /* 1006 */ 65242, 65505, 32, 65505, 307, 0,
  /* 1012 */ 65241, 65505, 32, 65505, 308, 0,
  /* 1018 */ 65240, 65505, 32, 65505, 309, 0,
  /* 1024 */ 65239, 65505, 32, 65505, 310, 0,
  /* 1030 */ 65238, 65505, 32, 65505, 311, 0,
  /* 1036 */ 65237, 65505, 32, 65505, 312, 0,
  /* 1042 */ 65236, 65505, 32, 65505, 313, 0,
  /* 1048 */ 65235, 65505, 32, 65505, 314, 0,
  /* 1054 */ 65234, 65505, 32, 65505, 315, 0,
  /* 1060 */ 65233, 65505, 32, 65505, 316, 0,
  /* 1066 */ 65252, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 331, 0,
  /* 1078 */ 65022, 212, 65329, 65535, 494, 0,
  /* 1084 */ 509, 0,
  /* 1086 */ 514, 0,
  /* 1088 */ 516, 0,
  /* 1090 */ 65323, 0,
  /* 1092 */ 65250, 65328, 0,
  /* 1095 */ 65342, 0,
  /* 1097 */ 65374, 0,
  /* 1099 */ 65389, 0,
  /* 1101 */ 65405, 0,
  /* 1103 */ 65421, 0,
  /* 1105 */ 65188, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 298, 64, 32, 1, 65440, 0,
  /* 1126 */ 65188, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 330, 64, 32, 1, 65440, 0,
  /* 1147 */ 65188, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 330, 64, 32, 65505, 65440, 0,
  /* 1168 */ 65220, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 329, 32, 32, 32, 64, 65473, 64, 65441, 65471, 64, 65441, 0,
  /* 1200 */ 65236, 112, 65456, 65472, 33, 112, 65456, 65472, 1, 112, 65456, 65472, 33, 112, 65456, 65472, 329, 64, 65473, 64, 65441, 0,
  /* 1222 */ 65469, 0,
  /* 1224 */ 65268, 112, 65456, 65472, 1, 112, 65456, 65472, 0,
  /* 1233 */ 65268, 112, 65456, 65472, 33, 112, 65456, 65472, 0,
  /* 1242 */ 65456, 112, 65456, 65472, 0,
  /* 1247 */ 65220, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 329, 32, 32, 32, 64, 65441, 64, 65473, 65439, 64, 65473, 0,
  /* 1279 */ 65220, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 297, 32, 32, 32, 64, 65473, 64, 65473, 65439, 64, 65473, 0,
  /* 1311 */ 65220, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 113, 65456, 112, 65456, 65472, 81, 65456, 112, 65456, 65472, 329, 32, 32, 32, 64, 65473, 64, 65473, 65439, 64, 65473, 0,
  /* 1343 */ 65236, 112, 65456, 65472, 1, 112, 65456, 65472, 33, 112, 65456, 65472, 33, 112, 65456, 65472, 329, 64, 65441, 64, 65473, 0,
  /* 1365 */ 65236, 112, 65456, 65472, 33, 112, 65456, 65472, 33, 112, 65456, 65472, 33, 112, 65456, 65472, 297, 64, 65473, 64, 65473, 0,
  /* 1387 */ 65236, 112, 65456, 65472, 33, 112, 65456, 65472, 33, 112, 65456, 65472, 1, 112, 65456, 65472, 329, 64, 65473, 64, 65473, 0,
  /* 1409 */ 65501, 0,
  /* 1411 */ 65204, 112, 65456, 65472, 1, 112, 65456, 65472, 33, 112, 65456, 65472, 330, 65505, 0,
  /* 1426 */ 65533, 0,
  /* 1428 */ 65535, 0,
};

extern const LaneBitmask AArch64LaneMaskLists[] = {
  /* 0 */ LaneBitmask(0x00000000), LaneBitmask::getAll(),
  /* 2 */ LaneBitmask(0x00000080), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 5 */ LaneBitmask(0x00000080), LaneBitmask(0x00000200), LaneBitmask(0x00000100), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 10 */ LaneBitmask(0x00000080), LaneBitmask(0x00000200), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 14 */ LaneBitmask(0x00000400), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 17 */ LaneBitmask(0x00000400), LaneBitmask(0x00001000), LaneBitmask(0x00000800), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 22 */ LaneBitmask(0x00000400), LaneBitmask(0x00001000), LaneBitmask(0x00000001), LaneBitmask::getAll(),
  /* 26 */ LaneBitmask(0x00002000), LaneBitmask(0x00000008), LaneBitmask::getAll(),
  /* 29 */ LaneBitmask(0x00000020), LaneBitmask(0x00000010), LaneBitmask::getAll(),
  /* 32 */ LaneBitmask(0x00000010), LaneBitmask(0x00000020), LaneBitmask::getAll(),
  /* 35 */ LaneBitmask(0x00000001), LaneBitmask(0x00000040), LaneBitmask::getAll(),
  /* 38 */ LaneBitmask(0x00004000), LaneBitmask(0x00000001), LaneBitmask(0x00008000), LaneBitmask(0x00000040), LaneBitmask::getAll(),
  /* 43 */ LaneBitmask(0x00004000), LaneBitmask(0x00040000), LaneBitmask(0x00010000), LaneBitmask(0x00000001), LaneBitmask(0x00008000), LaneBitmask(0x00080000), LaneBitmask(0x00020000), LaneBitmask(0x00000040), LaneBitmask::getAll(),
  /* 52 */ LaneBitmask(0x00004000), LaneBitmask(0x00040000), LaneBitmask(0x00000001), LaneBitmask(0x00008000), LaneBitmask(0x00080000), LaneBitmask(0x00000040), LaneBitmask::getAll(),
  /* 59 */ LaneBitmask(0x00000200), LaneBitmask(0x00000100), LaneBitmask(0x00000001), LaneBitmask(0x00000080), LaneBitmask::getAll(),
  /* 64 */ LaneBitmask(0x00000200), LaneBitmask(0x00000001), LaneBitmask(0x00000080), LaneBitmask::getAll(),
  /* 68 */ LaneBitmask(0x00000001), LaneBitmask(0x00000080), LaneBitmask(0x00000200), LaneBitmask(0x00000100), LaneBitmask::getAll(),
  /* 73 */ LaneBitmask(0x00000100), LaneBitmask(0x00000001), LaneBitmask(0x00000080), LaneBitmask(0x00000200), LaneBitmask::getAll(),
  /* 78 */ LaneBitmask(0x00001000), LaneBitmask(0x00000800), LaneBitmask(0x00000001), LaneBitmask(0x00000400), LaneBitmask::getAll(),
  /* 83 */ LaneBitmask(0x00001000), LaneBitmask(0x00000001), LaneBitmask(0x00000400), LaneBitmask::getAll(),
  /* 87 */ LaneBitmask(0x00000001), LaneBitmask(0x00000400), LaneBitmask(0x00001000), LaneBitmask(0x00000800), LaneBitmask::getAll(),
  /* 92 */ LaneBitmask(0x00000800), LaneBitmask(0x00000001), LaneBitmask(0x00000400), LaneBitmask(0x00001000), LaneBitmask::getAll(),
  /* 97 */ LaneBitmask(0x00000008), LaneBitmask(0x00002000), LaneBitmask::getAll(),
  /* 100 */ LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask::getAll(),
  /* 105 */ LaneBitmask(0x00040000), LaneBitmask(0x00010000), LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00080000), LaneBitmask(0x00020000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask::getAll(),
  /* 114 */ LaneBitmask(0x00040000), LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00080000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask::getAll(),
  /* 121 */ LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00040000), LaneBitmask(0x00010000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask(0x00080000), LaneBitmask(0x00020000), LaneBitmask::getAll(),
  /* 130 */ LaneBitmask(0x00010000), LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00040000), LaneBitmask(0x00020000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask(0x00080000), LaneBitmask::getAll(),
  /* 139 */ LaneBitmask(0x00000001), LaneBitmask(0x00004000), LaneBitmask(0x00040000), LaneBitmask(0x00000040), LaneBitmask(0x00008000), LaneBitmask(0x00080000), LaneBitmask::getAll(),
};

extern const uint16_t AArch64SubRegIdxLists[] = {
  /* 0 */ 2, 14, 7, 1, 0,
  /* 5 */ 15, 0,
  /* 7 */ 16, 18, 0,
  /* 10 */ 20, 2, 14, 7, 1, 25, 0,
  /* 17 */ 3, 14, 7, 1, 4, 28, 27, 26, 0,
  /* 26 */ 3, 14, 7, 1, 4, 28, 27, 26, 5, 34, 33, 32, 66, 68, 0,
  /* 41 */ 3, 14, 7, 1, 4, 28, 27, 26, 5, 34, 33, 32, 6, 31, 30, 29, 66, 67, 68, 69, 70, 0,
  /* 63 */ 10, 2, 14, 7, 1, 11, 36, 38, 37, 35, 71, 0,
  /* 75 */ 10, 2, 14, 7, 1, 11, 36, 38, 37, 35, 12, 44, 46, 45, 43, 71, 73, 74, 76, 79, 0,
  /* 96 */ 10, 2, 14, 7, 1, 11, 36, 38, 37, 35, 12, 44, 46, 45, 43, 13, 40, 42, 41, 39, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 0,
  /* 128 */ 17, 15, 19, 47, 82, 0,
  /* 134 */ 21, 20, 2, 14, 7, 1, 25, 22, 52, 49, 51, 50, 48, 53, 83, 84, 0,
  /* 151 */ 21, 20, 2, 14, 7, 1, 25, 22, 52, 49, 51, 50, 48, 53, 23, 64, 61, 63, 62, 60, 65, 83, 84, 86, 88, 89, 91, 94, 96, 0,
  /* 181 */ 21, 20, 2, 14, 7, 1, 25, 22, 52, 49, 51, 50, 48, 53, 23, 64, 61, 63, 62, 60, 65, 24, 58, 55, 57, 56, 54, 59, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 0,
};

extern const MCRegisterInfo::SubRegCoveredBits AArch64SubRegIdxRanges[] = {
  { 65535, 65535 },
  { 0, 8 },	// bsub
  { 0, 32 },	// dsub
  { 0, 64 },	// dsub0
  { 0, 64 },	// dsub1
  { 0, 64 },	// dsub2
  { 0, 64 },	// dsub3
  { 0, 16 },	// hsub
  { 0, 64 },	// qhisub
  { 0, 64 },	// qsub
  { 0, 128 },	// qsub0
  { 0, 128 },	// qsub1
  { 0, 128 },	// qsub2
  { 0, 128 },	// qsub3
  { 0, 32 },	// ssub
  { 0, 32 },	// sub_32
  { 0, 32 },	// sube32
  { 0, 64 },	// sube64
  { 0, 32 },	// subo32
  { 0, 64 },	// subo64
  { 0, 128 },	// zsub
  { 65535, 128 },	// zsub0
  { 65535, 128 },	// zsub1
  { 65535, 128 },	// zsub2
  { 65535, 128 },	// zsub3
  { 0, 128 },	// zsub_hi
  { 0, 8 },	// dsub1_then_bsub
  { 0, 16 },	// dsub1_then_hsub
  { 0, 32 },	// dsub1_then_ssub
  { 0, 8 },	// dsub3_then_bsub
  { 0, 16 },	// dsub3_then_hsub
  { 0, 32 },	// dsub3_then_ssub
  { 0, 8 },	// dsub2_then_bsub
  { 0, 16 },	// dsub2_then_hsub
  { 0, 32 },	// dsub2_then_ssub
  { 0, 8 },	// qsub1_then_bsub
  { 0, 32 },	// qsub1_then_dsub
  { 0, 16 },	// qsub1_then_hsub
  { 0, 32 },	// qsub1_then_ssub
  { 0, 8 },	// qsub3_then_bsub
  { 0, 32 },	// qsub3_then_dsub
  { 0, 16 },	// qsub3_then_hsub
  { 0, 32 },	// qsub3_then_ssub
  { 0, 8 },	// qsub2_then_bsub
  { 0, 32 },	// qsub2_then_dsub
  { 0, 16 },	// qsub2_then_hsub
  { 0, 32 },	// qsub2_then_ssub
  { 0, 32 },	// subo64_then_sub_32
  { 65535, 65535 },	// zsub1_then_bsub
  { 65535, 65535 },	// zsub1_then_dsub
  { 65535, 65535 },	// zsub1_then_hsub
  { 65535, 65535 },	// zsub1_then_ssub
  { 65535, 65535 },	// zsub1_then_zsub
  { 65535, 65535 },	// zsub1_then_zsub_hi
  { 65535, 65535 },	// zsub3_then_bsub
  { 65535, 65535 },	// zsub3_then_dsub
  { 65535, 65535 },	// zsub3_then_hsub
  { 65535, 65535 },	// zsub3_then_ssub
  { 65535, 65535 },	// zsub3_then_zsub
  { 65535, 65535 },	// zsub3_then_zsub_hi
  { 65535, 65535 },	// zsub2_then_bsub
  { 65535, 65535 },	// zsub2_then_dsub
  { 65535, 65535 },	// zsub2_then_hsub
  { 65535, 65535 },	// zsub2_then_ssub
  { 65535, 65535 },	// zsub2_then_zsub
  { 65535, 65535 },	// zsub2_then_zsub_hi
  { 65535, 128 },	// dsub0_dsub1
  { 65535, 192 },	// dsub0_dsub1_dsub2
  { 65535, 128 },	// dsub1_dsub2
  { 65535, 192 },	// dsub1_dsub2_dsub3
  { 65535, 128 },	// dsub2_dsub3
  { 65535, 64 },	// dsub_qsub1_then_dsub
  { 65535, 128 },	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  { 65535, 96 },	// dsub_qsub1_then_dsub_qsub2_then_dsub
  { 65535, 256 },	// qsub0_qsub1
  { 65535, 384 },	// qsub0_qsub1_qsub2
  { 65535, 256 },	// qsub1_qsub2
  { 65535, 384 },	// qsub1_qsub2_qsub3
  { 65535, 256 },	// qsub2_qsub3
  { 65535, 64 },	// qsub1_then_dsub_qsub2_then_dsub
  { 65535, 96 },	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  { 65535, 64 },	// qsub2_then_dsub_qsub3_then_dsub
  { 65535, 64 },	// sub_32_subo64_then_sub_32
  { 65535, 31 },	// dsub_zsub1_then_dsub
  { 65535, 127 },	// zsub_zsub1_then_zsub
  { 65535, 29 },	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
  { 65535, 30 },	// dsub_zsub1_then_dsub_zsub2_then_dsub
  { 65535, 125 },	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
  { 65535, 126 },	// zsub_zsub1_then_zsub_zsub2_then_zsub
  { 65535, 256 },	// zsub0_zsub1
  { 65535, 384 },	// zsub0_zsub1_zsub2
  { 65535, 256 },	// zsub1_zsub2
  { 65535, 384 },	// zsub1_zsub2_zsub3
  { 65535, 256 },	// zsub2_zsub3
  { 65535, 65534 },	// zsub1_then_dsub_zsub2_then_dsub
  { 65535, 65533 },	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
  { 65535, 65534 },	// zsub1_then_zsub_zsub2_then_zsub
  { 65535, 65533 },	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
  { 65535, 65534 },	// zsub2_then_dsub_zsub3_then_dsub
  { 65535, 65534 },	// zsub2_then_zsub_zsub3_then_zsub
};

extern const char AArch64RegStrings[] = {
  /* 0 */ 'B', '1', '0', 0,
  /* 4 */ 'D', '7', '_', 'D', '8', '_', 'D', '9', '_', 'D', '1', '0', 0,
  /* 17 */ 'H', '1', '0', 0,
  /* 21 */ 'P', '1', '0', 0,
  /* 25 */ 'Q', '7', '_', 'Q', '8', '_', 'Q', '9', '_', 'Q', '1', '0', 0,
  /* 38 */ 'S', '1', '0', 0,
  /* 42 */ 'W', '1', '0', 0,
  /* 46 */ 'X', '1', '0', 0,
  /* 50 */ 'Z', '7', '_', 'Z', '8', '_', 'Z', '9', '_', 'Z', '1', '0', 0,
  /* 63 */ 'B', '2', '0', 0,
  /* 67 */ 'D', '1', '7', '_', 'D', '1', '8', '_', 'D', '1', '9', '_', 'D', '2', '0', 0,
  /* 83 */ 'H', '2', '0', 0,
  /* 87 */ 'Q', '1', '7', '_', 'Q', '1', '8', '_', 'Q', '1', '9', '_', 'Q', '2', '0', 0,
  /* 103 */ 'S', '2', '0', 0,
  /* 107 */ 'W', '2', '0', 0,
  /* 111 */ 'X', '2', '0', 0,
  /* 115 */ 'Z', '1', '7', '_', 'Z', '1', '8', '_', 'Z', '1', '9', '_', 'Z', '2', '0', 0,
  /* 131 */ 'B', '3', '0', 0,
  /* 135 */ 'D', '2', '7', '_', 'D', '2', '8', '_', 'D', '2', '9', '_', 'D', '3', '0', 0,
  /* 151 */ 'H', '3', '0', 0,
  /* 155 */ 'Q', '2', '7', '_', 'Q', '2', '8', '_', 'Q', '2', '9', '_', 'Q', '3', '0', 0,
  /* 171 */ 'S', '3', '0', 0,
  /* 175 */ 'W', '3', '0', 0,
  /* 179 */ 'Z', '2', '7', '_', 'Z', '2', '8', '_', 'Z', '2', '9', '_', 'Z', '3', '0', 0,
  /* 195 */ 'B', '0', 0,
  /* 198 */ 'D', '2', '9', '_', 'D', '3', '0', '_', 'D', '3', '1', '_', 'D', '0', 0,
  /* 213 */ 'H', '0', 0,
  /* 216 */ 'P', '0', 0,
  /* 219 */ 'Q', '2', '9', '_', 'Q', '3', '0', '_', 'Q', '3', '1', '_', 'Q', '0', 0,
  /* 234 */ 'S', '0', 0,
  /* 237 */ 'W', '0', 0,
  /* 240 */ 'X', '0', 0,
  /* 243 */ 'Z', '2', '9', '_', 'Z', '3', '0', '_', 'Z', '3', '1', '_', 'Z', '0', 0,
  /* 258 */ 'B', '1', '1', 0,
  /* 262 */ 'D', '8', '_', 'D', '9', '_', 'D', '1', '0', '_', 'D', '1', '1', 0,
  /* 276 */ 'H', '1', '1', 0,
  /* 280 */ 'P', '1', '1', 0,
  /* 284 */ 'Q', '8', '_', 'Q', '9', '_', 'Q', '1', '0', '_', 'Q', '1', '1', 0,
  /* 298 */ 'S', '1', '1', 0,
  /* 302 */ 'W', '1', '0', '_', 'W', '1', '1', 0,
  /* 310 */ 'X', '1', '0', '_', 'X', '1', '1', 0,
  /* 318 */ 'Z', '8', '_', 'Z', '9', '_', 'Z', '1', '0', '_', 'Z', '1', '1', 0,
  /* 332 */ 'B', '2', '1', 0,
  /* 336 */ 'D', '1', '8', '_', 'D', '1', '9', '_', 'D', '2', '0', '_', 'D', '2', '1', 0,
  /* 352 */ 'H', '2', '1', 0,
  /* 356 */ 'Q', '1', '8', '_', 'Q', '1', '9', '_', 'Q', '2', '0', '_', 'Q', '2', '1', 0,
  /* 372 */ 'S', '2', '1', 0,
  /* 376 */ 'W', '2', '0', '_', 'W', '2', '1', 0,
  /* 384 */ 'X', '2', '0', '_', 'X', '2', '1', 0,
  /* 392 */ 'Z', '1', '8', '_', 'Z', '1', '9', '_', 'Z', '2', '0', '_', 'Z', '2', '1', 0,
  /* 408 */ 'B', '3', '1', 0,
  /* 412 */ 'D', '2', '8', '_', 'D', '2', '9', '_', 'D', '3', '0', '_', 'D', '3', '1', 0,
  /* 428 */ 'H', '3', '1', 0,
  /* 432 */ 'Q', '2', '8', '_', 'Q', '2', '9', '_', 'Q', '3', '0', '_', 'Q', '3', '1', 0,
  /* 448 */ 'S', '3', '1', 0,
  /* 452 */ 'Z', '2', '8', '_', 'Z', '2', '9', '_', 'Z', '3', '0', '_', 'Z', '3', '1', 0,
  /* 468 */ 'B', '1', 0,
  /* 471 */ 'D', '3', '0', '_', 'D', '3', '1', '_', 'D', '0', '_', 'D', '1', 0,
  /* 485 */ 'H', '1', 0,
  /* 488 */ 'P', '1', 0,
  /* 491 */ 'Q', '3', '0', '_', 'Q', '3', '1', '_', 'Q', '0', '_', 'Q', '1', 0,
  /* 505 */ 'S', '1', 0,
  /* 508 */ 'W', '0', '_', 'W', '1', 0,
  /* 514 */ 'X', '0', '_', 'X', '1', 0,
  /* 520 */ 'Z', '3', '0', '_', 'Z', '3', '1', '_', 'Z', '0', '_', 'Z', '1', 0,
  /* 534 */ 'B', '1', '2', 0,
  /* 538 */ 'D', '9', '_', 'D', '1', '0', '_', 'D', '1', '1', '_', 'D', '1', '2', 0,
  /* 553 */ 'H', '1', '2', 0,
  /* 557 */ 'P', '1', '2', 0,
  /* 561 */ 'Q', '9', '_', 'Q', '1', '0', '_', 'Q', '1', '1', '_', 'Q', '1', '2', 0,
  /* 576 */ 'S', '1', '2', 0,
  /* 580 */ 'W', '1', '2', 0,
  /* 584 */ 'X', '1', '2', 0,
  /* 588 */ 'Z', '9', '_', 'Z', '1', '0', '_', 'Z', '1', '1', '_', 'Z', '1', '2', 0,
  /* 603 */ 'B', '2', '2', 0,
  /* 607 */ 'D', '1', '9', '_', 'D', '2', '0', '_', 'D', '2', '1', '_', 'D', '2', '2', 0,
  /* 623 */ 'H', '2', '2', 0,
  /* 627 */ 'Q', '1', '9', '_', 'Q', '2', '0', '_', 'Q', '2', '1', '_', 'Q', '2', '2', 0,
  /* 643 */ 'S', '2', '2', 0,
  /* 647 */ 'W', '2', '2', 0,
  /* 651 */ 'X', '2', '2', 0,
  /* 655 */ 'Z', '1', '9', '_', 'Z', '2', '0', '_', 'Z', '2', '1', '_', 'Z', '2', '2', 0,
  /* 671 */ 'B', '2', 0,
  /* 674 */ 'D', '3', '1', '_', 'D', '0', '_', 'D', '1', '_', 'D', '2', 0,
  /* 687 */ 'H', '2', 0,
  /* 690 */ 'P', '2', 0,
  /* 693 */ 'Q', '3', '1', '_', 'Q', '0', '_', 'Q', '1', '_', 'Q', '2', 0,
  /* 706 */ 'S', '2', 0,
  /* 709 */ 'W', '2', 0,
  /* 712 */ 'X', '2', 0,
  /* 715 */ 'Z', '3', '1', '_', 'Z', '0', '_', 'Z', '1', '_', 'Z', '2', 0,
  /* 728 */ 'B', '1', '3', 0,
  /* 732 */ 'D', '1', '0', '_', 'D', '1', '1', '_', 'D', '1', '2', '_', 'D', '1', '3', 0,
  /* 748 */ 'H', '1', '3', 0,
  /* 752 */ 'P', '1', '3', 0,
  /* 756 */ 'Q', '1', '0', '_', 'Q', '1', '1', '_', 'Q', '1', '2', '_', 'Q', '1', '3', 0,
  /* 772 */ 'S', '1', '3', 0,
  /* 776 */ 'W', '1', '2', '_', 'W', '1', '3', 0,
  /* 784 */ 'X', '1', '2', '_', 'X', '1', '3', 0,
  /* 792 */ 'Z', '1', '0', '_', 'Z', '1', '1', '_', 'Z', '1', '2', '_', 'Z', '1', '3', 0,
  /* 808 */ 'B', '2', '3', 0,
  /* 812 */ 'D', '2', '0', '_', 'D', '2', '1', '_', 'D', '2', '2', '_', 'D', '2', '3', 0,
  /* 828 */ 'H', '2', '3', 0,
  /* 832 */ 'Q', '2', '0', '_', 'Q', '2', '1', '_', 'Q', '2', '2', '_', 'Q', '2', '3', 0,
  /* 848 */ 'S', '2', '3', 0,
  /* 852 */ 'W', '2', '2', '_', 'W', '2', '3', 0,
  /* 860 */ 'X', '2', '2', '_', 'X', '2', '3', 0,
  /* 868 */ 'Z', '2', '0', '_', 'Z', '2', '1', '_', 'Z', '2', '2', '_', 'Z', '2', '3', 0,
  /* 884 */ 'B', '3', 0,
  /* 887 */ 'D', '0', '_', 'D', '1', '_', 'D', '2', '_', 'D', '3', 0,
  /* 899 */ 'H', '3', 0,
  /* 902 */ 'P', '3', 0,
  /* 905 */ 'Q', '0', '_', 'Q', '1', '_', 'Q', '2', '_', 'Q', '3', 0,
  /* 917 */ 'S', '3', 0,
  /* 920 */ 'W', '2', '_', 'W', '3', 0,
  /* 926 */ 'X', '2', '_', 'X', '3', 0,
  /* 932 */ 'Z', '0', '_', 'Z', '1', '_', 'Z', '2', '_', 'Z', '3', 0,
  /* 944 */ 'B', '1', '4', 0,
  /* 948 */ 'D', '1', '1', '_', 'D', '1', '2', '_', 'D', '1', '3', '_', 'D', '1', '4', 0,
  /* 964 */ 'H', '1', '4', 0,
  /* 968 */ 'P', '1', '4', 0,
  /* 972 */ 'Q', '1', '1', '_', 'Q', '1', '2', '_', 'Q', '1', '3', '_', 'Q', '1', '4', 0,
  /* 988 */ 'S', '1', '4', 0,
  /* 992 */ 'W', '1', '4', 0,
  /* 996 */ 'X', '1', '4', 0,
  /* 1000 */ 'Z', '1', '1', '_', 'Z', '1', '2', '_', 'Z', '1', '3', '_', 'Z', '1', '4', 0,
  /* 1016 */ 'B', '2', '4', 0,
  /* 1020 */ 'D', '2', '1', '_', 'D', '2', '2', '_', 'D', '2', '3', '_', 'D', '2', '4', 0,
  /* 1036 */ 'H', '2', '4', 0,
  /* 1040 */ 'Q', '2', '1', '_', 'Q', '2', '2', '_', 'Q', '2', '3', '_', 'Q', '2', '4', 0,
  /* 1056 */ 'S', '2', '4', 0,
  /* 1060 */ 'W', '2', '4', 0,
  /* 1064 */ 'X', '2', '4', 0,
  /* 1068 */ 'Z', '2', '1', '_', 'Z', '2', '2', '_', 'Z', '2', '3', '_', 'Z', '2', '4', 0,
  /* 1084 */ 'B', '4', 0,
  /* 1087 */ 'D', '1', '_', 'D', '2', '_', 'D', '3', '_', 'D', '4', 0,
  /* 1099 */ 'H', '4', 0,
  /* 1102 */ 'P', '4', 0,
  /* 1105 */ 'Q', '1', '_', 'Q', '2', '_', 'Q', '3', '_', 'Q', '4', 0,
  /* 1117 */ 'S', '4', 0,
  /* 1120 */ 'W', '4', 0,
  /* 1123 */ 'X', '4', 0,
  /* 1126 */ 'Z', '1', '_', 'Z', '2', '_', 'Z', '3', '_', 'Z', '4', 0,
  /* 1138 */ 'B', '1', '5', 0,
  /* 1142 */ 'D', '1', '2', '_', 'D', '1', '3', '_', 'D', '1', '4', '_', 'D', '1', '5', 0,
  /* 1158 */ 'H', '1', '5', 0,
  /* 1162 */ 'P', '1', '5', 0,
  /* 1166 */ 'Q', '1', '2', '_', 'Q', '1', '3', '_', 'Q', '1', '4', '_', 'Q', '1', '5', 0,
  /* 1182 */ 'S', '1', '5', 0,
  /* 1186 */ 'W', '1', '4', '_', 'W', '1', '5', 0,
  /* 1194 */ 'X', '1', '4', '_', 'X', '1', '5', 0,
  /* 1202 */ 'Z', '1', '2', '_', 'Z', '1', '3', '_', 'Z', '1', '4', '_', 'Z', '1', '5', 0,
  /* 1218 */ 'B', '2', '5', 0,
  /* 1222 */ 'D', '2', '2', '_', 'D', '2', '3', '_', 'D', '2', '4', '_', 'D', '2', '5', 0,
  /* 1238 */ 'H', '2', '5', 0,
  /* 1242 */ 'Q', '2', '2', '_', 'Q', '2', '3', '_', 'Q', '2', '4', '_', 'Q', '2', '5', 0,
  /* 1258 */ 'S', '2', '5', 0,
  /* 1262 */ 'W', '2', '4', '_', 'W', '2', '5', 0,
  /* 1270 */ 'X', '2', '4', '_', 'X', '2', '5', 0,
  /* 1278 */ 'Z', '2', '2', '_', 'Z', '2', '3', '_', 'Z', '2', '4', '_', 'Z', '2', '5', 0,
  /* 1294 */ 'B', '5', 0,
  /* 1297 */ 'D', '2', '_', 'D', '3', '_', 'D', '4', '_', 'D', '5', 0,
  /* 1309 */ 'H', '5', 0,
  /* 1312 */ 'P', '5', 0,
  /* 1315 */ 'Q', '2', '_', 'Q', '3', '_', 'Q', '4', '_', 'Q', '5', 0,
  /* 1327 */ 'S', '5', 0,
  /* 1330 */ 'W', '4', '_', 'W', '5', 0,
  /* 1336 */ 'X', '4', '_', 'X', '5', 0,
  /* 1342 */ 'Z', '2', '_', 'Z', '3', '_', 'Z', '4', '_', 'Z', '5', 0,
  /* 1354 */ 'B', '1', '6', 0,
  /* 1358 */ 'D', '1', '3', '_', 'D', '1', '4', '_', 'D', '1', '5', '_', 'D', '1', '6', 0,
  /* 1374 */ 'H', '1', '6', 0,
  /* 1378 */ 'Q', '1', '3', '_', 'Q', '1', '4', '_', 'Q', '1', '5', '_', 'Q', '1', '6', 0,
  /* 1394 */ 'S', '1', '6', 0,
  /* 1398 */ 'W', '1', '6', 0,
  /* 1402 */ 'X', '1', '6', 0,
  /* 1406 */ 'Z', '1', '3', '_', 'Z', '1', '4', '_', 'Z', '1', '5', '_', 'Z', '1', '6', 0,
  /* 1422 */ 'B', '2', '6', 0,
  /* 1426 */ 'D', '2', '3', '_', 'D', '2', '4', '_', 'D', '2', '5', '_', 'D', '2', '6', 0,
  /* 1442 */ 'H', '2', '6', 0,
  /* 1446 */ 'Q', '2', '3', '_', 'Q', '2', '4', '_', 'Q', '2', '5', '_', 'Q', '2', '6', 0,
  /* 1462 */ 'S', '2', '6', 0,
  /* 1466 */ 'W', '2', '6', 0,
  /* 1470 */ 'X', '2', '6', 0,
  /* 1474 */ 'Z', '2', '3', '_', 'Z', '2', '4', '_', 'Z', '2', '5', '_', 'Z', '2', '6', 0,
  /* 1490 */ 'B', '6', 0,
  /* 1493 */ 'D', '3', '_', 'D', '4', '_', 'D', '5', '_', 'D', '6', 0,
  /* 1505 */ 'H', '6', 0,
  /* 1508 */ 'P', '6', 0,
  /* 1511 */ 'Q', '3', '_', 'Q', '4', '_', 'Q', '5', '_', 'Q', '6', 0,
  /* 1523 */ 'S', '6', 0,
  /* 1526 */ 'W', '6', 0,
  /* 1529 */ 'X', '6', 0,
  /* 1532 */ 'Z', '3', '_', 'Z', '4', '_', 'Z', '5', '_', 'Z', '6', 0,
  /* 1544 */ 'B', '1', '7', 0,
  /* 1548 */ 'D', '1', '4', '_', 'D', '1', '5', '_', 'D', '1', '6', '_', 'D', '1', '7', 0,
  /* 1564 */ 'H', '1', '7', 0,
  /* 1568 */ 'Q', '1', '4', '_', 'Q', '1', '5', '_', 'Q', '1', '6', '_', 'Q', '1', '7', 0,
  /* 1584 */ 'S', '1', '7', 0,
  /* 1588 */ 'W', '1', '6', '_', 'W', '1', '7', 0,
  /* 1596 */ 'X', '1', '6', '_', 'X', '1', '7', 0,
  /* 1604 */ 'Z', '1', '4', '_', 'Z', '1', '5', '_', 'Z', '1', '6', '_', 'Z', '1', '7', 0,
  /* 1620 */ 'B', '2', '7', 0,
  /* 1624 */ 'D', '2', '4', '_', 'D', '2', '5', '_', 'D', '2', '6', '_', 'D', '2', '7', 0,
  /* 1640 */ 'H', '2', '7', 0,
  /* 1644 */ 'Q', '2', '4', '_', 'Q', '2', '5', '_', 'Q', '2', '6', '_', 'Q', '2', '7', 0,
  /* 1660 */ 'S', '2', '7', 0,
  /* 1664 */ 'W', '2', '6', '_', 'W', '2', '7', 0,
  /* 1672 */ 'X', '2', '6', '_', 'X', '2', '7', 0,
  /* 1680 */ 'Z', '2', '4', '_', 'Z', '2', '5', '_', 'Z', '2', '6', '_', 'Z', '2', '7', 0,
  /* 1696 */ 'B', '7', 0,
  /* 1699 */ 'D', '4', '_', 'D', '5', '_', 'D', '6', '_', 'D', '7', 0,
  /* 1711 */ 'H', '7', 0,
  /* 1714 */ 'P', '7', 0,
  /* 1717 */ 'Q', '4', '_', 'Q', '5', '_', 'Q', '6', '_', 'Q', '7', 0,
  /* 1729 */ 'S', '7', 0,
  /* 1732 */ 'W', '6', '_', 'W', '7', 0,
  /* 1738 */ 'X', '6', '_', 'X', '7', 0,
  /* 1744 */ 'Z', '4', '_', 'Z', '5', '_', 'Z', '6', '_', 'Z', '7', 0,
  /* 1756 */ 'B', '1', '8', 0,
  /* 1760 */ 'D', '1', '5', '_', 'D', '1', '6', '_', 'D', '1', '7', '_', 'D', '1', '8', 0,
  /* 1776 */ 'H', '1', '8', 0,
  /* 1780 */ 'Q', '1', '5', '_', 'Q', '1', '6', '_', 'Q', '1', '7', '_', 'Q', '1', '8', 0,
  /* 1796 */ 'S', '1', '8', 0,
  /* 1800 */ 'W', '1', '8', 0,
  /* 1804 */ 'X', '1', '8', 0,
  /* 1808 */ 'Z', '1', '5', '_', 'Z', '1', '6', '_', 'Z', '1', '7', '_', 'Z', '1', '8', 0,
  /* 1824 */ 'B', '2', '8', 0,
  /* 1828 */ 'D', '2', '5', '_', 'D', '2', '6', '_', 'D', '2', '7', '_', 'D', '2', '8', 0,
  /* 1844 */ 'H', '2', '8', 0,
  /* 1848 */ 'Q', '2', '5', '_', 'Q', '2', '6', '_', 'Q', '2', '7', '_', 'Q', '2', '8', 0,
  /* 1864 */ 'S', '2', '8', 0,
  /* 1868 */ 'W', '2', '8', 0,
  /* 1872 */ 'X', '2', '8', 0,
  /* 1876 */ 'Z', '2', '5', '_', 'Z', '2', '6', '_', 'Z', '2', '7', '_', 'Z', '2', '8', 0,
  /* 1892 */ 'B', '8', 0,
  /* 1895 */ 'D', '5', '_', 'D', '6', '_', 'D', '7', '_', 'D', '8', 0,
  /* 1907 */ 'H', '8', 0,
  /* 1910 */ 'P', '8', 0,
  /* 1913 */ 'Q', '5', '_', 'Q', '6', '_', 'Q', '7', '_', 'Q', '8', 0,
  /* 1925 */ 'S', '8', 0,
  /* 1928 */ 'W', '8', 0,
  /* 1931 */ 'X', '8', 0,
  /* 1934 */ 'Z', '5', '_', 'Z', '6', '_', 'Z', '7', '_', 'Z', '8', 0,
  /* 1946 */ 'B', '1', '9', 0,
  /* 1950 */ 'D', '1', '6', '_', 'D', '1', '7', '_', 'D', '1', '8', '_', 'D', '1', '9', 0,
  /* 1966 */ 'H', '1', '9', 0,
  /* 1970 */ 'Q', '1', '6', '_', 'Q', '1', '7', '_', 'Q', '1', '8', '_', 'Q', '1', '9', 0,
  /* 1986 */ 'S', '1', '9', 0,
  /* 1990 */ 'W', '1', '8', '_', 'W', '1', '9', 0,
  /* 1998 */ 'X', '1', '8', '_', 'X', '1', '9', 0,
  /* 2006 */ 'Z', '1', '6', '_', 'Z', '1', '7', '_', 'Z', '1', '8', '_', 'Z', '1', '9', 0,
  /* 2022 */ 'B', '2', '9', 0,
  /* 2026 */ 'D', '2', '6', '_', 'D', '2', '7', '_', 'D', '2', '8', '_', 'D', '2', '9', 0,
  /* 2042 */ 'H', '2', '9', 0,
  /* 2046 */ 'Q', '2', '6', '_', 'Q', '2', '7', '_', 'Q', '2', '8', '_', 'Q', '2', '9', 0,
  /* 2062 */ 'S', '2', '9', 0,
  /* 2066 */ 'W', '2', '8', '_', 'W', '2', '9', 0,
  /* 2074 */ 'Z', '2', '6', '_', 'Z', '2', '7', '_', 'Z', '2', '8', '_', 'Z', '2', '9', 0,
  /* 2090 */ 'B', '9', 0,
  /* 2093 */ 'D', '6', '_', 'D', '7', '_', 'D', '8', '_', 'D', '9', 0,
  /* 2105 */ 'H', '9', 0,
  /* 2108 */ 'P', '9', 0,
  /* 2111 */ 'Q', '6', '_', 'Q', '7', '_', 'Q', '8', '_', 'Q', '9', 0,
  /* 2123 */ 'S', '9', 0,
  /* 2126 */ 'W', '8', '_', 'W', '9', 0,
  /* 2132 */ 'X', '8', '_', 'X', '9', 0,
  /* 2138 */ 'Z', '6', '_', 'Z', '7', '_', 'Z', '8', '_', 'Z', '9', 0,
  /* 2150 */ 'Z', '1', '0', '_', 'H', 'I', 0,
  /* 2157 */ 'Z', '2', '0', '_', 'H', 'I', 0,
  /* 2164 */ 'Z', '3', '0', '_', 'H', 'I', 0,
  /* 2171 */ 'Z', '0', '_', 'H', 'I', 0,
  /* 2177 */ 'Z', '1', '1', '_', 'H', 'I', 0,
  /* 2184 */ 'Z', '2', '1', '_', 'H', 'I', 0,
  /* 2191 */ 'Z', '3', '1', '_', 'H', 'I', 0,
  /* 2198 */ 'Z', '1', '_', 'H', 'I', 0,
  /* 2204 */ 'Z', '1', '2', '_', 'H', 'I', 0,
  /* 2211 */ 'Z', '2', '2', '_', 'H', 'I', 0,
  /* 2218 */ 'Z', '2', '_', 'H', 'I', 0,
  /* 2224 */ 'Z', '1', '3', '_', 'H', 'I', 0,
  /* 2231 */ 'Z', '2', '3', '_', 'H', 'I', 0,
  /* 2238 */ 'Z', '3', '_', 'H', 'I', 0,
  /* 2244 */ 'Z', '1', '4', '_', 'H', 'I', 0,
  /* 2251 */ 'Z', '2', '4', '_', 'H', 'I', 0,
  /* 2258 */ 'Z', '4', '_', 'H', 'I', 0,
  /* 2264 */ 'Z', '1', '5', '_', 'H', 'I', 0,
  /* 2271 */ 'Z', '2', '5', '_', 'H', 'I', 0,
  /* 2278 */ 'Z', '5', '_', 'H', 'I', 0,
  /* 2284 */ 'Z', '1', '6', '_', 'H', 'I', 0,
  /* 2291 */ 'Z', '2', '6', '_', 'H', 'I', 0,
  /* 2298 */ 'Z', '6', '_', 'H', 'I', 0,
  /* 2304 */ 'Z', '1', '7', '_', 'H', 'I', 0,
  /* 2311 */ 'Z', '2', '7', '_', 'H', 'I', 0,
  /* 2318 */ 'Z', '7', '_', 'H', 'I', 0,
  /* 2324 */ 'Z', '1', '8', '_', 'H', 'I', 0,
  /* 2331 */ 'Z', '2', '8', '_', 'H', 'I', 0,
  /* 2338 */ 'Z', '8', '_', 'H', 'I', 0,
  /* 2344 */ 'Z', '1', '9', '_', 'H', 'I', 0,
  /* 2351 */ 'Z', '2', '9', '_', 'H', 'I', 0,
  /* 2358 */ 'Z', '9', '_', 'H', 'I', 0,
  /* 2364 */ 'X', '2', '8', '_', 'F', 'P', 0,
  /* 2371 */ 'W', 'S', 'P', 0,
  /* 2375 */ 'F', 'F', 'R', 0,
  /* 2379 */ 'L', 'R', 0,
  /* 2382 */ 'W', '3', '0', '_', 'W', 'Z', 'R', 0,
  /* 2390 */ 'L', 'R', '_', 'X', 'Z', 'R', 0,
  /* 2397 */ 'N', 'Z', 'C', 'V', 0,
};

extern const MCRegisterDesc AArch64RegDesc[] = { // Descriptors
  { 3, 0, 0, 0, 0, 0 },
  { 2375, 8, 8, 4, 22849, 0 },
  { 2368, 929, 1088, 5, 22849, 27 },
  { 2379, 929, 1086, 5, 22849, 27 },
  { 2397, 8, 8, 4, 22849, 0 },
  { 2372, 7, 8, 5, 6608, 27 },
  { 2371, 8, 1428, 4, 6608, 0 },
  { 2386, 8, 417, 4, 6640, 0 },
  { 2393, 1428, 1084, 5, 6640, 27 },
  { 195, 8, 226, 4, 22817, 0 },
  { 468, 8, 308, 4, 22817, 0 },
  { 671, 8, 499, 4, 22817, 0 },
  { 884, 8, 162, 4, 22817, 0 },
  { 1084, 8, 162, 4, 22817, 0 },
  { 1294, 8, 162, 4, 22817, 0 },
  { 1490, 8, 162, 4, 22817, 0 },
  { 1696, 8, 162, 4, 22817, 0 },
  { 1892, 8, 162, 4, 22817, 0 },
  { 2090, 8, 162, 4, 22817, 0 },
  { 0, 8, 162, 4, 22817, 0 },
  { 258, 8, 162, 4, 22817, 0 },
  { 534, 8, 162, 4, 22817, 0 },
  { 728, 8, 162, 4, 22817, 0 },
  { 944, 8, 162, 4, 22817, 0 },
  { 1138, 8, 162, 4, 22817, 0 },
  { 1354, 8, 162, 4, 22817, 0 },
  { 1544, 8, 162, 4, 22817, 0 },
  { 1756, 8, 162, 4, 22817, 0 },
  { 1946, 8, 162, 4, 22817, 0 },
  { 63, 8, 162, 4, 22817, 0 },
  { 332, 8, 162, 4, 22817, 0 },
  { 603, 8, 162, 4, 22817, 0 },
  { 808, 8, 162, 4, 22817, 0 },
  { 1016, 8, 162, 4, 22817, 0 },
  { 1218, 8, 162, 4, 22817, 0 },
  { 1422, 8, 162, 4, 22817, 0 },
  { 1620, 8, 162, 4, 22817, 0 },
  { 1824, 8, 162, 4, 22817, 0 },
  { 2022, 8, 162, 4, 22817, 0 },
  { 131, 8, 162, 4, 22817, 0 },
  { 408, 8, 162, 4, 22817, 0 },
  { 210, 1229, 229, 1, 22545, 3 },
  { 482, 1229, 311, 1, 22545, 3 },
  { 684, 1229, 502, 1, 22545, 3 },
  { 896, 1229, 165, 1, 22545, 3 },
  { 1096, 1229, 165, 1, 22545, 3 },
  { 1306, 1229, 165, 1, 22545, 3 },
  { 1502, 1229, 165, 1, 22545, 3 },
  { 1708, 1229, 165, 1, 22545, 3 },
  { 1904, 1229, 165, 1, 22545, 3 },
  { 2102, 1229, 165, 1, 22545, 3 },
  { 13, 1229, 165, 1, 22545, 3 },
  { 272, 1229, 165, 1, 22545, 3 },
  { 549, 1229, 165, 1, 22545, 3 },
  { 744, 1229, 165, 1, 22545, 3 },
  { 960, 1229, 165, 1, 22545, 3 },
  { 1154, 1229, 165, 1, 22545, 3 },
  { 1370, 1229, 165, 1, 22545, 3 },
  { 1560, 1229, 165, 1, 22545, 3 },
  { 1772, 1229, 165, 1, 22545, 3 },
  { 1962, 1229, 165, 1, 22545, 3 },
  { 79, 1229, 165, 1, 22545, 3 },
  { 348, 1229, 165, 1, 22545, 3 },
  { 619, 1229, 165, 1, 22545, 3 },
  { 824, 1229, 165, 1, 22545, 3 },
  { 1032, 1229, 165, 1, 22545, 3 },
  { 1234, 1229, 165, 1, 22545, 3 },
  { 1438, 1229, 165, 1, 22545, 3 },
  { 1636, 1229, 165, 1, 22545, 3 },
  { 1840, 1229, 165, 1, 22545, 3 },
  { 2038, 1229, 165, 1, 22545, 3 },
  { 147, 1229, 165, 1, 22545, 3 },
  { 424, 1229, 165, 1, 22545, 3 },
  { 213, 1231, 227, 3, 19553, 3 },
  { 485, 1231, 309, 3, 19553, 3 },
  { 687, 1231, 500, 3, 19553, 3 },
  { 899, 1231, 163, 3, 19553, 3 },
  { 1099, 1231, 163, 3, 19553, 3 },
  { 1309, 1231, 163, 3, 19553, 3 },
  { 1505, 1231, 163, 3, 19553, 3 },
  { 1711, 1231, 163, 3, 19553, 3 },
  { 1907, 1231, 163, 3, 19553, 3 },
  { 2105, 1231, 163, 3, 19553, 3 },
  { 17, 1231, 163, 3, 19553, 3 },
  { 276, 1231, 163, 3, 19553, 3 },
  { 553, 1231, 163, 3, 19553, 3 },
  { 748, 1231, 163, 3, 19553, 3 },
  { 964, 1231, 163, 3, 19553, 3 },
  { 1158, 1231, 163, 3, 19553, 3 },
  { 1374, 1231, 163, 3, 19553, 3 },
  { 1564, 1231, 163, 3, 19553, 3 },
  { 1776, 1231, 163, 3, 19553, 3 },
  { 1966, 1231, 163, 3, 19553, 3 },
  { 83, 1231, 163, 3, 19553, 3 },
  { 352, 1231, 163, 3, 19553, 3 },
  { 623, 1231, 163, 3, 19553, 3 },
  { 828, 1231, 163, 3, 19553, 3 },
  { 1036, 1231, 163, 3, 19553, 3 },
  { 1238, 1231, 163, 3, 19553, 3 },
  { 1442, 1231, 163, 3, 19553, 3 },
  { 1640, 1231, 163, 3, 19553, 3 },
  { 1844, 1231, 163, 3, 19553, 3 },
  { 2042, 1231, 163, 3, 19553, 3 },
  { 151, 1231, 163, 3, 19553, 3 },
  { 428, 1231, 163, 3, 19553, 3 },
  { 216, 8, 8, 4, 19553, 0 },
  { 488, 8, 8, 4, 19553, 0 },
  { 690, 8, 8, 4, 19553, 0 },
  { 902, 8, 8, 4, 19553, 0 },
  { 1102, 8, 8, 4, 19553, 0 },
  { 1312, 8, 8, 4, 19553, 0 },
  { 1508, 8, 8, 4, 19553, 0 },
  { 1714, 8, 8, 4, 19553, 0 },
  { 1910, 8, 8, 4, 19553, 0 },
  { 2108, 8, 8, 4, 19553, 0 },
  { 21, 8, 8, 4, 19553, 0 },
  { 280, 8, 8, 4, 19553, 0 },
  { 557, 8, 8, 4, 19553, 0 },
  { 752, 8, 8, 4, 19553, 0 },
  { 968, 8, 8, 4, 19553, 0 },
  { 1162, 8, 8, 4, 19553, 0 },
  { 231, 1242, 259, 0, 17649, 3 },
  { 502, 1242, 341, 0, 17649, 3 },
  { 703, 1242, 532, 0, 17649, 3 },
  { 914, 1242, 195, 0, 17649, 3 },
  { 1114, 1242, 195, 0, 17649, 3 },
  { 1324, 1242, 195, 0, 17649, 3 },
  { 1520, 1242, 195, 0, 17649, 3 },
  { 1726, 1242, 195, 0, 17649, 3 },
  { 1922, 1242, 195, 0, 17649, 3 },
  { 2120, 1242, 195, 0, 17649, 3 },
  { 34, 1242, 195, 0, 17649, 3 },
  { 294, 1242, 195, 0, 17649, 3 },
  { 572, 1242, 195, 0, 17649, 3 },
  { 768, 1242, 195, 0, 17649, 3 },
  { 984, 1242, 195, 0, 17649, 3 },
  { 1178, 1242, 195, 0, 17649, 3 },
  { 1390, 1242, 195, 0, 17649, 3 },
  { 1580, 1242, 195, 0, 17649, 3 },
  { 1792, 1242, 195, 0, 17649, 3 },
  { 1982, 1242, 195, 0, 17649, 3 },
  { 99, 1242, 195, 0, 17649, 3 },
  { 368, 1242, 195, 0, 17649, 3 },
  { 639, 1242, 195, 0, 17649, 3 },
  { 844, 1242, 195, 0, 17649, 3 },
  { 1052, 1242, 195, 0, 17649, 3 },
  { 1254, 1242, 195, 0, 17649, 3 },
  { 1458, 1242, 195, 0, 17649, 3 },
  { 1656, 1242, 195, 0, 17649, 3 },
  { 1860, 1242, 195, 0, 17649, 3 },
  { 2058, 1242, 195, 0, 17649, 3 },
  { 167, 1242, 195, 0, 17649, 3 },
  { 444, 1242, 195, 0, 17649, 3 },
  { 234, 1230, 228, 2, 17585, 3 },
  { 505, 1230, 310, 2, 17585, 3 },
  { 706, 1230, 501, 2, 17585, 3 },
  { 917, 1230, 164, 2, 17585, 3 },
  { 1117, 1230, 164, 2, 17585, 3 },
  { 1327, 1230, 164, 2, 17585, 3 },
  { 1523, 1230, 164, 2, 17585, 3 },
  { 1729, 1230, 164, 2, 17585, 3 },
  { 1925, 1230, 164, 2, 17585, 3 },
  { 2123, 1230, 164, 2, 17585, 3 },
  { 38, 1230, 164, 2, 17585, 3 },
  { 298, 1230, 164, 2, 17585, 3 },
  { 576, 1230, 164, 2, 17585, 3 },
  { 772, 1230, 164, 2, 17585, 3 },
  { 988, 1230, 164, 2, 17585, 3 },
  { 1182, 1230, 164, 2, 17585, 3 },
  { 1394, 1230, 164, 2, 17585, 3 },
  { 1584, 1230, 164, 2, 17585, 3 },
  { 1796, 1230, 164, 2, 17585, 3 },
  { 1986, 1230, 164, 2, 17585, 3 },
  { 103, 1230, 164, 2, 17585, 3 },
  { 372, 1230, 164, 2, 17585, 3 },
  { 643, 1230, 164, 2, 17585, 3 },
  { 848, 1230, 164, 2, 17585, 3 },
  { 1056, 1230, 164, 2, 17585, 3 },
  { 1258, 1230, 164, 2, 17585, 3 },
  { 1462, 1230, 164, 2, 17585, 3 },
  { 1660, 1230, 164, 2, 17585, 3 },
  { 1864, 1230, 164, 2, 17585, 3 },
  { 2062, 1230, 164, 2, 17585, 3 },
  { 171, 1230, 164, 2, 17585, 3 },
  { 448, 1230, 164, 2, 17585, 3 },
  { 237, 8, 481, 4, 17617, 0 },
  { 511, 8, 477, 4, 17617, 0 },
  { 709, 8, 477, 4, 17617, 0 },
  { 923, 8, 473, 4, 17617, 0 },
  { 1120, 8, 473, 4, 17617, 0 },
  { 1333, 8, 469, 4, 17617, 0 },
  { 1526, 8, 469, 4, 17617, 0 },
  { 1735, 8, 465, 4, 17617, 0 },
  { 1928, 8, 465, 4, 17617, 0 },
  { 2129, 8, 461, 4, 17617, 0 },
  { 42, 8, 461, 4, 17617, 0 },
  { 306, 8, 457, 4, 17617, 0 },
  { 580, 8, 457, 4, 17617, 0 },
  { 780, 8, 453, 4, 17617, 0 },
  { 992, 8, 453, 4, 17617, 0 },
  { 1190, 8, 449, 4, 17617, 0 },
  { 1398, 8, 449, 4, 17617, 0 },
  { 1592, 8, 445, 4, 17617, 0 },
  { 1800, 8, 445, 4, 17617, 0 },
  { 1994, 8, 441, 4, 17617, 0 },
  { 107, 8, 441, 4, 17617, 0 },
  { 380, 8, 437, 4, 17617, 0 },
  { 647, 8, 437, 4, 17617, 0 },
  { 856, 8, 433, 4, 17617, 0 },
  { 1060, 8, 433, 4, 17617, 0 },
  { 1266, 8, 429, 4, 17617, 0 },
  { 1466, 8, 429, 4, 17617, 0 },
  { 1668, 8, 425, 4, 17617, 0 },
  { 1868, 8, 399, 4, 17617, 0 },
  { 2070, 8, 403, 4, 17441, 0 },
  { 175, 8, 421, 4, 17441, 0 },
  { 240, 1424, 986, 5, 17553, 27 },
  { 517, 1424, 980, 5, 17553, 27 },
  { 712, 1424, 980, 5, 17553, 27 },
  { 929, 1424, 974, 5, 17553, 27 },
  { 1123, 1424, 974, 5, 17553, 27 },
  { 1339, 1424, 972, 5, 17553, 27 },
  { 1529, 1424, 972, 5, 17553, 27 },
  { 1741, 1424, 970, 5, 17553, 27 },
  { 1931, 1424, 970, 5, 17553, 27 },
  { 2135, 1424, 958, 5, 17553, 27 },
  { 46, 1424, 958, 5, 17553, 27 },
  { 314, 1424, 956, 5, 17553, 27 },
  { 584, 1424, 956, 5, 17553, 27 },
  { 788, 1424, 954, 5, 17553, 27 },
  { 996, 1424, 954, 5, 17553, 27 },
  { 1198, 1424, 952, 5, 17553, 27 },
  { 1402, 1424, 952, 5, 17553, 27 },
  { 1600, 1424, 950, 5, 17553, 27 },
  { 1804, 1424, 950, 5, 17553, 27 },
  { 2002, 1424, 948, 5, 17553, 27 },
  { 111, 1424, 948, 5, 17553, 27 },
  { 388, 1424, 946, 5, 17553, 27 },
  { 651, 1424, 946, 5, 17553, 27 },
  { 864, 1424, 944, 5, 17553, 27 },
  { 1064, 1424, 944, 5, 17553, 27 },
  { 1274, 1424, 942, 5, 17553, 27 },
  { 1470, 1424, 942, 5, 17553, 27 },
  { 1676, 1424, 940, 5, 17553, 27 },
  { 1872, 1424, 938, 5, 17553, 27 },
  { 255, 931, 280, 10, 9745, 35 },
  { 531, 931, 362, 10, 9745, 35 },
  { 725, 931, 553, 10, 9745, 35 },
  { 941, 931, 216, 10, 9745, 35 },
  { 1135, 931, 216, 10, 9745, 35 },
  { 1351, 931, 216, 10, 9745, 35 },
  { 1541, 931, 216, 10, 9745, 35 },
  { 1753, 931, 216, 10, 9745, 35 },
  { 1943, 931, 216, 10, 9745, 35 },
  { 2147, 931, 216, 10, 9745, 35 },
  { 59, 931, 216, 10, 9745, 35 },
  { 328, 931, 216, 10, 9745, 35 },
  { 599, 931, 216, 10, 9745, 35 },
  { 804, 931, 216, 10, 9745, 35 },
  { 1012, 931, 216, 10, 9745, 35 },
  { 1214, 931, 216, 10, 9745, 35 },
  { 1418, 931, 216, 10, 9745, 35 },
  { 1616, 931, 216, 10, 9745, 35 },
  { 1820, 931, 216, 10, 9745, 35 },
  { 2018, 931, 216, 10, 9745, 35 },
  { 127, 931, 216, 10, 9745, 35 },
  { 404, 931, 216, 10, 9745, 35 },
  { 667, 931, 216, 10, 9745, 35 },
  { 880, 931, 216, 10, 9745, 35 },
  { 1080, 931, 216, 10, 9745, 35 },
  { 1290, 931, 216, 10, 9745, 35 },
  { 1486, 931, 216, 10, 9745, 35 },
  { 1692, 931, 216, 10, 9745, 35 },
  { 1888, 931, 216, 10, 9745, 35 },
  { 2086, 931, 216, 10, 9745, 35 },
  { 191, 931, 216, 10, 9745, 35 },
  { 464, 931, 216, 10, 9745, 35 },
  { 2171, 8, 279, 4, 17521, 0 },
  { 2198, 8, 361, 4, 17521, 0 },
  { 2218, 8, 552, 4, 17521, 0 },
  { 2238, 8, 215, 4, 17521, 0 },
  { 2258, 8, 215, 4, 17521, 0 },
  { 2278, 8, 215, 4, 17521, 0 },
  { 2298, 8, 215, 4, 17521, 0 },
  { 2318, 8, 215, 4, 17521, 0 },
  { 2338, 8, 215, 4, 17521, 0 },
  { 2358, 8, 215, 4, 17521, 0 },
  { 2150, 8, 215, 4, 17521, 0 },
  { 2177, 8, 215, 4, 17521, 0 },
  { 2204, 8, 215, 4, 17521, 0 },
  { 2224, 8, 215, 4, 17521, 0 },
  { 2244, 8, 215, 4, 17521, 0 },
  { 2264, 8, 215, 4, 17521, 0 },
  { 2284, 8, 215, 4, 17521, 0 },
  { 2304, 8, 215, 4, 17521, 0 },
  { 2324, 8, 215, 4, 17521, 0 },
  { 2344, 8, 215, 4, 17521, 0 },
  { 2157, 8, 215, 4, 17521, 0 },
  { 2184, 8, 215, 4, 17521, 0 },
  { 2211, 8, 215, 4, 17521, 0 },
  { 2231, 8, 215, 4, 17521, 0 },
  { 2251, 8, 215, 4, 17521, 0 },
  { 2271, 8, 215, 4, 17521, 0 },
  { 2291, 8, 215, 4, 17521, 0 },
  { 2311, 8, 215, 4, 17521, 0 },
  { 2331, 8, 215, 4, 17521, 0 },
  { 2351, 8, 215, 4, 17521, 0 },
  { 2164, 8, 215, 4, 17521, 0 },
  { 2191, 8, 215, 4, 17521, 0 },
  { 479, 1233, 372, 17, 2545, 61 },
  { 681, 1233, 574, 17, 2545, 61 },
  { 893, 1233, 290, 17, 2545, 61 },
  { 1093, 1233, 290, 17, 2545, 61 },
  { 1303, 1233, 290, 17, 2545, 61 },
  { 1499, 1233, 290, 17, 2545, 61 },
  { 1705, 1233, 290, 17, 2545, 61 },
  { 1901, 1233, 290, 17, 2545, 61 },
  { 2099, 1233, 290, 17, 2545, 61 },
  { 10, 1233, 290, 17, 2545, 61 },
  { 268, 1233, 290, 17, 2545, 61 },
  { 545, 1233, 290, 17, 2545, 61 },
  { 740, 1233, 290, 17, 2545, 61 },
  { 956, 1233, 290, 17, 2545, 61 },
  { 1150, 1233, 290, 17, 2545, 61 },
  { 1366, 1233, 290, 17, 2545, 61 },
  { 1556, 1233, 290, 17, 2545, 61 },
  { 1768, 1233, 290, 17, 2545, 61 },
  { 1958, 1233, 290, 17, 2545, 61 },
  { 75, 1233, 290, 17, 2545, 61 },
  { 344, 1233, 290, 17, 2545, 61 },
  { 615, 1233, 290, 17, 2545, 61 },
  { 820, 1233, 290, 17, 2545, 61 },
  { 1028, 1233, 290, 17, 2545, 61 },
  { 1230, 1233, 290, 17, 2545, 61 },
  { 1434, 1233, 290, 17, 2545, 61 },
  { 1632, 1233, 290, 17, 2545, 61 },
  { 1836, 1233, 290, 17, 2545, 61 },
  { 2034, 1233, 290, 17, 2545, 61 },
  { 143, 1233, 290, 17, 2545, 61 },
  { 420, 1233, 290, 17, 2545, 61 },
  { 206, 1224, 290, 17, 9472, 2 },
  { 887, 1365, 926, 41, 225, 68 },
  { 1087, 1365, 926, 41, 225, 68 },
  { 1297, 1365, 926, 41, 225, 68 },
  { 1493, 1365, 926, 41, 225, 68 },
  { 1699, 1365, 926, 41, 225, 68 },
  { 1895, 1365, 926, 41, 225, 68 },
  { 2093, 1365, 926, 41, 225, 68 },
  { 4, 1365, 926, 41, 225, 68 },
  { 262, 1365, 926, 41, 225, 68 },
  { 538, 1365, 926, 41, 225, 68 },
  { 732, 1365, 926, 41, 225, 68 },
  { 948, 1365, 926, 41, 225, 68 },
  { 1142, 1365, 926, 41, 225, 68 },
  { 1358, 1365, 926, 41, 225, 68 },
  { 1548, 1365, 926, 41, 225, 68 },
  { 1760, 1365, 926, 41, 225, 68 },
  { 1950, 1365, 926, 41, 225, 68 },
  { 67, 1365, 926, 41, 225, 68 },
  { 336, 1365, 926, 41, 225, 68 },
  { 607, 1365, 926, 41, 225, 68 },
  { 812, 1365, 926, 41, 225, 68 },
  { 1020, 1365, 926, 41, 225, 68 },
  { 1222, 1365, 926, 41, 225, 68 },
  { 1426, 1365, 926, 41, 225, 68 },
  { 1624, 1365, 926, 41, 225, 68 },
  { 1828, 1365, 926, 41, 225, 68 },
  { 2026, 1365, 926, 41, 225, 68 },
  { 135, 1365, 926, 41, 225, 68 },
  { 412, 1365, 926, 41, 225, 68 },
  { 198, 1387, 926, 41, 304, 73 },
  { 471, 1200, 926, 41, 768, 59 },
  { 674, 1343, 926, 41, 7760, 5 },
  { 678, 78, 600, 26, 705, 74 },
  { 890, 78, 390, 26, 705, 74 },
  { 1090, 78, 390, 26, 705, 74 },
  { 1300, 78, 390, 26, 705, 74 },
  { 1496, 78, 390, 26, 705, 74 },
  { 1702, 78, 390, 26, 705, 74 },
  { 1898, 78, 390, 26, 705, 74 },
  { 2096, 78, 390, 26, 705, 74 },
  { 7, 78, 390, 26, 705, 74 },
  { 265, 78, 390, 26, 705, 74 },
  { 541, 78, 390, 26, 705, 74 },
  { 736, 78, 390, 26, 705, 74 },
  { 952, 78, 390, 26, 705, 74 },
  { 1146, 78, 390, 26, 705, 74 },
  { 1362, 78, 390, 26, 705, 74 },
  { 1552, 78, 390, 26, 705, 74 },
  { 1764, 78, 390, 26, 705, 74 },
  { 1954, 78, 390, 26, 705, 74 },
  { 71, 78, 390, 26, 705, 74 },
  { 340, 78, 390, 26, 705, 74 },
  { 611, 78, 390, 26, 705, 74 },
  { 816, 78, 390, 26, 705, 74 },
  { 1024, 78, 390, 26, 705, 74 },
  { 1226, 78, 390, 26, 705, 74 },
  { 1430, 78, 390, 26, 705, 74 },
  { 1628, 78, 390, 26, 705, 74 },
  { 1832, 78, 390, 26, 705, 74 },
  { 2030, 78, 390, 26, 705, 74 },
  { 139, 78, 390, 26, 705, 74 },
  { 416, 78, 390, 26, 705, 74 },
  { 202, 93, 390, 26, 992, 64 },
  { 475, 1411, 390, 26, 9008, 10 },
  { 499, 960, 378, 63, 1825, 80 },
  { 700, 960, 580, 63, 1825, 80 },
  { 911, 960, 296, 63, 1825, 80 },
  { 1111, 960, 296, 63, 1825, 80 },
  { 1321, 960, 296, 63, 1825, 80 },
  { 1517, 960, 296, 63, 1825, 80 },
  { 1723, 960, 296, 63, 1825, 80 },
  { 1919, 960, 296, 63, 1825, 80 },
  { 2117, 960, 296, 63, 1825, 80 },
  { 31, 960, 296, 63, 1825, 80 },
  { 290, 960, 296, 63, 1825, 80 },
  { 568, 960, 296, 63, 1825, 80 },
  { 764, 960, 296, 63, 1825, 80 },
  { 980, 960, 296, 63, 1825, 80 },
  { 1174, 960, 296, 63, 1825, 80 },
  { 1386, 960, 296, 63, 1825, 80 },
  { 1576, 960, 296, 63, 1825, 80 },
  { 1788, 960, 296, 63, 1825, 80 },
  { 1978, 960, 296, 63, 1825, 80 },
  { 95, 960, 296, 63, 1825, 80 },
  { 364, 960, 296, 63, 1825, 80 },
  { 635, 960, 296, 63, 1825, 80 },
  { 840, 960, 296, 63, 1825, 80 },
  { 1048, 960, 296, 63, 1825, 80 },
  { 1250, 960, 296, 63, 1825, 80 },
  { 1454, 960, 296, 63, 1825, 80 },
  { 1652, 960, 296, 63, 1825, 80 },
  { 1856, 960, 296, 63, 1825, 80 },
  { 2054, 960, 296, 63, 1825, 80 },
  { 163, 960, 296, 63, 1825, 80 },
  { 440, 960, 296, 63, 1825, 80 },
  { 227, 1066, 296, 63, 9472, 14 },
  { 905, 1279, 927, 96, 145, 87 },
  { 1105, 1279, 927, 96, 145, 87 },
  { 1315, 1279, 927, 96, 145, 87 },
  { 1511, 1279, 927, 96, 145, 87 },
  { 1717, 1279, 927, 96, 145, 87 },
  { 1913, 1279, 927, 96, 145, 87 },
  { 2111, 1279, 927, 96, 145, 87 },
  { 25, 1279, 927, 96, 145, 87 },
  { 284, 1279, 927, 96, 145, 87 },
  { 561, 1279, 927, 96, 145, 87 },
  { 756, 1279, 927, 96, 145, 87 },
  { 972, 1279, 927, 96, 145, 87 },
  { 1166, 1279, 927, 96, 145, 87 },
  { 1378, 1279, 927, 96, 145, 87 },
  { 1568, 1279, 927, 96, 145, 87 },
  { 1780, 1279, 927, 96, 145, 87 },
  { 1970, 1279, 927, 96, 145, 87 },
  { 87, 1279, 927, 96, 145, 87 },
  { 356, 1279, 927, 96, 145, 87 },
  { 627, 1279, 927, 96, 145, 87 },
  { 832, 1279, 927, 96, 145, 87 },
  { 1040, 1279, 927, 96, 145, 87 },
  { 1242, 1279, 927, 96, 145, 87 },
  { 1446, 1279, 927, 96, 145, 87 },
  { 1644, 1279, 927, 96, 145, 87 },
  { 1848, 1279, 927, 96, 145, 87 },
  { 2046, 1279, 927, 96, 145, 87 },
  { 155, 1279, 927, 96, 145, 87 },
  { 432, 1279, 927, 96, 145, 87 },
  { 219, 1311, 927, 96, 304, 92 },
  { 491, 1168, 927, 96, 768, 78 },
  { 693, 1247, 927, 96, 7760, 17 },
  { 697, 1105, 603, 75, 641, 93 },
  { 908, 1105, 393, 75, 641, 93 },
  { 1108, 1105, 393, 75, 641, 93 },
  { 1318, 1105, 393, 75, 641, 93 },
  { 1514, 1105, 393, 75, 641, 93 },
  { 1720, 1105, 393, 75, 641, 93 },
  { 1916, 1105, 393, 75, 641, 93 },
  { 2114, 1105, 393, 75, 641, 93 },
  { 28, 1105, 393, 75, 641, 93 },
  { 287, 1105, 393, 75, 641, 93 },
  { 564, 1105, 393, 75, 641, 93 },
  { 760, 1105, 393, 75, 641, 93 },
  { 976, 1105, 393, 75, 641, 93 },
  { 1170, 1105, 393, 75, 641, 93 },
  { 1382, 1105, 393, 75, 641, 93 },
  { 1572, 1105, 393, 75, 641, 93 },
  { 1784, 1105, 393, 75, 641, 93 },
  { 1974, 1105, 393, 75, 641, 93 },
  { 91, 1105, 393, 75, 641, 93 },
  { 360, 1105, 393, 75, 641, 93 },
  { 631, 1105, 393, 75, 641, 93 },
  { 836, 1105, 393, 75, 641, 93 },
  { 1044, 1105, 393, 75, 641, 93 },
  { 1246, 1105, 393, 75, 641, 93 },
  { 1450, 1105, 393, 75, 641, 93 },
  { 1648, 1105, 393, 75, 641, 93 },
  { 1852, 1105, 393, 75, 641, 93 },
  { 2050, 1105, 393, 75, 641, 93 },
  { 159, 1105, 393, 75, 641, 93 },
  { 436, 1105, 393, 75, 641, 93 },
  { 223, 1126, 393, 75, 992, 83 },
  { 495, 1147, 393, 75, 9008, 22 },
  { 2382, 1092, 419, 7, 6512, 32 },
  { 508, 117, 427, 7, 1778, 32 },
  { 920, 120, 427, 7, 1778, 32 },
  { 1330, 123, 427, 7, 1778, 32 },
  { 1732, 126, 427, 7, 1778, 32 },
  { 2126, 129, 427, 7, 1778, 32 },
  { 302, 132, 427, 7, 1778, 32 },
  { 776, 135, 427, 7, 1778, 32 },
  { 1186, 138, 427, 7, 1778, 32 },
  { 1588, 141, 427, 7, 1778, 32 },
  { 1990, 144, 427, 7, 1778, 32 },
  { 376, 147, 427, 7, 1778, 32 },
  { 852, 150, 427, 7, 1778, 32 },
  { 1262, 153, 427, 7, 1778, 32 },
  { 1664, 156, 427, 7, 1778, 32 },
  { 2066, 159, 401, 7, 9841, 29 },
  { 2390, 1078, 8, 128, 6561, 97 },
  { 2364, 976, 8, 128, 9792, 26 },
  { 514, 1060, 8, 128, 1730, 97 },
  { 926, 1054, 8, 128, 1730, 97 },
  { 1336, 1048, 8, 128, 1730, 97 },
  { 1738, 1042, 8, 128, 1730, 97 },
  { 2132, 1036, 8, 128, 1730, 97 },
  { 310, 1030, 8, 128, 1730, 97 },
  { 784, 1024, 8, 128, 1730, 97 },
  { 1194, 1018, 8, 128, 1730, 97 },
  { 1596, 1012, 8, 128, 1730, 97 },
  { 1998, 1006, 8, 128, 1730, 97 },
  { 384, 1000, 8, 128, 1730, 97 },
  { 860, 994, 8, 128, 1730, 97 },
  { 1270, 988, 8, 128, 1730, 97 },
  { 1672, 982, 8, 128, 1730, 97 },
  { 528, 618, 384, 134, 1169, 100 },
  { 722, 618, 586, 134, 1169, 100 },
  { 938, 618, 302, 134, 1169, 100 },
  { 1132, 618, 302, 134, 1169, 100 },
  { 1348, 618, 302, 134, 1169, 100 },
  { 1538, 618, 302, 134, 1169, 100 },
  { 1750, 618, 302, 134, 1169, 100 },
  { 1940, 618, 302, 134, 1169, 100 },
  { 2144, 618, 302, 134, 1169, 100 },
  { 56, 618, 302, 134, 1169, 100 },
  { 324, 618, 302, 134, 1169, 100 },
  { 595, 618, 302, 134, 1169, 100 },
  { 800, 618, 302, 134, 1169, 100 },
  { 1008, 618, 302, 134, 1169, 100 },
  { 1210, 618, 302, 134, 1169, 100 },
  { 1414, 618, 302, 134, 1169, 100 },
  { 1612, 618, 302, 134, 1169, 100 },
  { 1816, 618, 302, 134, 1169, 100 },
  { 2014, 618, 302, 134, 1169, 100 },
  { 123, 618, 302, 134, 1169, 100 },
  { 400, 618, 302, 134, 1169, 100 },
  { 663, 618, 302, 134, 1169, 100 },
  { 876, 618, 302, 134, 1169, 100 },
  { 1076, 618, 302, 134, 1169, 100 },
  { 1286, 618, 302, 134, 1169, 100 },
  { 1482, 618, 302, 134, 1169, 100 },
  { 1688, 618, 302, 134, 1169, 100 },
  { 1884, 618, 302, 134, 1169, 100 },
  { 2082, 618, 302, 134, 1169, 100 },
  { 187, 618, 302, 134, 1169, 100 },
  { 460, 618, 302, 134, 1169, 100 },
  { 251, 635, 302, 134, 9520, 38 },
  { 932, 834, 8, 181, 1, 121 },
  { 1126, 834, 8, 181, 1, 121 },
  { 1342, 834, 8, 181, 1, 121 },
  { 1532, 834, 8, 181, 1, 121 },
  { 1744, 834, 8, 181, 1, 121 },
  { 1934, 834, 8, 181, 1, 121 },
  { 2138, 834, 8, 181, 1, 121 },
  { 50, 834, 8, 181, 1, 121 },
  { 318, 834, 8, 181, 1, 121 },
  { 588, 834, 8, 181, 1, 121 },
  { 792, 834, 8, 181, 1, 121 },
  { 1000, 834, 8, 181, 1, 121 },
  { 1202, 834, 8, 181, 1, 121 },
  { 1406, 834, 8, 181, 1, 121 },
  { 1604, 834, 8, 181, 1, 121 },
  { 1808, 834, 8, 181, 1, 121 },
  { 2006, 834, 8, 181, 1, 121 },
  { 115, 834, 8, 181, 1, 121 },
  { 392, 834, 8, 181, 1, 121 },
  { 655, 834, 8, 181, 1, 121 },
  { 868, 834, 8, 181, 1, 121 },
  { 1068, 834, 8, 181, 1, 121 },
  { 1278, 834, 8, 181, 1, 121 },
  { 1474, 834, 8, 181, 1, 121 },
  { 1680, 834, 8, 181, 1, 121 },
  { 1876, 834, 8, 181, 1, 121 },
  { 2074, 834, 8, 181, 1, 121 },
  { 179, 834, 8, 181, 1, 121 },
  { 452, 834, 8, 181, 1, 121 },
  { 243, 880, 8, 181, 384, 130 },
  { 520, 742, 8, 181, 848, 105 },
  { 715, 788, 8, 181, 7840, 43 },
  { 719, 652, 606, 151, 529, 139 },
  { 935, 652, 192, 151, 529, 139 },
  { 1129, 652, 192, 151, 529, 139 },
  { 1345, 652, 192, 151, 529, 139 },
  { 1535, 652, 192, 151, 529, 139 },
  { 1747, 652, 192, 151, 529, 139 },
  { 1937, 652, 192, 151, 529, 139 },
  { 2141, 652, 192, 151, 529, 139 },
  { 53, 652, 192, 151, 529, 139 },
  { 321, 652, 192, 151, 529, 139 },
  { 591, 652, 192, 151, 529, 139 },
  { 796, 652, 192, 151, 529, 139 },
  { 1004, 652, 192, 151, 529, 139 },
  { 1206, 652, 192, 151, 529, 139 },
  { 1410, 652, 192, 151, 529, 139 },
  { 1608, 652, 192, 151, 529, 139 },
  { 1812, 652, 192, 151, 529, 139 },
  { 2010, 652, 192, 151, 529, 139 },
  { 119, 652, 192, 151, 529, 139 },
  { 396, 652, 192, 151, 529, 139 },
  { 659, 652, 192, 151, 529, 139 },
  { 872, 652, 192, 151, 529, 139 },
  { 1072, 652, 192, 151, 529, 139 },
  { 1282, 652, 192, 151, 529, 139 },
  { 1478, 652, 192, 151, 529, 139 },
  { 1684, 652, 192, 151, 529, 139 },
  { 1880, 652, 192, 151, 529, 139 },
  { 2078, 652, 192, 151, 529, 139 },
  { 183, 652, 192, 151, 529, 139 },
  { 456, 652, 192, 151, 529, 139 },
  { 247, 682, 192, 151, 1056, 114 },
  { 524, 712, 192, 151, 9072, 52 },
};

extern const MCPhysReg AArch64RegUnitRoots[][2] = {
  { AArch64::FFR },
  { AArch64::W29 },
  { AArch64::W30 },
  { AArch64::NZCV },
  { AArch64::WSP },
  { AArch64::WZR },
  { AArch64::B0 },
  { AArch64::B1 },
  { AArch64::B2 },
  { AArch64::B3 },
  { AArch64::B4 },
  { AArch64::B5 },
  { AArch64::B6 },
  { AArch64::B7 },
  { AArch64::B8 },
  { AArch64::B9 },
  { AArch64::B10 },
  { AArch64::B11 },
  { AArch64::B12 },
  { AArch64::B13 },
  { AArch64::B14 },
  { AArch64::B15 },
  { AArch64::B16 },
  { AArch64::B17 },
  { AArch64::B18 },
  { AArch64::B19 },
  { AArch64::B20 },
  { AArch64::B21 },
  { AArch64::B22 },
  { AArch64::B23 },
  { AArch64::B24 },
  { AArch64::B25 },
  { AArch64::B26 },
  { AArch64::B27 },
  { AArch64::B28 },
  { AArch64::B29 },
  { AArch64::B30 },
  { AArch64::B31 },
  { AArch64::P0 },
  { AArch64::P1 },
  { AArch64::P2 },
  { AArch64::P3 },
  { AArch64::P4 },
  { AArch64::P5 },
  { AArch64::P6 },
  { AArch64::P7 },
  { AArch64::P8 },
  { AArch64::P9 },
  { AArch64::P10 },
  { AArch64::P11 },
  { AArch64::P12 },
  { AArch64::P13 },
  { AArch64::P14 },
  { AArch64::P15 },
  { AArch64::W0 },
  { AArch64::W1 },
  { AArch64::W2 },
  { AArch64::W3 },
  { AArch64::W4 },
  { AArch64::W5 },
  { AArch64::W6 },
  { AArch64::W7 },
  { AArch64::W8 },
  { AArch64::W9 },
  { AArch64::W10 },
  { AArch64::W11 },
  { AArch64::W12 },
  { AArch64::W13 },
  { AArch64::W14 },
  { AArch64::W15 },
  { AArch64::W16 },
  { AArch64::W17 },
  { AArch64::W18 },
  { AArch64::W19 },
  { AArch64::W20 },
  { AArch64::W21 },
  { AArch64::W22 },
  { AArch64::W23 },
  { AArch64::W24 },
  { AArch64::W25 },
  { AArch64::W26 },
  { AArch64::W27 },
  { AArch64::W28 },
  { AArch64::Z0_HI },
  { AArch64::Z1_HI },
  { AArch64::Z2_HI },
  { AArch64::Z3_HI },
  { AArch64::Z4_HI },
  { AArch64::Z5_HI },
  { AArch64::Z6_HI },
  { AArch64::Z7_HI },
  { AArch64::Z8_HI },
  { AArch64::Z9_HI },
  { AArch64::Z10_HI },
  { AArch64::Z11_HI },
  { AArch64::Z12_HI },
  { AArch64::Z13_HI },
  { AArch64::Z14_HI },
  { AArch64::Z15_HI },
  { AArch64::Z16_HI },
  { AArch64::Z17_HI },
  { AArch64::Z18_HI },
  { AArch64::Z19_HI },
  { AArch64::Z20_HI },
  { AArch64::Z21_HI },
  { AArch64::Z22_HI },
  { AArch64::Z23_HI },
  { AArch64::Z24_HI },
  { AArch64::Z25_HI },
  { AArch64::Z26_HI },
  { AArch64::Z27_HI },
  { AArch64::Z28_HI },
  { AArch64::Z29_HI },
  { AArch64::Z30_HI },
  { AArch64::Z31_HI },
};

namespace {     // Register classes...
  // FPR8 Register Class...
  const MCPhysReg FPR8[] = {
    AArch64::B0, AArch64::B1, AArch64::B2, AArch64::B3, AArch64::B4, AArch64::B5, AArch64::B6, AArch64::B7, AArch64::B8, AArch64::B9, AArch64::B10, AArch64::B11, AArch64::B12, AArch64::B13, AArch64::B14, AArch64::B15, AArch64::B16, AArch64::B17, AArch64::B18, AArch64::B19, AArch64::B20, AArch64::B21, AArch64::B22, AArch64::B23, AArch64::B24, AArch64::B25, AArch64::B26, AArch64::B27, AArch64::B28, AArch64::B29, AArch64::B30, AArch64::B31, 
  };

  // FPR8 Bit set.
  const uint8_t FPR8Bits[] = {
    0x00, 0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // FPR16 Register Class...
  const MCPhysReg FPR16[] = {
    AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7, AArch64::H8, AArch64::H9, AArch64::H10, AArch64::H11, AArch64::H12, AArch64::H13, AArch64::H14, AArch64::H15, AArch64::H16, AArch64::H17, AArch64::H18, AArch64::H19, AArch64::H20, AArch64::H21, AArch64::H22, AArch64::H23, AArch64::H24, AArch64::H25, AArch64::H26, AArch64::H27, AArch64::H28, AArch64::H29, AArch64::H30, AArch64::H31, 
  };

  // FPR16 Bit set.
  const uint8_t FPR16Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // PPR Register Class...
  const MCPhysReg PPR[] = {
    AArch64::P0, AArch64::P1, AArch64::P2, AArch64::P3, AArch64::P4, AArch64::P5, AArch64::P6, AArch64::P7, AArch64::P8, AArch64::P9, AArch64::P10, AArch64::P11, AArch64::P12, AArch64::P13, AArch64::P14, AArch64::P15, 
  };

  // PPR Bit set.
  const uint8_t PPRBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, 
  };

  // PPR_3b Register Class...
  const MCPhysReg PPR_3b[] = {
    AArch64::P0, AArch64::P1, AArch64::P2, AArch64::P3, AArch64::P4, AArch64::P5, AArch64::P6, AArch64::P7, 
  };

  // PPR_3b Bit set.
  const uint8_t PPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 
  };

  // GPR32all Register Class...
  const MCPhysReg GPR32all[] = {
    AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WZR, AArch64::WSP, 
  };

  // GPR32all Bit set.
  const uint8_t GPR32allBits[] = {
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 
  };

  // FPR32 Register Class...
  const MCPhysReg FPR32[] = {
    AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7, AArch64::S8, AArch64::S9, AArch64::S10, AArch64::S11, AArch64::S12, AArch64::S13, AArch64::S14, AArch64::S15, AArch64::S16, AArch64::S17, AArch64::S18, AArch64::S19, AArch64::S20, AArch64::S21, AArch64::S22, AArch64::S23, AArch64::S24, AArch64::S25, AArch64::S26, AArch64::S27, AArch64::S28, AArch64::S29, AArch64::S30, AArch64::S31, 
  };

  // FPR32 Bit set.
  const uint8_t FPR32Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // GPR32 Register Class...
  const MCPhysReg GPR32[] = {
    AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WZR, 
  };

  // GPR32 Bit set.
  const uint8_t GPR32Bits[] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 
  };

  // GPR32sp Register Class...
  const MCPhysReg GPR32sp[] = {
    AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WSP, 
  };

  // GPR32sp Bit set.
  const uint8_t GPR32spBits[] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 
  };

  // GPR32common Register Class...
  const MCPhysReg GPR32common[] = {
    AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, 
  };

  // GPR32common Bit set.
  const uint8_t GPR32commonBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 
  };

  // GPR32arg Register Class...
  const MCPhysReg GPR32arg[] = {
    AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, 
  };

  // GPR32arg Bit set.
  const uint8_t GPR32argBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 
  };

  // CCR Register Class...
  const MCPhysReg CCR[] = {
    AArch64::NZCV, 
  };

  // CCR Bit set.
  const uint8_t CCRBits[] = {
    0x10, 
  };

  // GPR32sponly Register Class...
  const MCPhysReg GPR32sponly[] = {
    AArch64::WSP, 
  };

  // GPR32sponly Bit set.
  const uint8_t GPR32sponlyBits[] = {
    0x40, 
  };

  // WSeqPairsClass Register Class...
  const MCPhysReg WSeqPairsClass[] = {
    AArch64::W0_W1, AArch64::W2_W3, AArch64::W4_W5, AArch64::W6_W7, AArch64::W8_W9, AArch64::W10_W11, AArch64::W12_W13, AArch64::W14_W15, AArch64::W16_W17, AArch64::W18_W19, AArch64::W20_W21, AArch64::W22_W23, AArch64::W24_W25, AArch64::W26_W27, AArch64::W28_W29, AArch64::W30_WZR, 
  };

  // WSeqPairsClass Bit set.
  const uint8_t WSeqPairsClassBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // WSeqPairsClass_with_subo32_in_GPR32common Register Class...
  const MCPhysReg WSeqPairsClass_with_subo32_in_GPR32common[] = {
    AArch64::W0_W1, AArch64::W2_W3, AArch64::W4_W5, AArch64::W6_W7, AArch64::W8_W9, AArch64::W10_W11, AArch64::W12_W13, AArch64::W14_W15, AArch64::W16_W17, AArch64::W18_W19, AArch64::W20_W21, AArch64::W22_W23, AArch64::W24_W25, AArch64::W26_W27, AArch64::W28_W29, 
  };

  // WSeqPairsClass_with_subo32_in_GPR32common Bit set.
  const uint8_t WSeqPairsClass_with_subo32_in_GPR32commonBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0x1f, 
  };

  // WSeqPairsClass_with_sube32_in_GPR32arg Register Class...
  const MCPhysReg WSeqPairsClass_with_sube32_in_GPR32arg[] = {
    AArch64::W0_W1, AArch64::W2_W3, AArch64::W4_W5, AArch64::W6_W7, 
  };

  // WSeqPairsClass_with_sube32_in_GPR32arg Bit set.
  const uint8_t WSeqPairsClass_with_sube32_in_GPR32argBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 
  };

  // GPR64all Register Class...
  const MCPhysReg GPR64all[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::XZR, AArch64::SP, 
  };

  // GPR64all Bit set.
  const uint8_t GPR64allBits[] = {
    0x2c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 
  };

  // FPR64 Register Class...
  const MCPhysReg FPR64[] = {
    AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, 
  };

  // FPR64 Bit set.
  const uint8_t FPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // GPR64 Register Class...
  const MCPhysReg GPR64[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::XZR, 
  };

  // GPR64 Bit set.
  const uint8_t GPR64Bits[] = {
    0x0c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 
  };

  // GPR64sp Register Class...
  const MCPhysReg GPR64sp[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::SP, 
  };

  // GPR64sp Bit set.
  const uint8_t GPR64spBits[] = {
    0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 
  };

  // GPR64common Register Class...
  const MCPhysReg GPR64common[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, 
  };

  // GPR64common Bit set.
  const uint8_t GPR64commonBits[] = {
    0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x1f, 
  };

  // GPR64noip Register Class...
  const MCPhysReg GPR64noip[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::XZR, 
  };

  // GPR64noip Bit set.
  const uint8_t GPR64noipBits[] = {
    0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x1f, 
  };

  // GPR64common_and_GPR64noip Register Class...
  const MCPhysReg GPR64common_and_GPR64noip[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, 
  };

  // GPR64common_and_GPR64noip Bit set.
  const uint8_t GPR64common_and_GPR64noipBits[] = {
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x1f, 
  };

  // tcGPR64 Register Class...
  const MCPhysReg tcGPR64[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, 
  };

  // tcGPR64 Bit set.
  const uint8_t tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x07, 
  };

  // GPR64noip_and_tcGPR64 Register Class...
  const MCPhysReg GPR64noip_and_tcGPR64[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, 
  };

  // GPR64noip_and_tcGPR64 Bit set.
  const uint8_t GPR64noip_and_tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x04, 
  };

  // GPR64arg Register Class...
  const MCPhysReg GPR64arg[] = {
    AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, 
  };

  // GPR64arg Bit set.
  const uint8_t GPR64argBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
  };

  // rtcGPR64 Register Class...
  const MCPhysReg rtcGPR64[] = {
    AArch64::X16, AArch64::X17, 
  };

  // rtcGPR64 Bit set.
  const uint8_t rtcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
  };

  // GPR64sponly Register Class...
  const MCPhysReg GPR64sponly[] = {
    AArch64::SP, 
  };

  // GPR64sponly Bit set.
  const uint8_t GPR64sponlyBits[] = {
    0x20, 
  };

  // DD Register Class...
  const MCPhysReg DD[] = {
    AArch64::D0_D1, AArch64::D1_D2, AArch64::D2_D3, AArch64::D3_D4, AArch64::D4_D5, AArch64::D5_D6, AArch64::D6_D7, AArch64::D7_D8, AArch64::D8_D9, AArch64::D9_D10, AArch64::D10_D11, AArch64::D11_D12, AArch64::D12_D13, AArch64::D13_D14, AArch64::D14_D15, AArch64::D15_D16, AArch64::D16_D17, AArch64::D17_D18, AArch64::D18_D19, AArch64::D19_D20, AArch64::D20_D21, AArch64::D21_D22, AArch64::D22_D23, AArch64::D23_D24, AArch64::D24_D25, AArch64::D25_D26, AArch64::D26_D27, AArch64::D27_D28, AArch64::D28_D29, AArch64::D29_D30, AArch64::D30_D31, AArch64::D31_D0, 
  };

  // DD Bit set.
  const uint8_t DDBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // XSeqPairsClass Register Class...
  const MCPhysReg XSeqPairsClass[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X16_X17, AArch64::X18_X19, AArch64::X20_X21, AArch64::X22_X23, AArch64::X24_X25, AArch64::X26_X27, AArch64::X28_FP, AArch64::LR_XZR, 
  };

  // XSeqPairsClass Bit set.
  const uint8_t XSeqPairsClassBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64common Register Class...
  const MCPhysReg XSeqPairsClass_with_subo64_in_GPR64common[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X16_X17, AArch64::X18_X19, AArch64::X20_X21, AArch64::X22_X23, AArch64::X24_X25, AArch64::X26_X27, AArch64::X28_FP, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64common Bit set.
  const uint8_t XSeqPairsClass_with_subo64_in_GPR64commonBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0x1f, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64noip Register Class...
  const MCPhysReg XSeqPairsClass_with_subo64_in_GPR64noip[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X18_X19, AArch64::X20_X21, AArch64::X22_X23, AArch64::X24_X25, AArch64::X26_X27, AArch64::X28_FP, AArch64::LR_XZR, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64noip Bit set.
  const uint8_t XSeqPairsClass_with_subo64_in_GPR64noipBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x1f, 
  };

  // XSeqPairsClass_with_sube64_in_GPR64noip Register Class...
  const MCPhysReg XSeqPairsClass_with_sube64_in_GPR64noip[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X18_X19, AArch64::X20_X21, AArch64::X22_X23, AArch64::X24_X25, AArch64::X26_X27, AArch64::X28_FP, 
  };

  // XSeqPairsClass_with_sube64_in_GPR64noip Bit set.
  const uint8_t XSeqPairsClass_with_sube64_in_GPR64noipBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x7f, 0x1f, 
  };

  // XSeqPairsClass_with_sube64_in_tcGPR64 Register Class...
  const MCPhysReg XSeqPairsClass_with_sube64_in_tcGPR64[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X16_X17, AArch64::X18_X19, 
  };

  // XSeqPairsClass_with_sube64_in_tcGPR64 Bit set.
  const uint8_t XSeqPairsClass_with_sube64_in_tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x01, 
  };

  // XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64 Register Class...
  const MCPhysReg XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X18_X19, 
  };

  // XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64 Bit set.
  const uint8_t XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7f, 0x01, 
  };

  // XSeqPairsClass_with_subo64_in_tcGPR64 Register Class...
  const MCPhysReg XSeqPairsClass_with_subo64_in_tcGPR64[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, AArch64::X16_X17, 
  };

  // XSeqPairsClass_with_subo64_in_tcGPR64 Bit set.
  const uint8_t XSeqPairsClass_with_subo64_in_tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64 Register Class...
  const MCPhysReg XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, AArch64::X8_X9, AArch64::X10_X11, AArch64::X12_X13, AArch64::X14_X15, 
  };

  // XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64 Bit set.
  const uint8_t XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7f, 
  };

  // XSeqPairsClass_with_sub_32_in_GPR32arg Register Class...
  const MCPhysReg XSeqPairsClass_with_sub_32_in_GPR32arg[] = {
    AArch64::X0_X1, AArch64::X2_X3, AArch64::X4_X5, AArch64::X6_X7, 
  };

  // XSeqPairsClass_with_sub_32_in_GPR32arg Bit set.
  const uint8_t XSeqPairsClass_with_sub_32_in_GPR32argBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 
  };

  // XSeqPairsClass_with_sube64_in_rtcGPR64 Register Class...
  const MCPhysReg XSeqPairsClass_with_sube64_in_rtcGPR64[] = {
    AArch64::X16_X17, 
  };

  // XSeqPairsClass_with_sube64_in_rtcGPR64 Bit set.
  const uint8_t XSeqPairsClass_with_sube64_in_rtcGPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 
  };

  // FPR128 Register Class...
  const MCPhysReg FPR128[] = {
    AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 
  };

  // FPR128 Bit set.
  const uint8_t FPR128Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // ZPR Register Class...
  const MCPhysReg ZPR[] = {
    AArch64::Z0, AArch64::Z1, AArch64::Z2, AArch64::Z3, AArch64::Z4, AArch64::Z5, AArch64::Z6, AArch64::Z7, AArch64::Z8, AArch64::Z9, AArch64::Z10, AArch64::Z11, AArch64::Z12, AArch64::Z13, AArch64::Z14, AArch64::Z15, AArch64::Z16, AArch64::Z17, AArch64::Z18, AArch64::Z19, AArch64::Z20, AArch64::Z21, AArch64::Z22, AArch64::Z23, AArch64::Z24, AArch64::Z25, AArch64::Z26, AArch64::Z27, AArch64::Z28, AArch64::Z29, AArch64::Z30, AArch64::Z31, 
  };

  // ZPR Bit set.
  const uint8_t ZPRBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // FPR128_lo Register Class...
  const MCPhysReg FPR128_lo[] = {
    AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, 
  };

  // FPR128_lo Bit set.
  const uint8_t FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, 
  };

  // ZPR_4b Register Class...
  const MCPhysReg ZPR_4b[] = {
    AArch64::Z0, AArch64::Z1, AArch64::Z2, AArch64::Z3, AArch64::Z4, AArch64::Z5, AArch64::Z6, AArch64::Z7, AArch64::Z8, AArch64::Z9, AArch64::Z10, AArch64::Z11, AArch64::Z12, AArch64::Z13, AArch64::Z14, AArch64::Z15, 
  };

  // ZPR_4b Bit set.
  const uint8_t ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // ZPR_3b Register Class...
  const MCPhysReg ZPR_3b[] = {
    AArch64::Z0, AArch64::Z1, AArch64::Z2, AArch64::Z3, AArch64::Z4, AArch64::Z5, AArch64::Z6, AArch64::Z7, 
  };

  // ZPR_3b Bit set.
  const uint8_t ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 
  };

  // DDD Register Class...
  const MCPhysReg DDD[] = {
    AArch64::D0_D1_D2, AArch64::D1_D2_D3, AArch64::D2_D3_D4, AArch64::D3_D4_D5, AArch64::D4_D5_D6, AArch64::D5_D6_D7, AArch64::D6_D7_D8, AArch64::D7_D8_D9, AArch64::D8_D9_D10, AArch64::D9_D10_D11, AArch64::D10_D11_D12, AArch64::D11_D12_D13, AArch64::D12_D13_D14, AArch64::D13_D14_D15, AArch64::D14_D15_D16, AArch64::D15_D16_D17, AArch64::D16_D17_D18, AArch64::D17_D18_D19, AArch64::D18_D19_D20, AArch64::D19_D20_D21, AArch64::D20_D21_D22, AArch64::D21_D22_D23, AArch64::D22_D23_D24, AArch64::D23_D24_D25, AArch64::D24_D25_D26, AArch64::D25_D26_D27, AArch64::D26_D27_D28, AArch64::D27_D28_D29, AArch64::D28_D29_D30, AArch64::D29_D30_D31, AArch64::D30_D31_D0, AArch64::D31_D0_D1, 
  };

  // DDD Bit set.
  const uint8_t DDDBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // DDDD Register Class...
  const MCPhysReg DDDD[] = {
    AArch64::D0_D1_D2_D3, AArch64::D1_D2_D3_D4, AArch64::D2_D3_D4_D5, AArch64::D3_D4_D5_D6, AArch64::D4_D5_D6_D7, AArch64::D5_D6_D7_D8, AArch64::D6_D7_D8_D9, AArch64::D7_D8_D9_D10, AArch64::D8_D9_D10_D11, AArch64::D9_D10_D11_D12, AArch64::D10_D11_D12_D13, AArch64::D11_D12_D13_D14, AArch64::D12_D13_D14_D15, AArch64::D13_D14_D15_D16, AArch64::D14_D15_D16_D17, AArch64::D15_D16_D17_D18, AArch64::D16_D17_D18_D19, AArch64::D17_D18_D19_D20, AArch64::D18_D19_D20_D21, AArch64::D19_D20_D21_D22, AArch64::D20_D21_D22_D23, AArch64::D21_D22_D23_D24, AArch64::D22_D23_D24_D25, AArch64::D23_D24_D25_D26, AArch64::D24_D25_D26_D27, AArch64::D25_D26_D27_D28, AArch64::D26_D27_D28_D29, AArch64::D27_D28_D29_D30, AArch64::D28_D29_D30_D31, AArch64::D29_D30_D31_D0, AArch64::D30_D31_D0_D1, AArch64::D31_D0_D1_D2, 
  };

  // DDDD Bit set.
  const uint8_t DDDDBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // QQ Register Class...
  const MCPhysReg QQ[] = {
    AArch64::Q0_Q1, AArch64::Q1_Q2, AArch64::Q2_Q3, AArch64::Q3_Q4, AArch64::Q4_Q5, AArch64::Q5_Q6, AArch64::Q6_Q7, AArch64::Q7_Q8, AArch64::Q8_Q9, AArch64::Q9_Q10, AArch64::Q10_Q11, AArch64::Q11_Q12, AArch64::Q12_Q13, AArch64::Q13_Q14, AArch64::Q14_Q15, AArch64::Q15_Q16, AArch64::Q16_Q17, AArch64::Q17_Q18, AArch64::Q18_Q19, AArch64::Q19_Q20, AArch64::Q20_Q21, AArch64::Q21_Q22, AArch64::Q22_Q23, AArch64::Q23_Q24, AArch64::Q24_Q25, AArch64::Q25_Q26, AArch64::Q26_Q27, AArch64::Q27_Q28, AArch64::Q28_Q29, AArch64::Q29_Q30, AArch64::Q30_Q31, AArch64::Q31_Q0, 
  };

  // QQ Bit set.
  const uint8_t QQBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // ZPR2 Register Class...
  const MCPhysReg ZPR2[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z7_Z8, AArch64::Z8_Z9, AArch64::Z9_Z10, AArch64::Z10_Z11, AArch64::Z11_Z12, AArch64::Z12_Z13, AArch64::Z13_Z14, AArch64::Z14_Z15, AArch64::Z15_Z16, AArch64::Z16_Z17, AArch64::Z17_Z18, AArch64::Z18_Z19, AArch64::Z19_Z20, AArch64::Z20_Z21, AArch64::Z21_Z22, AArch64::Z22_Z23, AArch64::Z23_Z24, AArch64::Z24_Z25, AArch64::Z25_Z26, AArch64::Z26_Z27, AArch64::Z27_Z28, AArch64::Z28_Z29, AArch64::Z29_Z30, AArch64::Z30_Z31, AArch64::Z31_Z0, 
  };

  // ZPR2 Bit set.
  const uint8_t ZPR2Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // QQ_with_qsub0_in_FPR128_lo Register Class...
  const MCPhysReg QQ_with_qsub0_in_FPR128_lo[] = {
    AArch64::Q0_Q1, AArch64::Q1_Q2, AArch64::Q2_Q3, AArch64::Q3_Q4, AArch64::Q4_Q5, AArch64::Q5_Q6, AArch64::Q6_Q7, AArch64::Q7_Q8, AArch64::Q8_Q9, AArch64::Q9_Q10, AArch64::Q10_Q11, AArch64::Q11_Q12, AArch64::Q12_Q13, AArch64::Q13_Q14, AArch64::Q14_Q15, AArch64::Q15_Q16, 
  };

  // QQ_with_qsub0_in_FPR128_lo Bit set.
  const uint8_t QQ_with_qsub0_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1, AArch64::Q1_Q2, AArch64::Q2_Q3, AArch64::Q3_Q4, AArch64::Q4_Q5, AArch64::Q5_Q6, AArch64::Q6_Q7, AArch64::Q7_Q8, AArch64::Q8_Q9, AArch64::Q9_Q10, AArch64::Q10_Q11, AArch64::Q11_Q12, AArch64::Q12_Q13, AArch64::Q13_Q14, AArch64::Q14_Q15, AArch64::Q31_Q0, 
  };

  // QQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // ZPR2_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR2_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z7_Z8, AArch64::Z8_Z9, AArch64::Z9_Z10, AArch64::Z10_Z11, AArch64::Z11_Z12, AArch64::Z12_Z13, AArch64::Z13_Z14, AArch64::Z14_Z15, AArch64::Z31_Z0, 
  };

  // ZPR2_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR2_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // ZPR2_with_zsub_in_FPR128_lo Register Class...
  const MCPhysReg ZPR2_with_zsub_in_FPR128_lo[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z7_Z8, AArch64::Z8_Z9, AArch64::Z9_Z10, AArch64::Z10_Z11, AArch64::Z11_Z12, AArch64::Z12_Z13, AArch64::Z13_Z14, AArch64::Z14_Z15, AArch64::Z15_Z16, 
  };

  // ZPR2_with_zsub_in_FPR128_lo Bit set.
  const uint8_t ZPR2_with_zsub_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1, AArch64::Q1_Q2, AArch64::Q2_Q3, AArch64::Q3_Q4, AArch64::Q4_Q5, AArch64::Q5_Q6, AArch64::Q6_Q7, AArch64::Q7_Q8, AArch64::Q8_Q9, AArch64::Q9_Q10, AArch64::Q10_Q11, AArch64::Q11_Q12, AArch64::Q12_Q13, AArch64::Q13_Q14, AArch64::Q14_Q15, 
  };

  // QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z7_Z8, AArch64::Z8_Z9, AArch64::Z9_Z10, AArch64::Z10_Z11, AArch64::Z11_Z12, AArch64::Z12_Z13, AArch64::Z13_Z14, AArch64::Z14_Z15, 
  };

  // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // ZPR2_with_zsub0_in_ZPR_3b Register Class...
  const MCPhysReg ZPR2_with_zsub0_in_ZPR_3b[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z7_Z8, 
  };

  // ZPR2_with_zsub0_in_ZPR_3b Bit set.
  const uint8_t ZPR2_with_zsub0_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 
  };

  // ZPR2_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR2_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, AArch64::Z31_Z0, 
  };

  // ZPR2_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR2_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x10, 
  };

  // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1, AArch64::Z1_Z2, AArch64::Z2_Z3, AArch64::Z3_Z4, AArch64::Z4_Z5, AArch64::Z5_Z6, AArch64::Z6_Z7, 
  };

  // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 
  };

  // QQQ Register Class...
  const MCPhysReg QQQ[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q14_Q15_Q16, AArch64::Q15_Q16_Q17, AArch64::Q16_Q17_Q18, AArch64::Q17_Q18_Q19, AArch64::Q18_Q19_Q20, AArch64::Q19_Q20_Q21, AArch64::Q20_Q21_Q22, AArch64::Q21_Q22_Q23, AArch64::Q22_Q23_Q24, AArch64::Q23_Q24_Q25, AArch64::Q24_Q25_Q26, AArch64::Q25_Q26_Q27, AArch64::Q26_Q27_Q28, AArch64::Q27_Q28_Q29, AArch64::Q28_Q29_Q30, AArch64::Q29_Q30_Q31, AArch64::Q30_Q31_Q0, AArch64::Q31_Q0_Q1, 
  };

  // QQQ Bit set.
  const uint8_t QQQBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // ZPR3 Register Class...
  const MCPhysReg ZPR3[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z14_Z15_Z16, AArch64::Z15_Z16_Z17, AArch64::Z16_Z17_Z18, AArch64::Z17_Z18_Z19, AArch64::Z18_Z19_Z20, AArch64::Z19_Z20_Z21, AArch64::Z20_Z21_Z22, AArch64::Z21_Z22_Z23, AArch64::Z22_Z23_Z24, AArch64::Z23_Z24_Z25, AArch64::Z24_Z25_Z26, AArch64::Z25_Z26_Z27, AArch64::Z26_Z27_Z28, AArch64::Z27_Z28_Z29, AArch64::Z28_Z29_Z30, AArch64::Z29_Z30_Z31, AArch64::Z30_Z31_Z0, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3 Bit set.
  const uint8_t ZPR3Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // QQQ_with_qsub0_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub0_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q14_Q15_Q16, AArch64::Q15_Q16_Q17, 
  };

  // QQQ_with_qsub0_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub0_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q14_Q15_Q16, AArch64::Q31_Q0_Q1, 
  };

  // QQQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // QQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q30_Q31_Q0, AArch64::Q31_Q0_Q1, 
  };

  // QQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x18, 
  };

  // ZPR3_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR3_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z14_Z15_Z16, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR3_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // ZPR3_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR3_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z30_Z31_Z0, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR3_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x18, 
  };

  // ZPR3_with_zsub_in_FPR128_lo Register Class...
  const MCPhysReg ZPR3_with_zsub_in_FPR128_lo[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z14_Z15_Z16, AArch64::Z15_Z16_Z17, 
  };

  // ZPR3_with_zsub_in_FPR128_lo Bit set.
  const uint8_t ZPR3_with_zsub_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q14_Q15_Q16, 
  };

  // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, AArch64::Q31_Q0_Q1, 
  };

  // QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x10, 
  };

  // ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x10, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, AArch64::Z14_Z15_Z16, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2, AArch64::Q1_Q2_Q3, AArch64::Q2_Q3_Q4, AArch64::Q3_Q4_Q5, AArch64::Q4_Q5_Q6, AArch64::Q5_Q6_Q7, AArch64::Q6_Q7_Q8, AArch64::Q7_Q8_Q9, AArch64::Q8_Q9_Q10, AArch64::Q9_Q10_Q11, AArch64::Q10_Q11_Q12, AArch64::Q11_Q12_Q13, AArch64::Q12_Q13_Q14, AArch64::Q13_Q14_Q15, 
  };

  // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, AArch64::Z8_Z9_Z10, AArch64::Z9_Z10_Z11, AArch64::Z10_Z11_Z12, AArch64::Z11_Z12_Z13, AArch64::Z12_Z13_Z14, AArch64::Z13_Z14_Z15, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 
  };

  // ZPR3_with_zsub0_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub0_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z7_Z8_Z9, 
  };

  // ZPR3_with_zsub0_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub0_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 
  };

  // ZPR3_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x10, 
  };

  // ZPR3_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z30_Z31_Z0, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x18, 
  };

  // ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z31_Z0_Z1, 
  };

  // ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x10, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, AArch64::Z6_Z7_Z8, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2, AArch64::Z1_Z2_Z3, AArch64::Z2_Z3_Z4, AArch64::Z3_Z4_Z5, AArch64::Z4_Z5_Z6, AArch64::Z5_Z6_Z7, 
  };

  // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 
  };

  // QQQQ Register Class...
  const MCPhysReg QQQQ[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q14_Q15_Q16_Q17, AArch64::Q15_Q16_Q17_Q18, AArch64::Q16_Q17_Q18_Q19, AArch64::Q17_Q18_Q19_Q20, AArch64::Q18_Q19_Q20_Q21, AArch64::Q19_Q20_Q21_Q22, AArch64::Q20_Q21_Q22_Q23, AArch64::Q21_Q22_Q23_Q24, AArch64::Q22_Q23_Q24_Q25, AArch64::Q23_Q24_Q25_Q26, AArch64::Q24_Q25_Q26_Q27, AArch64::Q25_Q26_Q27_Q28, AArch64::Q26_Q27_Q28_Q29, AArch64::Q27_Q28_Q29_Q30, AArch64::Q28_Q29_Q30_Q31, AArch64::Q29_Q30_Q31_Q0, AArch64::Q30_Q31_Q0_Q1, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ Bit set.
  const uint8_t QQQQBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // ZPR4 Register Class...
  const MCPhysReg ZPR4[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z14_Z15_Z16_Z17, AArch64::Z15_Z16_Z17_Z18, AArch64::Z16_Z17_Z18_Z19, AArch64::Z17_Z18_Z19_Z20, AArch64::Z18_Z19_Z20_Z21, AArch64::Z19_Z20_Z21_Z22, AArch64::Z20_Z21_Z22_Z23, AArch64::Z21_Z22_Z23_Z24, AArch64::Z22_Z23_Z24_Z25, AArch64::Z23_Z24_Z25_Z26, AArch64::Z24_Z25_Z26_Z27, AArch64::Z25_Z26_Z27_Z28, AArch64::Z26_Z27_Z28_Z29, AArch64::Z27_Z28_Z29_Z30, AArch64::Z28_Z29_Z30_Z31, AArch64::Z29_Z30_Z31_Z0, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4 Bit set.
  const uint8_t ZPR4Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub0_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q14_Q15_Q16_Q17, AArch64::Q15_Q16_Q17_Q18, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub0_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q14_Q15_Q16_Q17, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // QQQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q30_Q31_Q0_Q1, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x18, 
  };

  // QQQQ_with_qsub3_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub3_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q29_Q30_Q31_Q0, AArch64::Q30_Q31_Q0_Q1, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub3_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub3_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x1c, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z14_Z15_Z16_Z17, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x10, 
  };

  // ZPR4_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x18, 
  };

  // ZPR4_with_zsub3_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub3_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z29_Z30_Z31_Z0, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub3_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub3_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x1c, 
  };

  // ZPR4_with_zsub_in_FPR128_lo Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z14_Z15_Z16_Z17, AArch64::Z15_Z16_Z17_Z18, 
  };

  // ZPR4_with_zsub_in_FPR128_lo Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q14_Q15_Q16_Q17, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x10, 
  };

  // QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q30_Q31_Q0_Q1, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x18, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 0x00, 0x10, 
  };

  // ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x18, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, AArch64::Z14_Z15_Z16_Z17, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x0f, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q13_Q14_Q15_Q16, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, AArch64::Q31_Q0_Q1_Q2, 
  };

  // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x10, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x10, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, AArch64::Z13_Z14_Z15_Z16, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Register Class...
  const MCPhysReg QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo[] = {
    AArch64::Q0_Q1_Q2_Q3, AArch64::Q1_Q2_Q3_Q4, AArch64::Q2_Q3_Q4_Q5, AArch64::Q3_Q4_Q5_Q6, AArch64::Q4_Q5_Q6_Q7, AArch64::Q5_Q6_Q7_Q8, AArch64::Q6_Q7_Q8_Q9, AArch64::Q7_Q8_Q9_Q10, AArch64::Q8_Q9_Q10_Q11, AArch64::Q9_Q10_Q11_Q12, AArch64::Q10_Q11_Q12_Q13, AArch64::Q11_Q12_Q13_Q14, AArch64::Q12_Q13_Q14_Q15, 
  };

  // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo Bit set.
  const uint8_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, AArch64::Z8_Z9_Z10_Z11, AArch64::Z9_Z10_Z11_Z12, AArch64::Z10_Z11_Z12_Z13, AArch64::Z11_Z12_Z13_Z14, AArch64::Z12_Z13_Z14_Z15, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 
  };

  // ZPR4_with_zsub0_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub0_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z7_Z8_Z9_Z10, 
  };

  // ZPR4_with_zsub0_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub0_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x10, 
  };

  // ZPR4_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x18, 
  };

  // ZPR4_with_zsub3_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub3_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z29_Z30_Z31_Z0, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub3_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub3_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x1c, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x10, 
  };

  // ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z30_Z31_Z0_Z1, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x18, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, AArch64::Z6_Z7_Z8_Z9, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z31_Z0_Z1_Z2, 
  };

  // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x10, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, AArch64::Z5_Z6_Z7_Z8, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b Register Class...
  const MCPhysReg ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b[] = {
    AArch64::Z0_Z1_Z2_Z3, AArch64::Z1_Z2_Z3_Z4, AArch64::Z2_Z3_Z4_Z5, AArch64::Z3_Z4_Z5_Z6, AArch64::Z4_Z5_Z6_Z7, 
  };

  // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b Bit set.
  const uint8_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 
  };

} // end anonymous namespace

extern const char AArch64RegClassStrings[] = {
  /* 0 */ 'F', 'P', 'R', '3', '2', 0,
  /* 6 */ 'G', 'P', 'R', '3', '2', 0,
  /* 12 */ 'Z', 'P', 'R', '2', 0,
  /* 17 */ 'Z', 'P', 'R', '3', 0,
  /* 22 */ 'F', 'P', 'R', '6', '4', 0,
  /* 28 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'e', '6', '4', '_', 'i', 'n', '_', 'G', 'P', 'R', '6', '4', 'n', 'o', 'i', 'p', '_', 'a', 'n', 'd', '_', 't', 'c', 'G', 'P', 'R', '6', '4', 0,
  /* 80 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'o', '6', '4', '_', 'i', 'n', '_', 'G', 'P', 'R', '6', '4', 'n', 'o', 'i', 'p', '_', 'a', 'n', 'd', '_', 't', 'c', 'G', 'P', 'R', '6', '4', 0,
  /* 132 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'e', '6', '4', '_', 'i', 'n', '_', 't', 'c', 'G', 'P', 'R', '6', '4', 0,
  /* 170 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'o', '6', '4', '_', 'i', 'n', '_', 't', 'c', 'G', 'P', 'R', '6', '4', 0,
  /* 208 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'e', '6', '4', '_', 'i', 'n', '_', 'r', 't', 'c', 'G', 'P', 'R', '6', '4', 0,
  /* 247 */ 'Z', 'P', 'R', '4', 0,
  /* 252 */ 'F', 'P', 'R', '1', '6', 0,
  /* 258 */ 'F', 'P', 'R', '1', '2', '8', 0,
  /* 265 */ 'F', 'P', 'R', '8', 0,
  /* 270 */ 'D', 'D', 'D', 'D', 0,
  /* 275 */ 'Q', 'Q', 'Q', 'Q', 0,
  /* 280 */ 'C', 'C', 'R', 0,
  /* 284 */ 'P', 'P', 'R', 0,
  /* 288 */ 'Z', 'P', 'R', 0,
  /* 292 */ 'P', 'P', 'R', '_', '3', 'b', 0,
  /* 299 */ 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 325 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 351 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 377 */ 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 435 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 493 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 551 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 607 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 665 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 721 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 779 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 835 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 891 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '3', 'b', 0,
  /* 949 */ 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1007 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1065 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1123 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1179 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1237 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1293 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1351 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1407 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1463 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'Z', 'P', 'R', '_', '4', 'b', 0,
  /* 1521 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', '_', '3', '2', '_', 'i', 'n', '_', 'G', 'P', 'R', '3', '2', 'a', 'r', 'g', 0,
  /* 1560 */ 'W', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'e', '3', '2', '_', 'i', 'n', '_', 'G', 'P', 'R', '3', '2', 'a', 'r', 'g', 0,
  /* 1599 */ 'G', 'P', 'R', '6', '4', 'a', 'r', 'g', 0,
  /* 1608 */ 'G', 'P', 'R', '3', '2', 'a', 'l', 'l', 0,
  /* 1617 */ 'G', 'P', 'R', '6', '4', 'a', 'l', 'l', 0,
  /* 1626 */ 'W', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'o', '3', '2', '_', 'i', 'n', '_', 'G', 'P', 'R', '3', '2', 'c', 'o', 'm', 'm', 'o', 'n', 0,
  /* 1668 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'o', '6', '4', '_', 'i', 'n', '_', 'G', 'P', 'R', '6', '4', 'c', 'o', 'm', 'm', 'o', 'n', 0,
  /* 1710 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 1739 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 1801 */ 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 1861 */ 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 1919 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 1981 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2043 */ 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2103 */ 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2163 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '0', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2225 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '1', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2287 */ 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '2', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', '_', 'a', 'n', 'd', '_', 'Q', 'Q', 'Q', 'Q', '_', 'w', 'i', 't', 'h', '_', 'q', 's', 'u', 'b', '3', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2349 */ 'Z', 'P', 'R', '2', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2377 */ 'Z', 'P', 'R', '3', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2405 */ 'Z', 'P', 'R', '4', '_', 'w', 'i', 't', 'h', '_', 'z', 's', 'u', 'b', '_', 'i', 'n', '_', 'F', 'P', 'R', '1', '2', '8', '_', 'l', 'o', 0,
  /* 2433 */ 'G', 'P', 'R', '6', '4', 'c', 'o', 'm', 'm', 'o', 'n', '_', 'a', 'n', 'd', '_', 'G', 'P', 'R', '6', '4', 'n', 'o', 'i', 'p', 0,
  /* 2459 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'e', '6', '4', '_', 'i', 'n', '_', 'G', 'P', 'R', '6', '4', 'n', 'o', 'i', 'p', 0,
  /* 2499 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', '_', 'w', 'i', 't', 'h', '_', 's', 'u', 'b', 'o', '6', '4', '_', 'i', 'n', '_', 'G', 'P', 'R', '6', '4', 'n', 'o', 'i', 'p', 0,
  /* 2539 */ 'G', 'P', 'R', '3', '2', 's', 'p', 0,
  /* 2547 */ 'G', 'P', 'R', '6', '4', 's', 'p', 0,
  /* 2555 */ 'W', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', 0,
  /* 2570 */ 'X', 'S', 'e', 'q', 'P', 'a', 'i', 'r', 's', 'C', 'l', 'a', 's', 's', 0,
  /* 2585 */ 'G', 'P', 'R', '3', '2', 's', 'p', 'o', 'n', 'l', 'y', 0,
  /* 2597 */ 'G', 'P', 'R', '6', '4', 's', 'p', 'o', 'n', 'l', 'y', 0,
};

extern const MCRegisterClass AArch64MCRegisterClasses[] = {
  { FPR8, FPR8Bits, 265, 32, sizeof(FPR8Bits), AArch64::FPR8RegClassID, 1, true },
  { FPR16, FPR16Bits, 252, 32, sizeof(FPR16Bits), AArch64::FPR16RegClassID, 1, true },
  { PPR, PPRBits, 284, 16, sizeof(PPRBits), AArch64::PPRRegClassID, 1, true },
  { PPR_3b, PPR_3bBits, 292, 8, sizeof(PPR_3bBits), AArch64::PPR_3bRegClassID, 1, true },
  { GPR32all, GPR32allBits, 1608, 33, sizeof(GPR32allBits), AArch64::GPR32allRegClassID, 1, true },
  { FPR32, FPR32Bits, 0, 32, sizeof(FPR32Bits), AArch64::FPR32RegClassID, 1, true },
  { GPR32, GPR32Bits, 6, 32, sizeof(GPR32Bits), AArch64::GPR32RegClassID, 1, true },
  { GPR32sp, GPR32spBits, 2539, 32, sizeof(GPR32spBits), AArch64::GPR32spRegClassID, 1, true },
  { GPR32common, GPR32commonBits, 1656, 31, sizeof(GPR32commonBits), AArch64::GPR32commonRegClassID, 1, true },
  { GPR32arg, GPR32argBits, 1551, 8, sizeof(GPR32argBits), AArch64::GPR32argRegClassID, 1, true },
  { CCR, CCRBits, 280, 1, sizeof(CCRBits), AArch64::CCRRegClassID, -1, false },
  { GPR32sponly, GPR32sponlyBits, 2585, 1, sizeof(GPR32sponlyBits), AArch64::GPR32sponlyRegClassID, 1, true },
  { WSeqPairsClass, WSeqPairsClassBits, 2555, 16, sizeof(WSeqPairsClassBits), AArch64::WSeqPairsClassRegClassID, 1, true },
  { WSeqPairsClass_with_subo32_in_GPR32common, WSeqPairsClass_with_subo32_in_GPR32commonBits, 1626, 15, sizeof(WSeqPairsClass_with_subo32_in_GPR32commonBits), AArch64::WSeqPairsClass_with_subo32_in_GPR32commonRegClassID, 1, true },
  { WSeqPairsClass_with_sube32_in_GPR32arg, WSeqPairsClass_with_sube32_in_GPR32argBits, 1560, 4, sizeof(WSeqPairsClass_with_sube32_in_GPR32argBits), AArch64::WSeqPairsClass_with_sube32_in_GPR32argRegClassID, 1, true },
  { GPR64all, GPR64allBits, 1617, 33, sizeof(GPR64allBits), AArch64::GPR64allRegClassID, 1, true },
  { FPR64, FPR64Bits, 22, 32, sizeof(FPR64Bits), AArch64::FPR64RegClassID, 1, true },
  { GPR64, GPR64Bits, 74, 32, sizeof(GPR64Bits), AArch64::GPR64RegClassID, 1, true },
  { GPR64sp, GPR64spBits, 2547, 32, sizeof(GPR64spBits), AArch64::GPR64spRegClassID, 1, true },
  { GPR64common, GPR64commonBits, 1698, 31, sizeof(GPR64commonBits), AArch64::GPR64commonRegClassID, 1, true },
  { GPR64noip, GPR64noipBits, 2449, 29, sizeof(GPR64noipBits), AArch64::GPR64noipRegClassID, 1, true },
  { GPR64common_and_GPR64noip, GPR64common_and_GPR64noipBits, 2433, 28, sizeof(GPR64common_and_GPR64noipBits), AArch64::GPR64common_and_GPR64noipRegClassID, 1, true },
  { tcGPR64, tcGPR64Bits, 72, 19, sizeof(tcGPR64Bits), AArch64::tcGPR64RegClassID, 1, true },
  { GPR64noip_and_tcGPR64, GPR64noip_and_tcGPR64Bits, 58, 17, sizeof(GPR64noip_and_tcGPR64Bits), AArch64::GPR64noip_and_tcGPR64RegClassID, 1, true },
  { GPR64arg, GPR64argBits, 1599, 8, sizeof(GPR64argBits), AArch64::GPR64argRegClassID, 1, true },
  { rtcGPR64, rtcGPR64Bits, 238, 2, sizeof(rtcGPR64Bits), AArch64::rtcGPR64RegClassID, 1, true },
  { GPR64sponly, GPR64sponlyBits, 2597, 1, sizeof(GPR64sponlyBits), AArch64::GPR64sponlyRegClassID, 1, true },
  { DD, DDBits, 272, 32, sizeof(DDBits), AArch64::DDRegClassID, 1, true },
  { XSeqPairsClass, XSeqPairsClassBits, 2570, 16, sizeof(XSeqPairsClassBits), AArch64::XSeqPairsClassRegClassID, 1, true },
  { XSeqPairsClass_with_subo64_in_GPR64common, XSeqPairsClass_with_subo64_in_GPR64commonBits, 1668, 15, sizeof(XSeqPairsClass_with_subo64_in_GPR64commonBits), AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClassID, 1, true },
  { XSeqPairsClass_with_subo64_in_GPR64noip, XSeqPairsClass_with_subo64_in_GPR64noipBits, 2499, 15, sizeof(XSeqPairsClass_with_subo64_in_GPR64noipBits), AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClassID, 1, true },
  { XSeqPairsClass_with_sube64_in_GPR64noip, XSeqPairsClass_with_sube64_in_GPR64noipBits, 2459, 14, sizeof(XSeqPairsClass_with_sube64_in_GPR64noipBits), AArch64::XSeqPairsClass_with_sube64_in_GPR64noipRegClassID, 1, true },
  { XSeqPairsClass_with_sube64_in_tcGPR64, XSeqPairsClass_with_sube64_in_tcGPR64Bits, 132, 10, sizeof(XSeqPairsClass_with_sube64_in_tcGPR64Bits), AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClassID, 1, true },
  { XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64, XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64Bits, 28, 9, sizeof(XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64Bits), AArch64::XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClassID, 1, true },
  { XSeqPairsClass_with_subo64_in_tcGPR64, XSeqPairsClass_with_subo64_in_tcGPR64Bits, 170, 9, sizeof(XSeqPairsClass_with_subo64_in_tcGPR64Bits), AArch64::XSeqPairsClass_with_subo64_in_tcGPR64RegClassID, 1, true },
  { XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64, XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64Bits, 80, 8, sizeof(XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64Bits), AArch64::XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClassID, 1, true },
  { XSeqPairsClass_with_sub_32_in_GPR32arg, XSeqPairsClass_with_sub_32_in_GPR32argBits, 1521, 4, sizeof(XSeqPairsClass_with_sub_32_in_GPR32argBits), AArch64::XSeqPairsClass_with_sub_32_in_GPR32argRegClassID, 1, true },
  { XSeqPairsClass_with_sube64_in_rtcGPR64, XSeqPairsClass_with_sube64_in_rtcGPR64Bits, 208, 1, sizeof(XSeqPairsClass_with_sube64_in_rtcGPR64Bits), AArch64::XSeqPairsClass_with_sube64_in_rtcGPR64RegClassID, 1, true },
  { FPR128, FPR128Bits, 258, 32, sizeof(FPR128Bits), AArch64::FPR128RegClassID, 1, true },
  { ZPR, ZPRBits, 288, 32, sizeof(ZPRBits), AArch64::ZPRRegClassID, 1, true },
  { FPR128_lo, FPR128_loBits, 1729, 16, sizeof(FPR128_loBits), AArch64::FPR128_loRegClassID, 1, true },
  { ZPR_4b, ZPR_4bBits, 1000, 16, sizeof(ZPR_4bBits), AArch64::ZPR_4bRegClassID, 1, true },
  { ZPR_3b, ZPR_3bBits, 318, 8, sizeof(ZPR_3bBits), AArch64::ZPR_3bRegClassID, 1, true },
  { DDD, DDDBits, 271, 32, sizeof(DDDBits), AArch64::DDDRegClassID, 1, true },
  { DDDD, DDDDBits, 270, 32, sizeof(DDDDBits), AArch64::DDDDRegClassID, 1, true },
  { QQ, QQBits, 277, 32, sizeof(QQBits), AArch64::QQRegClassID, 1, true },
  { ZPR2, ZPR2Bits, 12, 32, sizeof(ZPR2Bits), AArch64::ZPR2RegClassID, 1, true },
  { QQ_with_qsub0_in_FPR128_lo, QQ_with_qsub0_in_FPR128_loBits, 1712, 16, sizeof(QQ_with_qsub0_in_FPR128_loBits), AArch64::QQ_with_qsub0_in_FPR128_loRegClassID, 1, true },
  { QQ_with_qsub1_in_FPR128_lo, QQ_with_qsub1_in_FPR128_loBits, 1774, 16, sizeof(QQ_with_qsub1_in_FPR128_loBits), AArch64::QQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { ZPR2_with_zsub1_in_ZPR_4b, ZPR2_with_zsub1_in_ZPR_4bBits, 981, 16, sizeof(ZPR2_with_zsub1_in_ZPR_4bBits), AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { ZPR2_with_zsub_in_FPR128_lo, ZPR2_with_zsub_in_FPR128_loBits, 2349, 16, sizeof(ZPR2_with_zsub_in_FPR128_loBits), AArch64::ZPR2_with_zsub_in_FPR128_loRegClassID, 1, true },
  { QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo, QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loBits, 1861, 15, sizeof(QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loBits), AArch64::QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b, ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bBits, 949, 15, sizeof(ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bBits), AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { ZPR2_with_zsub0_in_ZPR_3b, ZPR2_with_zsub0_in_ZPR_3bBits, 299, 8, sizeof(ZPR2_with_zsub0_in_ZPR_3bBits), AArch64::ZPR2_with_zsub0_in_ZPR_3bRegClassID, 1, true },
  { ZPR2_with_zsub1_in_ZPR_3b, ZPR2_with_zsub1_in_ZPR_3bBits, 409, 8, sizeof(ZPR2_with_zsub1_in_ZPR_3bBits), AArch64::ZPR2_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b, ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bBits, 377, 7, sizeof(ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bBits), AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { QQQ, QQQBits, 276, 32, sizeof(QQQBits), AArch64::QQQRegClassID, 1, true },
  { ZPR3, ZPR3Bits, 17, 32, sizeof(ZPR3Bits), AArch64::ZPR3RegClassID, 1, true },
  { QQQ_with_qsub0_in_FPR128_lo, QQQ_with_qsub0_in_FPR128_loBits, 1711, 16, sizeof(QQQ_with_qsub0_in_FPR128_loBits), AArch64::QQQ_with_qsub0_in_FPR128_loRegClassID, 1, true },
  { QQQ_with_qsub1_in_FPR128_lo, QQQ_with_qsub1_in_FPR128_loBits, 1773, 16, sizeof(QQQ_with_qsub1_in_FPR128_loBits), AArch64::QQQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { QQQ_with_qsub2_in_FPR128_lo, QQQ_with_qsub2_in_FPR128_loBits, 1953, 16, sizeof(QQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { ZPR3_with_zsub1_in_ZPR_4b, ZPR3_with_zsub1_in_ZPR_4bBits, 1039, 16, sizeof(ZPR3_with_zsub1_in_ZPR_4bBits), AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { ZPR3_with_zsub2_in_ZPR_4b, ZPR3_with_zsub2_in_ZPR_4bBits, 1153, 16, sizeof(ZPR3_with_zsub2_in_ZPR_4bBits), AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { ZPR3_with_zsub_in_FPR128_lo, ZPR3_with_zsub_in_FPR128_loBits, 2377, 16, sizeof(ZPR3_with_zsub_in_FPR128_loBits), AArch64::ZPR3_with_zsub_in_FPR128_loRegClassID, 1, true },
  { QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo, QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loBits, 1801, 15, sizeof(QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loBits), AArch64::QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo, QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits, 2103, 15, sizeof(QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b, ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bBits, 1123, 15, sizeof(ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bBits), AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b, ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bBits, 1007, 15, sizeof(ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bBits), AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo, QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits, 2043, 14, sizeof(QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b, ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bBits, 1179, 14, sizeof(ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bBits), AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { ZPR3_with_zsub0_in_ZPR_3b, ZPR3_with_zsub0_in_ZPR_3bBits, 325, 8, sizeof(ZPR3_with_zsub0_in_ZPR_3bBits), AArch64::ZPR3_with_zsub0_in_ZPR_3bRegClassID, 1, true },
  { ZPR3_with_zsub1_in_ZPR_3b, ZPR3_with_zsub1_in_ZPR_3bBits, 467, 8, sizeof(ZPR3_with_zsub1_in_ZPR_3bBits), AArch64::ZPR3_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { ZPR3_with_zsub2_in_ZPR_3b, ZPR3_with_zsub2_in_ZPR_3bBits, 581, 8, sizeof(ZPR3_with_zsub2_in_ZPR_3bBits), AArch64::ZPR3_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b, ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bBits, 551, 7, sizeof(ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bBits), AArch64::ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b, ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bBits, 435, 7, sizeof(ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bBits), AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b, ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bBits, 607, 6, sizeof(ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bBits), AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { QQQQ, QQQQBits, 275, 32, sizeof(QQQQBits), AArch64::QQQQRegClassID, 1, true },
  { ZPR4, ZPR4Bits, 247, 32, sizeof(ZPR4Bits), AArch64::ZPR4RegClassID, 1, true },
  { QQQQ_with_qsub0_in_FPR128_lo, QQQQ_with_qsub0_in_FPR128_loBits, 1710, 16, sizeof(QQQQ_with_qsub0_in_FPR128_loBits), AArch64::QQQQ_with_qsub0_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub1_in_FPR128_lo, QQQQ_with_qsub1_in_FPR128_loBits, 1772, 16, sizeof(QQQQ_with_qsub1_in_FPR128_loBits), AArch64::QQQQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub2_in_FPR128_lo, QQQQ_with_qsub2_in_FPR128_loBits, 1952, 16, sizeof(QQQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub3_in_FPR128_lo, QQQQ_with_qsub3_in_FPR128_loBits, 2196, 16, sizeof(QQQQ_with_qsub3_in_FPR128_loBits), AArch64::QQQQ_with_qsub3_in_FPR128_loRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_4b, ZPR4_with_zsub1_in_ZPR_4bBits, 1097, 16, sizeof(ZPR4_with_zsub1_in_ZPR_4bBits), AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub2_in_ZPR_4b, ZPR4_with_zsub2_in_ZPR_4bBits, 1267, 16, sizeof(ZPR4_with_zsub2_in_ZPR_4bBits), AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub3_in_ZPR_4b, ZPR4_with_zsub3_in_ZPR_4bBits, 1381, 16, sizeof(ZPR4_with_zsub3_in_ZPR_4bBits), AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo, ZPR4_with_zsub_in_FPR128_loBits, 2405, 16, sizeof(ZPR4_with_zsub_in_FPR128_loBits), AArch64::ZPR4_with_zsub_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo, QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loBits, 1739, 15, sizeof(QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loBits), AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo, QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits, 1981, 15, sizeof(QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo, QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits, 2287, 15, sizeof(QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits), AArch64::QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b, ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bBits, 1237, 15, sizeof(ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bBits), AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b, ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits, 1407, 15, sizeof(ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits), AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bBits, 1065, 15, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClassID, 1, true },
  { QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo, QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits, 1919, 14, sizeof(QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loBits), AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID, 1, true },
  { QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo, QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits, 2225, 14, sizeof(QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits), AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b, ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits, 1351, 14, sizeof(ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bBits), AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bBits, 1293, 14, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID, 1, true },
  { QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo, QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits, 2163, 13, sizeof(QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loBits), AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bBits, 1463, 13, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID, 1, true },
  { ZPR4_with_zsub0_in_ZPR_3b, ZPR4_with_zsub0_in_ZPR_3bBits, 351, 8, sizeof(ZPR4_with_zsub0_in_ZPR_3bBits), AArch64::ZPR4_with_zsub0_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_3b, ZPR4_with_zsub1_in_ZPR_3bBits, 525, 8, sizeof(ZPR4_with_zsub1_in_ZPR_3bBits), AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub2_in_ZPR_3b, ZPR4_with_zsub2_in_ZPR_3bBits, 695, 8, sizeof(ZPR4_with_zsub2_in_ZPR_3bBits), AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub3_in_ZPR_3b, ZPR4_with_zsub3_in_ZPR_3bBits, 809, 8, sizeof(ZPR4_with_zsub3_in_ZPR_3bBits), AArch64::ZPR4_with_zsub3_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b, ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bBits, 665, 7, sizeof(ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bBits), AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b, ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits, 835, 7, sizeof(ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits), AArch64::ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bBits, 493, 7, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b, ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits, 779, 6, sizeof(ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bBits), AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bBits, 721, 6, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID, 1, true },
  { ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b, ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bBits, 891, 5, sizeof(ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bBits), AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID, 1, true },
};

// AArch64 Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair AArch64DwarfFlavour0Dwarf2L[] = {
  { 0U, AArch64::W0 },
  { 1U, AArch64::W1 },
  { 2U, AArch64::W2 },
  { 3U, AArch64::W3 },
  { 4U, AArch64::W4 },
  { 5U, AArch64::W5 },
  { 6U, AArch64::W6 },
  { 7U, AArch64::W7 },
  { 8U, AArch64::W8 },
  { 9U, AArch64::W9 },
  { 10U, AArch64::W10 },
  { 11U, AArch64::W11 },
  { 12U, AArch64::W12 },
  { 13U, AArch64::W13 },
  { 14U, AArch64::W14 },
  { 15U, AArch64::W15 },
  { 16U, AArch64::W16 },
  { 17U, AArch64::W17 },
  { 18U, AArch64::W18 },
  { 19U, AArch64::W19 },
  { 20U, AArch64::W20 },
  { 21U, AArch64::W21 },
  { 22U, AArch64::W22 },
  { 23U, AArch64::W23 },
  { 24U, AArch64::W24 },
  { 25U, AArch64::W25 },
  { 26U, AArch64::W26 },
  { 27U, AArch64::W27 },
  { 28U, AArch64::W28 },
  { 29U, AArch64::W29 },
  { 30U, AArch64::W30 },
  { 31U, AArch64::WSP },
  { 47U, AArch64::FFR },
  { 48U, AArch64::P0 },
  { 49U, AArch64::P1 },
  { 50U, AArch64::P2 },
  { 51U, AArch64::P3 },
  { 52U, AArch64::P4 },
  { 53U, AArch64::P5 },
  { 54U, AArch64::P6 },
  { 55U, AArch64::P7 },
  { 56U, AArch64::P8 },
  { 57U, AArch64::P9 },
  { 58U, AArch64::P10 },
  { 59U, AArch64::P11 },
  { 60U, AArch64::P12 },
  { 61U, AArch64::P13 },
  { 62U, AArch64::P14 },
  { 63U, AArch64::P15 },
  { 64U, AArch64::B0 },
  { 65U, AArch64::B1 },
  { 66U, AArch64::B2 },
  { 67U, AArch64::B3 },
  { 68U, AArch64::B4 },
  { 69U, AArch64::B5 },
  { 70U, AArch64::B6 },
  { 71U, AArch64::B7 },
  { 72U, AArch64::B8 },
  { 73U, AArch64::B9 },
  { 74U, AArch64::B10 },
  { 75U, AArch64::B11 },
  { 76U, AArch64::B12 },
  { 77U, AArch64::B13 },
  { 78U, AArch64::B14 },
  { 79U, AArch64::B15 },
  { 80U, AArch64::B16 },
  { 81U, AArch64::B17 },
  { 82U, AArch64::B18 },
  { 83U, AArch64::B19 },
  { 84U, AArch64::B20 },
  { 85U, AArch64::B21 },
  { 86U, AArch64::B22 },
  { 87U, AArch64::B23 },
  { 88U, AArch64::B24 },
  { 89U, AArch64::B25 },
  { 90U, AArch64::B26 },
  { 91U, AArch64::B27 },
  { 92U, AArch64::B28 },
  { 93U, AArch64::B29 },
  { 94U, AArch64::B30 },
  { 95U, AArch64::B31 },
  { 96U, AArch64::Z0 },
  { 97U, AArch64::Z1 },
  { 98U, AArch64::Z2 },
  { 99U, AArch64::Z3 },
  { 100U, AArch64::Z4 },
  { 101U, AArch64::Z5 },
  { 102U, AArch64::Z6 },
  { 103U, AArch64::Z7 },
  { 104U, AArch64::Z8 },
  { 105U, AArch64::Z9 },
  { 106U, AArch64::Z10 },
  { 107U, AArch64::Z11 },
  { 108U, AArch64::Z12 },
  { 109U, AArch64::Z13 },
  { 110U, AArch64::Z14 },
  { 111U, AArch64::Z15 },
  { 112U, AArch64::Z16 },
  { 113U, AArch64::Z17 },
  { 114U, AArch64::Z18 },
  { 115U, AArch64::Z19 },
  { 116U, AArch64::Z20 },
  { 117U, AArch64::Z21 },
  { 118U, AArch64::Z22 },
  { 119U, AArch64::Z23 },
  { 120U, AArch64::Z24 },
  { 121U, AArch64::Z25 },
  { 122U, AArch64::Z26 },
  { 123U, AArch64::Z27 },
  { 124U, AArch64::Z28 },
  { 125U, AArch64::Z29 },
  { 126U, AArch64::Z30 },
  { 127U, AArch64::Z31 },
};
extern const unsigned AArch64DwarfFlavour0Dwarf2LSize = array_lengthof(AArch64DwarfFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64EHFlavour0Dwarf2L[] = {
  { 0U, AArch64::W0 },
  { 1U, AArch64::W1 },
  { 2U, AArch64::W2 },
  { 3U, AArch64::W3 },
  { 4U, AArch64::W4 },
  { 5U, AArch64::W5 },
  { 6U, AArch64::W6 },
  { 7U, AArch64::W7 },
  { 8U, AArch64::W8 },
  { 9U, AArch64::W9 },
  { 10U, AArch64::W10 },
  { 11U, AArch64::W11 },
  { 12U, AArch64::W12 },
  { 13U, AArch64::W13 },
  { 14U, AArch64::W14 },
  { 15U, AArch64::W15 },
  { 16U, AArch64::W16 },
  { 17U, AArch64::W17 },
  { 18U, AArch64::W18 },
  { 19U, AArch64::W19 },
  { 20U, AArch64::W20 },
  { 21U, AArch64::W21 },
  { 22U, AArch64::W22 },
  { 23U, AArch64::W23 },
  { 24U, AArch64::W24 },
  { 25U, AArch64::W25 },
  { 26U, AArch64::W26 },
  { 27U, AArch64::W27 },
  { 28U, AArch64::W28 },
  { 29U, AArch64::W29 },
  { 30U, AArch64::W30 },
  { 31U, AArch64::WSP },
  { 47U, AArch64::FFR },
  { 48U, AArch64::P0 },
  { 49U, AArch64::P1 },
  { 50U, AArch64::P2 },
  { 51U, AArch64::P3 },
  { 52U, AArch64::P4 },
  { 53U, AArch64::P5 },
  { 54U, AArch64::P6 },
  { 55U, AArch64::P7 },
  { 56U, AArch64::P8 },
  { 57U, AArch64::P9 },
  { 58U, AArch64::P10 },
  { 59U, AArch64::P11 },
  { 60U, AArch64::P12 },
  { 61U, AArch64::P13 },
  { 62U, AArch64::P14 },
  { 63U, AArch64::P15 },
  { 64U, AArch64::B0 },
  { 65U, AArch64::B1 },
  { 66U, AArch64::B2 },
  { 67U, AArch64::B3 },
  { 68U, AArch64::B4 },
  { 69U, AArch64::B5 },
  { 70U, AArch64::B6 },
  { 71U, AArch64::B7 },
  { 72U, AArch64::B8 },
  { 73U, AArch64::B9 },
  { 74U, AArch64::B10 },
  { 75U, AArch64::B11 },
  { 76U, AArch64::B12 },
  { 77U, AArch64::B13 },
  { 78U, AArch64::B14 },
  { 79U, AArch64::B15 },
  { 80U, AArch64::B16 },
  { 81U, AArch64::B17 },
  { 82U, AArch64::B18 },
  { 83U, AArch64::B19 },
  { 84U, AArch64::B20 },
  { 85U, AArch64::B21 },
  { 86U, AArch64::B22 },
  { 87U, AArch64::B23 },
  { 88U, AArch64::B24 },
  { 89U, AArch64::B25 },
  { 90U, AArch64::B26 },
  { 91U, AArch64::B27 },
  { 92U, AArch64::B28 },
  { 93U, AArch64::B29 },
  { 94U, AArch64::B30 },
  { 95U, AArch64::B31 },
  { 96U, AArch64::Z0 },
  { 97U, AArch64::Z1 },
  { 98U, AArch64::Z2 },
  { 99U, AArch64::Z3 },
  { 100U, AArch64::Z4 },
  { 101U, AArch64::Z5 },
  { 102U, AArch64::Z6 },
  { 103U, AArch64::Z7 },
  { 104U, AArch64::Z8 },
  { 105U, AArch64::Z9 },
  { 106U, AArch64::Z10 },
  { 107U, AArch64::Z11 },
  { 108U, AArch64::Z12 },
  { 109U, AArch64::Z13 },
  { 110U, AArch64::Z14 },
  { 111U, AArch64::Z15 },
  { 112U, AArch64::Z16 },
  { 113U, AArch64::Z17 },
  { 114U, AArch64::Z18 },
  { 115U, AArch64::Z19 },
  { 116U, AArch64::Z20 },
  { 117U, AArch64::Z21 },
  { 118U, AArch64::Z22 },
  { 119U, AArch64::Z23 },
  { 120U, AArch64::Z24 },
  { 121U, AArch64::Z25 },
  { 122U, AArch64::Z26 },
  { 123U, AArch64::Z27 },
  { 124U, AArch64::Z28 },
  { 125U, AArch64::Z29 },
  { 126U, AArch64::Z30 },
  { 127U, AArch64::Z31 },
};
extern const unsigned AArch64EHFlavour0Dwarf2LSize = array_lengthof(AArch64EHFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64DwarfFlavour0L2Dwarf[] = {
  { AArch64::FFR, 47U },
  { AArch64::FP, 29U },
  { AArch64::LR, 30U },
  { AArch64::SP, 31U },
  { AArch64::WSP, 31U },
  { AArch64::WZR, 31U },
  { AArch64::XZR, 31U },
  { AArch64::B0, 64U },
  { AArch64::B1, 65U },
  { AArch64::B2, 66U },
  { AArch64::B3, 67U },
  { AArch64::B4, 68U },
  { AArch64::B5, 69U },
  { AArch64::B6, 70U },
  { AArch64::B7, 71U },
  { AArch64::B8, 72U },
  { AArch64::B9, 73U },
  { AArch64::B10, 74U },
  { AArch64::B11, 75U },
  { AArch64::B12, 76U },
  { AArch64::B13, 77U },
  { AArch64::B14, 78U },
  { AArch64::B15, 79U },
  { AArch64::B16, 80U },
  { AArch64::B17, 81U },
  { AArch64::B18, 82U },
  { AArch64::B19, 83U },
  { AArch64::B20, 84U },
  { AArch64::B21, 85U },
  { AArch64::B22, 86U },
  { AArch64::B23, 87U },
  { AArch64::B24, 88U },
  { AArch64::B25, 89U },
  { AArch64::B26, 90U },
  { AArch64::B27, 91U },
  { AArch64::B28, 92U },
  { AArch64::B29, 93U },
  { AArch64::B30, 94U },
  { AArch64::B31, 95U },
  { AArch64::D0, 64U },
  { AArch64::D1, 65U },
  { AArch64::D2, 66U },
  { AArch64::D3, 67U },
  { AArch64::D4, 68U },
  { AArch64::D5, 69U },
  { AArch64::D6, 70U },
  { AArch64::D7, 71U },
  { AArch64::D8, 72U },
  { AArch64::D9, 73U },
  { AArch64::D10, 74U },
  { AArch64::D11, 75U },
  { AArch64::D12, 76U },
  { AArch64::D13, 77U },
  { AArch64::D14, 78U },
  { AArch64::D15, 79U },
  { AArch64::D16, 80U },
  { AArch64::D17, 81U },
  { AArch64::D18, 82U },
  { AArch64::D19, 83U },
  { AArch64::D20, 84U },
  { AArch64::D21, 85U },
  { AArch64::D22, 86U },
  { AArch64::D23, 87U },
  { AArch64::D24, 88U },
  { AArch64::D25, 89U },
  { AArch64::D26, 90U },
  { AArch64::D27, 91U },
  { AArch64::D28, 92U },
  { AArch64::D29, 93U },
  { AArch64::D30, 94U },
  { AArch64::D31, 95U },
  { AArch64::H0, 64U },
  { AArch64::H1, 65U },
  { AArch64::H2, 66U },
  { AArch64::H3, 67U },
  { AArch64::H4, 68U },
  { AArch64::H5, 69U },
  { AArch64::H6, 70U },
  { AArch64::H7, 71U },
  { AArch64::H8, 72U },
  { AArch64::H9, 73U },
  { AArch64::H10, 74U },
  { AArch64::H11, 75U },
  { AArch64::H12, 76U },
  { AArch64::H13, 77U },
  { AArch64::H14, 78U },
  { AArch64::H15, 79U },
  { AArch64::H16, 80U },
  { AArch64::H17, 81U },
  { AArch64::H18, 82U },
  { AArch64::H19, 83U },
  { AArch64::H20, 84U },
  { AArch64::H21, 85U },
  { AArch64::H22, 86U },
  { AArch64::H23, 87U },
  { AArch64::H24, 88U },
  { AArch64::H25, 89U },
  { AArch64::H26, 90U },
  { AArch64::H27, 91U },
  { AArch64::H28, 92U },
  { AArch64::H29, 93U },
  { AArch64::H30, 94U },
  { AArch64::H31, 95U },
  { AArch64::P0, 48U },
  { AArch64::P1, 49U },
  { AArch64::P2, 50U },
  { AArch64::P3, 51U },
  { AArch64::P4, 52U },
  { AArch64::P5, 53U },
  { AArch64::P6, 54U },
  { AArch64::P7, 55U },
  { AArch64::P8, 56U },
  { AArch64::P9, 57U },
  { AArch64::P10, 58U },
  { AArch64::P11, 59U },
  { AArch64::P12, 60U },
  { AArch64::P13, 61U },
  { AArch64::P14, 62U },
  { AArch64::P15, 63U },
  { AArch64::Q0, 64U },
  { AArch64::Q1, 65U },
  { AArch64::Q2, 66U },
  { AArch64::Q3, 67U },
  { AArch64::Q4, 68U },
  { AArch64::Q5, 69U },
  { AArch64::Q6, 70U },
  { AArch64::Q7, 71U },
  { AArch64::Q8, 72U },
  { AArch64::Q9, 73U },
  { AArch64::Q10, 74U },
  { AArch64::Q11, 75U },
  { AArch64::Q12, 76U },
  { AArch64::Q13, 77U },
  { AArch64::Q14, 78U },
  { AArch64::Q15, 79U },
  { AArch64::Q16, 80U },
  { AArch64::Q17, 81U },
  { AArch64::Q18, 82U },
  { AArch64::Q19, 83U },
  { AArch64::Q20, 84U },
  { AArch64::Q21, 85U },
  { AArch64::Q22, 86U },
  { AArch64::Q23, 87U },
  { AArch64::Q24, 88U },
  { AArch64::Q25, 89U },
  { AArch64::Q26, 90U },
  { AArch64::Q27, 91U },
  { AArch64::Q28, 92U },
  { AArch64::Q29, 93U },
  { AArch64::Q30, 94U },
  { AArch64::Q31, 95U },
  { AArch64::S0, 64U },
  { AArch64::S1, 65U },
  { AArch64::S2, 66U },
  { AArch64::S3, 67U },
  { AArch64::S4, 68U },
  { AArch64::S5, 69U },
  { AArch64::S6, 70U },
  { AArch64::S7, 71U },
  { AArch64::S8, 72U },
  { AArch64::S9, 73U },
  { AArch64::S10, 74U },
  { AArch64::S11, 75U },
  { AArch64::S12, 76U },
  { AArch64::S13, 77U },
  { AArch64::S14, 78U },
  { AArch64::S15, 79U },
  { AArch64::S16, 80U },
  { AArch64::S17, 81U },
  { AArch64::S18, 82U },
  { AArch64::S19, 83U },
  { AArch64::S20, 84U },
  { AArch64::S21, 85U },
  { AArch64::S22, 86U },
  { AArch64::S23, 87U },
  { AArch64::S24, 88U },
  { AArch64::S25, 89U },
  { AArch64::S26, 90U },
  { AArch64::S27, 91U },
  { AArch64::S28, 92U },
  { AArch64::S29, 93U },
  { AArch64::S30, 94U },
  { AArch64::S31, 95U },
  { AArch64::W0, 0U },
  { AArch64::W1, 1U },
  { AArch64::W2, 2U },
  { AArch64::W3, 3U },
  { AArch64::W4, 4U },
  { AArch64::W5, 5U },
  { AArch64::W6, 6U },
  { AArch64::W7, 7U },
  { AArch64::W8, 8U },
  { AArch64::W9, 9U },
  { AArch64::W10, 10U },
  { AArch64::W11, 11U },
  { AArch64::W12, 12U },
  { AArch64::W13, 13U },
  { AArch64::W14, 14U },
  { AArch64::W15, 15U },
  { AArch64::W16, 16U },
  { AArch64::W17, 17U },
  { AArch64::W18, 18U },
  { AArch64::W19, 19U },
  { AArch64::W20, 20U },
  { AArch64::W21, 21U },
  { AArch64::W22, 22U },
  { AArch64::W23, 23U },
  { AArch64::W24, 24U },
  { AArch64::W25, 25U },
  { AArch64::W26, 26U },
  { AArch64::W27, 27U },
  { AArch64::W28, 28U },
  { AArch64::W29, 29U },
  { AArch64::W30, 30U },
  { AArch64::X0, 0U },
  { AArch64::X1, 1U },
  { AArch64::X2, 2U },
  { AArch64::X3, 3U },
  { AArch64::X4, 4U },
  { AArch64::X5, 5U },
  { AArch64::X6, 6U },
  { AArch64::X7, 7U },
  { AArch64::X8, 8U },
  { AArch64::X9, 9U },
  { AArch64::X10, 10U },
  { AArch64::X11, 11U },
  { AArch64::X12, 12U },
  { AArch64::X13, 13U },
  { AArch64::X14, 14U },
  { AArch64::X15, 15U },
  { AArch64::X16, 16U },
  { AArch64::X17, 17U },
  { AArch64::X18, 18U },
  { AArch64::X19, 19U },
  { AArch64::X20, 20U },
  { AArch64::X21, 21U },
  { AArch64::X22, 22U },
  { AArch64::X23, 23U },
  { AArch64::X24, 24U },
  { AArch64::X25, 25U },
  { AArch64::X26, 26U },
  { AArch64::X27, 27U },
  { AArch64::X28, 28U },
  { AArch64::Z0, 96U },
  { AArch64::Z1, 97U },
  { AArch64::Z2, 98U },
  { AArch64::Z3, 99U },
  { AArch64::Z4, 100U },
  { AArch64::Z5, 101U },
  { AArch64::Z6, 102U },
  { AArch64::Z7, 103U },
  { AArch64::Z8, 104U },
  { AArch64::Z9, 105U },
  { AArch64::Z10, 106U },
  { AArch64::Z11, 107U },
  { AArch64::Z12, 108U },
  { AArch64::Z13, 109U },
  { AArch64::Z14, 110U },
  { AArch64::Z15, 111U },
  { AArch64::Z16, 112U },
  { AArch64::Z17, 113U },
  { AArch64::Z18, 114U },
  { AArch64::Z19, 115U },
  { AArch64::Z20, 116U },
  { AArch64::Z21, 117U },
  { AArch64::Z22, 118U },
  { AArch64::Z23, 119U },
  { AArch64::Z24, 120U },
  { AArch64::Z25, 121U },
  { AArch64::Z26, 122U },
  { AArch64::Z27, 123U },
  { AArch64::Z28, 124U },
  { AArch64::Z29, 125U },
  { AArch64::Z30, 126U },
  { AArch64::Z31, 127U },
};
extern const unsigned AArch64DwarfFlavour0L2DwarfSize = array_lengthof(AArch64DwarfFlavour0L2Dwarf);

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64EHFlavour0L2Dwarf[] = {
  { AArch64::FFR, 47U },
  { AArch64::FP, 29U },
  { AArch64::LR, 30U },
  { AArch64::SP, 31U },
  { AArch64::WSP, 31U },
  { AArch64::WZR, 31U },
  { AArch64::XZR, 31U },
  { AArch64::B0, 64U },
  { AArch64::B1, 65U },
  { AArch64::B2, 66U },
  { AArch64::B3, 67U },
  { AArch64::B4, 68U },
  { AArch64::B5, 69U },
  { AArch64::B6, 70U },
  { AArch64::B7, 71U },
  { AArch64::B8, 72U },
  { AArch64::B9, 73U },
  { AArch64::B10, 74U },
  { AArch64::B11, 75U },
  { AArch64::B12, 76U },
  { AArch64::B13, 77U },
  { AArch64::B14, 78U },
  { AArch64::B15, 79U },
  { AArch64::B16, 80U },
  { AArch64::B17, 81U },
  { AArch64::B18, 82U },
  { AArch64::B19, 83U },
  { AArch64::B20, 84U },
  { AArch64::B21, 85U },
  { AArch64::B22, 86U },
  { AArch64::B23, 87U },
  { AArch64::B24, 88U },
  { AArch64::B25, 89U },
  { AArch64::B26, 90U },
  { AArch64::B27, 91U },
  { AArch64::B28, 92U },
  { AArch64::B29, 93U },
  { AArch64::B30, 94U },
  { AArch64::B31, 95U },
  { AArch64::D0, 64U },
  { AArch64::D1, 65U },
  { AArch64::D2, 66U },
  { AArch64::D3, 67U },
  { AArch64::D4, 68U },
  { AArch64::D5, 69U },
  { AArch64::D6, 70U },
  { AArch64::D7, 71U },
  { AArch64::D8, 72U },
  { AArch64::D9, 73U },
  { AArch64::D10, 74U },
  { AArch64::D11, 75U },
  { AArch64::D12, 76U },
  { AArch64::D13, 77U },
  { AArch64::D14, 78U },
  { AArch64::D15, 79U },
  { AArch64::D16, 80U },
  { AArch64::D17, 81U },
  { AArch64::D18, 82U },
  { AArch64::D19, 83U },
  { AArch64::D20, 84U },
  { AArch64::D21, 85U },
  { AArch64::D22, 86U },
  { AArch64::D23, 87U },
  { AArch64::D24, 88U },
  { AArch64::D25, 89U },
  { AArch64::D26, 90U },
  { AArch64::D27, 91U },
  { AArch64::D28, 92U },
  { AArch64::D29, 93U },
  { AArch64::D30, 94U },
  { AArch64::D31, 95U },
  { AArch64::H0, 64U },
  { AArch64::H1, 65U },
  { AArch64::H2, 66U },
  { AArch64::H3, 67U },
  { AArch64::H4, 68U },
  { AArch64::H5, 69U },
  { AArch64::H6, 70U },
  { AArch64::H7, 71U },
  { AArch64::H8, 72U },
  { AArch64::H9, 73U },
  { AArch64::H10, 74U },
  { AArch64::H11, 75U },
  { AArch64::H12, 76U },
  { AArch64::H13, 77U },
  { AArch64::H14, 78U },
  { AArch64::H15, 79U },
  { AArch64::H16, 80U },
  { AArch64::H17, 81U },
  { AArch64::H18, 82U },
  { AArch64::H19, 83U },
  { AArch64::H20, 84U },
  { AArch64::H21, 85U },
  { AArch64::H22, 86U },
  { AArch64::H23, 87U },
  { AArch64::H24, 88U },
  { AArch64::H25, 89U },
  { AArch64::H26, 90U },
  { AArch64::H27, 91U },
  { AArch64::H28, 92U },
  { AArch64::H29, 93U },
  { AArch64::H30, 94U },
  { AArch64::H31, 95U },
  { AArch64::P0, 48U },
  { AArch64::P1, 49U },
  { AArch64::P2, 50U },
  { AArch64::P3, 51U },
  { AArch64::P4, 52U },
  { AArch64::P5, 53U },
  { AArch64::P6, 54U },
  { AArch64::P7, 55U },
  { AArch64::P8, 56U },
  { AArch64::P9, 57U },
  { AArch64::P10, 58U },
  { AArch64::P11, 59U },
  { AArch64::P12, 60U },
  { AArch64::P13, 61U },
  { AArch64::P14, 62U },
  { AArch64::P15, 63U },
  { AArch64::Q0, 64U },
  { AArch64::Q1, 65U },
  { AArch64::Q2, 66U },
  { AArch64::Q3, 67U },
  { AArch64::Q4, 68U },
  { AArch64::Q5, 69U },
  { AArch64::Q6, 70U },
  { AArch64::Q7, 71U },
  { AArch64::Q8, 72U },
  { AArch64::Q9, 73U },
  { AArch64::Q10, 74U },
  { AArch64::Q11, 75U },
  { AArch64::Q12, 76U },
  { AArch64::Q13, 77U },
  { AArch64::Q14, 78U },
  { AArch64::Q15, 79U },
  { AArch64::Q16, 80U },
  { AArch64::Q17, 81U },
  { AArch64::Q18, 82U },
  { AArch64::Q19, 83U },
  { AArch64::Q20, 84U },
  { AArch64::Q21, 85U },
  { AArch64::Q22, 86U },
  { AArch64::Q23, 87U },
  { AArch64::Q24, 88U },
  { AArch64::Q25, 89U },
  { AArch64::Q26, 90U },
  { AArch64::Q27, 91U },
  { AArch64::Q28, 92U },
  { AArch64::Q29, 93U },
  { AArch64::Q30, 94U },
  { AArch64::Q31, 95U },
  { AArch64::S0, 64U },
  { AArch64::S1, 65U },
  { AArch64::S2, 66U },
  { AArch64::S3, 67U },
  { AArch64::S4, 68U },
  { AArch64::S5, 69U },
  { AArch64::S6, 70U },
  { AArch64::S7, 71U },
  { AArch64::S8, 72U },
  { AArch64::S9, 73U },
  { AArch64::S10, 74U },
  { AArch64::S11, 75U },
  { AArch64::S12, 76U },
  { AArch64::S13, 77U },
  { AArch64::S14, 78U },
  { AArch64::S15, 79U },
  { AArch64::S16, 80U },
  { AArch64::S17, 81U },
  { AArch64::S18, 82U },
  { AArch64::S19, 83U },
  { AArch64::S20, 84U },
  { AArch64::S21, 85U },
  { AArch64::S22, 86U },
  { AArch64::S23, 87U },
  { AArch64::S24, 88U },
  { AArch64::S25, 89U },
  { AArch64::S26, 90U },
  { AArch64::S27, 91U },
  { AArch64::S28, 92U },
  { AArch64::S29, 93U },
  { AArch64::S30, 94U },
  { AArch64::S31, 95U },
  { AArch64::W0, 0U },
  { AArch64::W1, 1U },
  { AArch64::W2, 2U },
  { AArch64::W3, 3U },
  { AArch64::W4, 4U },
  { AArch64::W5, 5U },
  { AArch64::W6, 6U },
  { AArch64::W7, 7U },
  { AArch64::W8, 8U },
  { AArch64::W9, 9U },
  { AArch64::W10, 10U },
  { AArch64::W11, 11U },
  { AArch64::W12, 12U },
  { AArch64::W13, 13U },
  { AArch64::W14, 14U },
  { AArch64::W15, 15U },
  { AArch64::W16, 16U },
  { AArch64::W17, 17U },
  { AArch64::W18, 18U },
  { AArch64::W19, 19U },
  { AArch64::W20, 20U },
  { AArch64::W21, 21U },
  { AArch64::W22, 22U },
  { AArch64::W23, 23U },
  { AArch64::W24, 24U },
  { AArch64::W25, 25U },
  { AArch64::W26, 26U },
  { AArch64::W27, 27U },
  { AArch64::W28, 28U },
  { AArch64::W29, 29U },
  { AArch64::W30, 30U },
  { AArch64::X0, 0U },
  { AArch64::X1, 1U },
  { AArch64::X2, 2U },
  { AArch64::X3, 3U },
  { AArch64::X4, 4U },
  { AArch64::X5, 5U },
  { AArch64::X6, 6U },
  { AArch64::X7, 7U },
  { AArch64::X8, 8U },
  { AArch64::X9, 9U },
  { AArch64::X10, 10U },
  { AArch64::X11, 11U },
  { AArch64::X12, 12U },
  { AArch64::X13, 13U },
  { AArch64::X14, 14U },
  { AArch64::X15, 15U },
  { AArch64::X16, 16U },
  { AArch64::X17, 17U },
  { AArch64::X18, 18U },
  { AArch64::X19, 19U },
  { AArch64::X20, 20U },
  { AArch64::X21, 21U },
  { AArch64::X22, 22U },
  { AArch64::X23, 23U },
  { AArch64::X24, 24U },
  { AArch64::X25, 25U },
  { AArch64::X26, 26U },
  { AArch64::X27, 27U },
  { AArch64::X28, 28U },
  { AArch64::Z0, 96U },
  { AArch64::Z1, 97U },
  { AArch64::Z2, 98U },
  { AArch64::Z3, 99U },
  { AArch64::Z4, 100U },
  { AArch64::Z5, 101U },
  { AArch64::Z6, 102U },
  { AArch64::Z7, 103U },
  { AArch64::Z8, 104U },
  { AArch64::Z9, 105U },
  { AArch64::Z10, 106U },
  { AArch64::Z11, 107U },
  { AArch64::Z12, 108U },
  { AArch64::Z13, 109U },
  { AArch64::Z14, 110U },
  { AArch64::Z15, 111U },
  { AArch64::Z16, 112U },
  { AArch64::Z17, 113U },
  { AArch64::Z18, 114U },
  { AArch64::Z19, 115U },
  { AArch64::Z20, 116U },
  { AArch64::Z21, 117U },
  { AArch64::Z22, 118U },
  { AArch64::Z23, 119U },
  { AArch64::Z24, 120U },
  { AArch64::Z25, 121U },
  { AArch64::Z26, 122U },
  { AArch64::Z27, 123U },
  { AArch64::Z28, 124U },
  { AArch64::Z29, 125U },
  { AArch64::Z30, 126U },
  { AArch64::Z31, 127U },
};
extern const unsigned AArch64EHFlavour0L2DwarfSize = array_lengthof(AArch64EHFlavour0L2Dwarf);

extern const uint16_t AArch64RegEncodingTable[] = {
  0,
  0,
  29,
  30,
  0,
  31,
  31,
  31,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  30,
  0,
  2,
  4,
  6,
  8,
  10,
  12,
  14,
  16,
  18,
  20,
  22,
  24,
  26,
  28,
  30,
  28,
  0,
  2,
  4,
  6,
  8,
  10,
  12,
  14,
  16,
  18,
  20,
  22,
  24,
  26,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
};
static inline void InitAArch64MCRegisterInfo(MCRegisterInfo *RI, unsigned RA, unsigned DwarfFlavour = 0, unsigned EHFlavour = 0, unsigned PC = 0) {
  RI->InitMCRegisterInfo(AArch64RegDesc, 629, RA, PC, AArch64MCRegisterClasses, 108, AArch64RegUnitRoots, 115, AArch64RegDiffLists, AArch64LaneMaskLists, AArch64RegStrings, AArch64RegClassStrings, AArch64SubRegIdxLists, 100,
AArch64SubRegIdxRanges, AArch64RegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(AArch64DwarfFlavour0Dwarf2L, AArch64DwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(AArch64EHFlavour0Dwarf2L, AArch64EHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(AArch64DwarfFlavour0L2Dwarf, AArch64DwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(AArch64EHFlavour0L2Dwarf, AArch64EHFlavour0L2DwarfSize, true);
    break;
  }
}

} // end namespace llvm

#endif // GET_REGINFO_MC_DESC

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Register Information Header Fragment                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_HEADER
#undef GET_REGINFO_HEADER

#include "llvm/CodeGen/TargetRegisterInfo.h"

namespace llvm {

class AArch64FrameLowering;

struct AArch64GenRegisterInfo : public TargetRegisterInfo {
  explicit AArch64GenRegisterInfo(unsigned RA, unsigned D = 0, unsigned E = 0,
      unsigned PC = 0, unsigned HwMode = 0);
  unsigned composeSubRegIndicesImpl(unsigned, unsigned) const override;
  LaneBitmask composeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  LaneBitmask reverseComposeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  const TargetRegisterClass *getSubClassWithSubReg(const TargetRegisterClass*, unsigned) const override;
  const RegClassWeight &getRegClassWeight(const TargetRegisterClass *RC) const override;
  unsigned getRegUnitWeight(unsigned RegUnit) const override;
  unsigned getNumRegPressureSets() const override;
  const char *getRegPressureSetName(unsigned Idx) const override;
  unsigned getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const override;
  const int *getRegClassPressureSets(const TargetRegisterClass *RC) const override;
  const int *getRegUnitPressureSets(unsigned RegUnit) const override;
  ArrayRef<const char *> getRegMaskNames() const override;
  ArrayRef<const uint32_t *> getRegMasks() const override;
  /// Devirtualized TargetFrameLowering.
  static const AArch64FrameLowering *getFrameLowering(
      const MachineFunction &MF);
};

namespace AArch64 { // Register classes
  extern const TargetRegisterClass FPR8RegClass;
  extern const TargetRegisterClass FPR16RegClass;
  extern const TargetRegisterClass PPRRegClass;
  extern const TargetRegisterClass PPR_3bRegClass;
  extern const TargetRegisterClass GPR32allRegClass;
  extern const TargetRegisterClass FPR32RegClass;
  extern const TargetRegisterClass GPR32RegClass;
  extern const TargetRegisterClass GPR32spRegClass;
  extern const TargetRegisterClass GPR32commonRegClass;
  extern const TargetRegisterClass GPR32argRegClass;
  extern const TargetRegisterClass CCRRegClass;
  extern const TargetRegisterClass GPR32sponlyRegClass;
  extern const TargetRegisterClass WSeqPairsClassRegClass;
  extern const TargetRegisterClass WSeqPairsClass_with_subo32_in_GPR32commonRegClass;
  extern const TargetRegisterClass WSeqPairsClass_with_sube32_in_GPR32argRegClass;
  extern const TargetRegisterClass GPR64allRegClass;
  extern const TargetRegisterClass FPR64RegClass;
  extern const TargetRegisterClass GPR64RegClass;
  extern const TargetRegisterClass GPR64spRegClass;
  extern const TargetRegisterClass GPR64commonRegClass;
  extern const TargetRegisterClass GPR64noipRegClass;
  extern const TargetRegisterClass GPR64common_and_GPR64noipRegClass;
  extern const TargetRegisterClass tcGPR64RegClass;
  extern const TargetRegisterClass GPR64noip_and_tcGPR64RegClass;
  extern const TargetRegisterClass GPR64argRegClass;
  extern const TargetRegisterClass rtcGPR64RegClass;
  extern const TargetRegisterClass GPR64sponlyRegClass;
  extern const TargetRegisterClass DDRegClass;
  extern const TargetRegisterClass XSeqPairsClassRegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64commonRegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64noipRegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_GPR64noipRegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_tcGPR64RegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_tcGPR64RegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_sub_32_in_GPR32argRegClass;
  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_rtcGPR64RegClass;
  extern const TargetRegisterClass FPR128RegClass;
  extern const TargetRegisterClass ZPRRegClass;
  extern const TargetRegisterClass FPR128_loRegClass;
  extern const TargetRegisterClass ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR_3bRegClass;
  extern const TargetRegisterClass DDDRegClass;
  extern const TargetRegisterClass DDDDRegClass;
  extern const TargetRegisterClass QQRegClass;
  extern const TargetRegisterClass ZPR2RegClass;
  extern const TargetRegisterClass QQ_with_qsub0_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub0_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass QQQRegClass;
  extern const TargetRegisterClass ZPR3RegClass;
  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub0_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass QQQQRegClass;
  extern const TargetRegisterClass ZPR4RegClass;
  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub3_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub3_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass;
  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub0_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub3_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClass;
  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClass;
} // end namespace AArch64

} // end namespace llvm

#endif // GET_REGINFO_HEADER

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Target Register and Register Classes Information                           *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_TARGET_DESC
#undef GET_REGINFO_TARGET_DESC

namespace llvm {

extern const MCRegisterClass AArch64MCRegisterClasses[];

static const MVT::SimpleValueType VTLists[] = {
  /* 0 */ MVT::f32, MVT::i32, MVT::Other,
  /* 3 */ MVT::i64, MVT::Other,
  /* 5 */ MVT::f16, MVT::Other,
  /* 7 */ MVT::f64, MVT::i64, MVT::v2f32, MVT::v1f64, MVT::v8i8, MVT::v4i16, MVT::v2i32, MVT::v1i64, MVT::v4f16, MVT::Other,
  /* 17 */ MVT::v16i8, MVT::v8i16, MVT::v4i32, MVT::v2i64, MVT::v4f32, MVT::v2f64, MVT::f128, MVT::v8f16, MVT::Other,
  /* 26 */ MVT::v16i8, MVT::v8i16, MVT::v4i32, MVT::v2i64, MVT::v4f32, MVT::v2f64, MVT::v8f16, MVT::Other,
  /* 34 */ MVT::nxv16i1, MVT::nxv8i1, MVT::nxv4i1, MVT::nxv2i1, MVT::Other,
  /* 39 */ MVT::nxv16i8, MVT::nxv8i16, MVT::nxv4i32, MVT::nxv2i64, MVT::nxv2f16, MVT::nxv4f16, MVT::nxv8f16, MVT::nxv2f32, MVT::nxv4f32, MVT::nxv2f64, MVT::Other,
  /* 50 */ MVT::Untyped, MVT::Other,
};

static const char *const SubRegIndexNameTable[] = { "bsub", "dsub", "dsub0", "dsub1", "dsub2", "dsub3", "hsub", "qhisub", "qsub", "qsub0", "qsub1", "qsub2", "qsub3", "ssub", "sub_32", "sube32", "sube64", "subo32", "subo64", "zsub", "zsub0", "zsub1", "zsub2", "zsub3", "zsub_hi", "dsub1_then_bsub", "dsub1_then_hsub", "dsub1_then_ssub", "dsub3_then_bsub", "dsub3_then_hsub", "dsub3_then_ssub", "dsub2_then_bsub", "dsub2_then_hsub", "dsub2_then_ssub", "qsub1_then_bsub", "qsub1_then_dsub", "qsub1_then_hsub", "qsub1_then_ssub", "qsub3_then_bsub", "qsub3_then_dsub", "qsub3_then_hsub", "qsub3_then_ssub", "qsub2_then_bsub", "qsub2_then_dsub", "qsub2_then_hsub", "qsub2_then_ssub", "subo64_then_sub_32", "zsub1_then_bsub", "zsub1_then_dsub", "zsub1_then_hsub", "zsub1_then_ssub", "zsub1_then_zsub", "zsub1_then_zsub_hi", "zsub3_then_bsub", "zsub3_then_dsub", "zsub3_then_hsub", "zsub3_then_ssub", "zsub3_then_zsub", "zsub3_then_zsub_hi", "zsub2_then_bsub", "zsub2_then_dsub", "zsub2_then_hsub", "zsub2_then_ssub", "zsub2_then_zsub", "zsub2_then_zsub_hi", "dsub0_dsub1", "dsub0_dsub1_dsub2", "dsub1_dsub2", "dsub1_dsub2_dsub3", "dsub2_dsub3", "dsub_qsub1_then_dsub", "dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub", "dsub_qsub1_then_dsub_qsub2_then_dsub", "qsub0_qsub1", "qsub0_qsub1_qsub2", "qsub1_qsub2", "qsub1_qsub2_qsub3", "qsub2_qsub3", "qsub1_then_dsub_qsub2_then_dsub", "qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub", "qsub2_then_dsub_qsub3_then_dsub", "sub_32_subo64_then_sub_32", "dsub_zsub1_then_dsub", "zsub_zsub1_then_zsub", "dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub", "dsub_zsub1_then_dsub_zsub2_then_dsub", "zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub", "zsub_zsub1_then_zsub_zsub2_then_zsub", "zsub0_zsub1", "zsub0_zsub1_zsub2", "zsub1_zsub2", "zsub1_zsub2_zsub3", "zsub2_zsub3", "zsub1_then_dsub_zsub2_then_dsub", "zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub", "zsub1_then_zsub_zsub2_then_zsub", "zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub", "zsub2_then_dsub_zsub3_then_dsub", "zsub2_then_zsub_zsub3_then_zsub", "" };


static const LaneBitmask SubRegIndexLaneMaskTable[] = {
  LaneBitmask::getAll(),
  LaneBitmask(0x00000001), // bsub
  LaneBitmask(0x00000001), // dsub
  LaneBitmask(0x00000001), // dsub0
  LaneBitmask(0x00000080), // dsub1
  LaneBitmask(0x00000200), // dsub2
  LaneBitmask(0x00000100), // dsub3
  LaneBitmask(0x00000001), // hsub
  LaneBitmask(0x00000002), // qhisub
  LaneBitmask(0x00000004), // qsub
  LaneBitmask(0x00000001), // qsub0
  LaneBitmask(0x00000400), // qsub1
  LaneBitmask(0x00001000), // qsub2
  LaneBitmask(0x00000800), // qsub3
  LaneBitmask(0x00000001), // ssub
  LaneBitmask(0x00000008), // sub_32
  LaneBitmask(0x00000010), // sube32
  LaneBitmask(0x00000008), // sube64
  LaneBitmask(0x00000020), // subo32
  LaneBitmask(0x00002000), // subo64
  LaneBitmask(0x00000001), // zsub
  LaneBitmask(0x00000041), // zsub0
  LaneBitmask(0x0000C000), // zsub1
  LaneBitmask(0x000C0000), // zsub2
  LaneBitmask(0x00030000), // zsub3
  LaneBitmask(0x00000040), // zsub_hi
  LaneBitmask(0x00000080), // dsub1_then_bsub
  LaneBitmask(0x00000080), // dsub1_then_hsub
  LaneBitmask(0x00000080), // dsub1_then_ssub
  LaneBitmask(0x00000100), // dsub3_then_bsub
  LaneBitmask(0x00000100), // dsub3_then_hsub
  LaneBitmask(0x00000100), // dsub3_then_ssub
  LaneBitmask(0x00000200), // dsub2_then_bsub
  LaneBitmask(0x00000200), // dsub2_then_hsub
  LaneBitmask(0x00000200), // dsub2_then_ssub
  LaneBitmask(0x00000400), // qsub1_then_bsub
  LaneBitmask(0x00000400), // qsub1_then_dsub
  LaneBitmask(0x00000400), // qsub1_then_hsub
  LaneBitmask(0x00000400), // qsub1_then_ssub
  LaneBitmask(0x00000800), // qsub3_then_bsub
  LaneBitmask(0x00000800), // qsub3_then_dsub
  LaneBitmask(0x00000800), // qsub3_then_hsub
  LaneBitmask(0x00000800), // qsub3_then_ssub
  LaneBitmask(0x00001000), // qsub2_then_bsub
  LaneBitmask(0x00001000), // qsub2_then_dsub
  LaneBitmask(0x00001000), // qsub2_then_hsub
  LaneBitmask(0x00001000), // qsub2_then_ssub
  LaneBitmask(0x00002000), // subo64_then_sub_32
  LaneBitmask(0x00004000), // zsub1_then_bsub
  LaneBitmask(0x00004000), // zsub1_then_dsub
  LaneBitmask(0x00004000), // zsub1_then_hsub
  LaneBitmask(0x00004000), // zsub1_then_ssub
  LaneBitmask(0x00004000), // zsub1_then_zsub
  LaneBitmask(0x00008000), // zsub1_then_zsub_hi
  LaneBitmask(0x00010000), // zsub3_then_bsub
  LaneBitmask(0x00010000), // zsub3_then_dsub
  LaneBitmask(0x00010000), // zsub3_then_hsub
  LaneBitmask(0x00010000), // zsub3_then_ssub
  LaneBitmask(0x00010000), // zsub3_then_zsub
  LaneBitmask(0x00020000), // zsub3_then_zsub_hi
  LaneBitmask(0x00040000), // zsub2_then_bsub
  LaneBitmask(0x00040000), // zsub2_then_dsub
  LaneBitmask(0x00040000), // zsub2_then_hsub
  LaneBitmask(0x00040000), // zsub2_then_ssub
  LaneBitmask(0x00040000), // zsub2_then_zsub
  LaneBitmask(0x00080000), // zsub2_then_zsub_hi
  LaneBitmask(0x00000081), // dsub0_dsub1
  LaneBitmask(0x00000281), // dsub0_dsub1_dsub2
  LaneBitmask(0x00000280), // dsub1_dsub2
  LaneBitmask(0x00000380), // dsub1_dsub2_dsub3
  LaneBitmask(0x00000300), // dsub2_dsub3
  LaneBitmask(0x00000401), // dsub_qsub1_then_dsub
  LaneBitmask(0x00001C01), // dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  LaneBitmask(0x00001401), // dsub_qsub1_then_dsub_qsub2_then_dsub
  LaneBitmask(0x00000401), // qsub0_qsub1
  LaneBitmask(0x00001401), // qsub0_qsub1_qsub2
  LaneBitmask(0x00001400), // qsub1_qsub2
  LaneBitmask(0x00001C00), // qsub1_qsub2_qsub3
  LaneBitmask(0x00001800), // qsub2_qsub3
  LaneBitmask(0x00001400), // qsub1_then_dsub_qsub2_then_dsub
  LaneBitmask(0x00001C00), // qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  LaneBitmask(0x00001800), // qsub2_then_dsub_qsub3_then_dsub
  LaneBitmask(0x00002008), // sub_32_subo64_then_sub_32
  LaneBitmask(0x00004001), // dsub_zsub1_then_dsub
  LaneBitmask(0x00004001), // zsub_zsub1_then_zsub
  LaneBitmask(0x00054001), // dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
  LaneBitmask(0x00044001), // dsub_zsub1_then_dsub_zsub2_then_dsub
  LaneBitmask(0x00054001), // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
  LaneBitmask(0x00044001), // zsub_zsub1_then_zsub_zsub2_then_zsub
  LaneBitmask(0x0000C041), // zsub0_zsub1
  LaneBitmask(0x000CC041), // zsub0_zsub1_zsub2
  LaneBitmask(0x000CC000), // zsub1_zsub2
  LaneBitmask(0x000FC000), // zsub1_zsub2_zsub3
  LaneBitmask(0x000F0000), // zsub2_zsub3
  LaneBitmask(0x00044000), // zsub1_then_dsub_zsub2_then_dsub
  LaneBitmask(0x00054000), // zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
  LaneBitmask(0x00044000), // zsub1_then_zsub_zsub2_then_zsub
  LaneBitmask(0x00054000), // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
  LaneBitmask(0x00050000), // zsub2_then_dsub_zsub3_then_dsub
  LaneBitmask(0x00050000), // zsub2_then_zsub_zsub3_then_zsub
 };



static const TargetRegisterInfo::RegClassInfo RegClassInfos[] = {
  // Mode = 0 (Default)
  { 8, 8, 8, VTLists+50 },    // FPR8
  { 16, 16, 16, VTLists+5 },    // FPR16
  { 16, 16, 16, VTLists+34 },    // PPR
  { 16, 16, 16, VTLists+34 },    // PPR_3b
  { 32, 32, 32, VTLists+1 },    // GPR32all
  { 32, 32, 32, VTLists+0 },    // FPR32
  { 32, 32, 32, VTLists+1 },    // GPR32
  { 32, 32, 32, VTLists+1 },    // GPR32sp
  { 32, 32, 32, VTLists+1 },    // GPR32common
  { 32, 32, 32, VTLists+1 },    // GPR32arg
  { 32, 32, 32, VTLists+1 },    // CCR
  { 32, 32, 32, VTLists+1 },    // GPR32sponly
  { 64, 64, 32, VTLists+50 },    // WSeqPairsClass
  { 64, 64, 32, VTLists+50 },    // WSeqPairsClass_with_subo32_in_GPR32common
  { 64, 64, 32, VTLists+50 },    // WSeqPairsClass_with_sube32_in_GPR32arg
  { 64, 64, 64, VTLists+3 },    // GPR64all
  { 64, 64, 64, VTLists+7 },    // FPR64
  { 64, 64, 64, VTLists+3 },    // GPR64
  { 64, 64, 64, VTLists+3 },    // GPR64sp
  { 64, 64, 64, VTLists+3 },    // GPR64common
  { 64, 64, 64, VTLists+3 },    // GPR64noip
  { 64, 64, 64, VTLists+3 },    // GPR64common_and_GPR64noip
  { 64, 64, 64, VTLists+3 },    // tcGPR64
  { 64, 64, 64, VTLists+3 },    // GPR64noip_and_tcGPR64
  { 64, 64, 64, VTLists+3 },    // GPR64arg
  { 64, 64, 64, VTLists+3 },    // rtcGPR64
  { 64, 64, 64, VTLists+3 },    // GPR64sponly
  { 128, 128, 64, VTLists+50 },    // DD
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_subo64_in_GPR64common
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_subo64_in_GPR64noip
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_sube64_in_GPR64noip
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_sube64_in_tcGPR64
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_subo64_in_tcGPR64
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_sub_32_in_GPR32arg
  { 128, 128, 64, VTLists+50 },    // XSeqPairsClass_with_sube64_in_rtcGPR64
  { 128, 128, 128, VTLists+17 },    // FPR128
  { 128, 128, 128, VTLists+39 },    // ZPR
  { 128, 128, 128, VTLists+26 },    // FPR128_lo
  { 128, 128, 128, VTLists+39 },    // ZPR_4b
  { 128, 128, 128, VTLists+39 },    // ZPR_3b
  { 192, 192, 64, VTLists+50 },    // DDD
  { 256, 256, 64, VTLists+50 },    // DDDD
  { 256, 256, 128, VTLists+50 },    // QQ
  { 256, 256, 128, VTLists+50 },    // ZPR2
  { 256, 256, 128, VTLists+50 },    // QQ_with_qsub0_in_FPR128_lo
  { 256, 256, 128, VTLists+50 },    // QQ_with_qsub1_in_FPR128_lo
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub1_in_ZPR_4b
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub_in_FPR128_lo
  { 256, 256, 128, VTLists+50 },    // QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub0_in_ZPR_3b
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub1_in_ZPR_3b
  { 256, 256, 128, VTLists+50 },    // ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // QQQ
  { 384, 384, 128, VTLists+50 },    // ZPR3
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub0_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub1_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub2_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub1_in_ZPR_4b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub2_in_ZPR_4b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
  { 384, 384, 128, VTLists+50 },    // QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub0_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub1_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub2_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
  { 384, 384, 128, VTLists+50 },    // ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // QQQQ
  { 512, 512, 128, VTLists+50 },    // ZPR4
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub0_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub1_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub2_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub3_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub2_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub3_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub0_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub2_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub3_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
  { 512, 512, 128, VTLists+50 },    // ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
};

static const TargetRegisterClass *const NullRegClasses[] = { nullptr };

static const uint32_t FPR8SubClassMask[] = {
  0x00000001, 0x00000000, 0x00000000, 0x00000000, 
  0x08010022, 0xffffffc0, 0xffffffff, 0x00000fff, // bsub
  0x08000000, 0x00001800, 0x00000000, 0x00000000, // dsub1_then_bsub
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub3_then_bsub
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub2_then_bsub
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub1_then_bsub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub3_then_bsub
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub2_then_bsub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1_then_bsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3_then_bsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2_then_bsub
};

static const uint32_t FPR16SubClassMask[] = {
  0x00000002, 0x00000000, 0x00000000, 0x00000000, 
  0x08010020, 0xffffffc0, 0xffffffff, 0x00000fff, // hsub
  0x08000000, 0x00001800, 0x00000000, 0x00000000, // dsub1_then_hsub
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub3_then_hsub
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub2_then_hsub
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub1_then_hsub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub3_then_hsub
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub2_then_hsub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1_then_hsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3_then_hsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2_then_hsub
};

static const uint32_t PPRSubClassMask[] = {
  0x0000000c, 0x00000000, 0x00000000, 0x00000000, 
};

static const uint32_t PPR_3bSubClassMask[] = {
  0x00000008, 0x00000000, 0x00000000, 0x00000000, 
};

static const uint32_t GPR32allSubClassMask[] = {
  0x00000bd0, 0x00000000, 0x00000000, 0x00000000, 
  0xf7fe8000, 0x0000003f, 0x00000000, 0x00000000, // sub_32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // sube32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // subo32
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64_then_sub_32
};

static const uint32_t FPR32SubClassMask[] = {
  0x00000020, 0x00000000, 0x00000000, 0x00000000, 
  0x08010000, 0xffffffc0, 0xffffffff, 0x00000fff, // ssub
  0x08000000, 0x00001800, 0x00000000, 0x00000000, // dsub1_then_ssub
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub3_then_ssub
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub2_then_ssub
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub1_then_ssub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub3_then_ssub
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub2_then_ssub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1_then_ssub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3_then_ssub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2_then_ssub
};

static const uint32_t GPR32SubClassMask[] = {
  0x00000340, 0x00000000, 0x00000000, 0x00000000, 
  0xf3fa0000, 0x0000003f, 0x00000000, 0x00000000, // sub_32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // sube32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // subo32
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64_then_sub_32
};

static const uint32_t GPR32spSubClassMask[] = {
  0x00000b80, 0x00000000, 0x00000000, 0x00000000, 
  0xf7ec0000, 0x0000003f, 0x00000000, 0x00000000, // sub_32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // sube32
  0x00006000, 0x00000000, 0x00000000, 0x00000000, // subo32
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64_then_sub_32
};

static const uint32_t GPR32commonSubClassMask[] = {
  0x00000300, 0x00000000, 0x00000000, 0x00000000, 
  0xf3e80000, 0x0000003f, 0x00000000, 0x00000000, // sub_32
  0x00007000, 0x00000000, 0x00000000, 0x00000000, // sube32
  0x00006000, 0x00000000, 0x00000000, 0x00000000, // subo32
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64_then_sub_32
};

static const uint32_t GPR32argSubClassMask[] = {
  0x00000200, 0x00000000, 0x00000000, 0x00000000, 
  0x01000000, 0x00000010, 0x00000000, 0x00000000, // sub_32
  0x00004000, 0x00000000, 0x00000000, 0x00000000, // sube32
  0x00004000, 0x00000000, 0x00000000, 0x00000000, // subo32
  0x00000000, 0x00000010, 0x00000000, 0x00000000, // subo64_then_sub_32
};

static const uint32_t CCRSubClassMask[] = {
  0x00000400, 0x00000000, 0x00000000, 0x00000000, 
};

static const uint32_t GPR32sponlySubClassMask[] = {
  0x00000800, 0x00000000, 0x00000000, 0x00000000, 
  0x04000000, 0x00000000, 0x00000000, 0x00000000, // sub_32
};

static const uint32_t WSeqPairsClassSubClassMask[] = {
  0x00007000, 0x00000000, 0x00000000, 0x00000000, 
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // sub_32_subo64_then_sub_32
};

static const uint32_t WSeqPairsClass_with_subo32_in_GPR32commonSubClassMask[] = {
  0x00006000, 0x00000000, 0x00000000, 0x00000000, 
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, // sub_32_subo64_then_sub_32
};

static const uint32_t WSeqPairsClass_with_sube32_in_GPR32argSubClassMask[] = {
  0x00004000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000010, 0x00000000, 0x00000000, // sub_32_subo64_then_sub_32
};

static const uint32_t GPR64allSubClassMask[] = {
  0x07fe8000, 0x00000000, 0x00000000, 0x00000000, 
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // sube64
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64
};

static const uint32_t FPR64SubClassMask[] = {
  0x00010000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0xffffe7c0, 0xffffffff, 0x00000fff, // dsub
  0x08000000, 0x00001800, 0x00000000, 0x00000000, // dsub0
  0x08000000, 0x00001800, 0x00000000, 0x00000000, // dsub1
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub2
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub3
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub1_then_dsub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub3_then_dsub
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub2_then_dsub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1_then_dsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3_then_dsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2_then_dsub
};

static const uint32_t GPR64SubClassMask[] = {
  0x03fa0000, 0x00000000, 0x00000000, 0x00000000, 
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // sube64
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64spSubClassMask[] = {
  0x07ec0000, 0x00000000, 0x00000000, 0x00000000, 
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // sube64
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64commonSubClassMask[] = {
  0x03e80000, 0x00000000, 0x00000000, 0x00000000, 
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, // sube64
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64noipSubClassMask[] = {
  0x01b00000, 0x00000000, 0x00000000, 0x00000000, 
  0x80000000, 0x0000001a, 0x00000000, 0x00000000, // sube64
  0xc0000000, 0x0000001a, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64common_and_GPR64noipSubClassMask[] = {
  0x01a00000, 0x00000000, 0x00000000, 0x00000000, 
  0x80000000, 0x0000001a, 0x00000000, 0x00000000, // sube64
  0x80000000, 0x0000001a, 0x00000000, 0x00000000, // subo64
};

static const uint32_t tcGPR64SubClassMask[] = {
  0x03c00000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x0000003f, 0x00000000, 0x00000000, // sube64
  0x00000000, 0x0000003c, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64noip_and_tcGPR64SubClassMask[] = {
  0x01800000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x0000001a, 0x00000000, 0x00000000, // sube64
  0x00000000, 0x00000018, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64argSubClassMask[] = {
  0x01000000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000010, 0x00000000, 0x00000000, // sube64
  0x00000000, 0x00000010, 0x00000000, 0x00000000, // subo64
};

static const uint32_t rtcGPR64SubClassMask[] = {
  0x02000000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000020, 0x00000000, 0x00000000, // sube64
  0x00000000, 0x00000020, 0x00000000, 0x00000000, // subo64
};

static const uint32_t GPR64sponlySubClassMask[] = {
  0x04000000, 0x00000000, 0x00000000, 0x00000000, 
};

static const uint32_t DDSubClassMask[] = {
  0x08000000, 0x00000000, 0x00000000, 0x00000000, 
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub0_dsub1
  0x00000000, 0x00001800, 0x00000000, 0x00000000, // dsub1_dsub2
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub2_dsub3
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // dsub_qsub1_then_dsub
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub1_then_dsub_qsub2_then_dsub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub2_then_dsub_qsub3_then_dsub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // dsub_zsub1_then_dsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub1_then_dsub_zsub2_then_dsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub2_then_dsub_zsub3_then_dsub
};

static const uint32_t XSeqPairsClassSubClassMask[] = {
  0xf0000000, 0x0000003f, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_subo64_in_GPR64commonSubClassMask[] = {
  0xa0000000, 0x0000003f, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_subo64_in_GPR64noipSubClassMask[] = {
  0xc0000000, 0x0000001a, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_sube64_in_GPR64noipSubClassMask[] = {
  0x80000000, 0x0000001a, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_sube64_in_tcGPR64SubClassMask[] = {
  0x00000000, 0x0000003f, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64SubClassMask[] = {
  0x00000000, 0x0000001a, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_subo64_in_tcGPR64SubClassMask[] = {
  0x00000000, 0x0000003c, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64SubClassMask[] = {
  0x00000000, 0x00000018, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_sub_32_in_GPR32argSubClassMask[] = {
  0x00000000, 0x00000010, 0x00000000, 0x00000000, 
};

static const uint32_t XSeqPairsClass_with_sube64_in_rtcGPR64SubClassMask[] = {
  0x00000000, 0x00000020, 0x00000000, 0x00000000, 
};

static const uint32_t FPR128SubClassMask[] = {
  0x00000000, 0x00000140, 0x00000000, 0x00000000, 
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub0
  0x00000000, 0x1d09a000, 0x31c3d013, 0x00000001, // qsub1
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub2
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub3
  0x00000000, 0xe2f64680, 0xce3c2fec, 0x00000ffe, // zsub
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1_then_zsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3_then_zsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2_then_zsub
};

static const uint32_t ZPRSubClassMask[] = {
  0x00000000, 0x00000680, 0x00000000, 0x00000000, 
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub0
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub1
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub2
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub3
};

static const uint32_t FPR128_loSubClassMask[] = {
  0x00000000, 0x00000100, 0x00000000, 0x00000000, 
  0x00000000, 0x04088000, 0x10404011, 0x00000001, // qsub0
  0x00000000, 0x08090000, 0x30c08013, 0x00000001, // qsub1
  0x00000000, 0x10000000, 0x31810012, 0x00000001, // qsub2
  0x00000000, 0x00000000, 0x21020000, 0x00000001, // qsub3
  0x00000000, 0x80b40600, 0x88200c68, 0x00000d06, // zsub
  0x00000000, 0x20f20000, 0xca040eec, 0x00000f4e, // zsub1_then_zsub
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub3_then_zsub
  0x00000000, 0x40000000, 0xc6080fe4, 0x00000fde, // zsub2_then_zsub
};

static const uint32_t ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000600, 0x00000000, 0x00000000, 
  0x00000000, 0x80b40000, 0x88200c68, 0x00000d06, // zsub0
  0x00000000, 0x20f20000, 0xca040eec, 0x00000f4e, // zsub1
  0x00000000, 0x40000000, 0xc6080fe4, 0x00000fde, // zsub2
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub3
};

static const uint32_t ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000400, 0x00000000, 0x00000000, 
  0x00000000, 0x00a00000, 0x00000c40, 0x00000d04, // zsub0
  0x00000000, 0x00c00000, 0x00000e80, 0x00000f48, // zsub1
  0x00000000, 0x00000000, 0x00000b00, 0x00000ed0, // zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000aa0, // zsub3
};

static const uint32_t DDDSubClassMask[] = {
  0x00000000, 0x00000800, 0x00000000, 0x00000000, 
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub0_dsub1_dsub2
  0x00000000, 0x00001000, 0x00000000, 0x00000000, // dsub1_dsub2_dsub3
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // dsub_qsub1_then_dsub_qsub2_then_dsub
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // dsub_zsub1_then_dsub_zsub2_then_dsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
};

static const uint32_t DDDDSubClassMask[] = {
  0x00000000, 0x00001000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
};

static const uint32_t QQSubClassMask[] = {
  0x00000000, 0x0009a000, 0x00000000, 0x00000000, 
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub0_qsub1
  0x00000000, 0x1d000000, 0x31c3d013, 0x00000001, // qsub1_qsub2
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub2_qsub3
  0x00000000, 0xe2f64000, 0xce3c2fec, 0x00000ffe, // zsub_zsub1_then_zsub
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR2SubClassMask[] = {
  0x00000000, 0x00f64000, 0x00000000, 0x00000000, 
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub0_zsub1
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub1_zsub2
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub2_zsub3
};

static const uint32_t QQ_with_qsub0_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00088000, 0x00000000, 0x00000000, 
  0x00000000, 0x04000000, 0x10404011, 0x00000001, // qsub0_qsub1
  0x00000000, 0x08000000, 0x30c08013, 0x00000001, // qsub1_qsub2
  0x00000000, 0x00000000, 0x31810000, 0x00000001, // qsub2_qsub3
  0x00000000, 0x80b40000, 0x88200c68, 0x00000d06, // zsub_zsub1_then_zsub
  0x00000000, 0x20000000, 0xca040eec, 0x00000f4e, // zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00090000, 0x00000000, 0x00000000, 
  0x00000000, 0x08000000, 0x30c08013, 0x00000001, // qsub0_qsub1
  0x00000000, 0x10000000, 0x31810012, 0x00000001, // qsub1_qsub2
  0x00000000, 0x00000000, 0x21020000, 0x00000001, // qsub2_qsub3
  0x00000000, 0x20f20000, 0xca040eec, 0x00000f4e, // zsub_zsub1_then_zsub
  0x00000000, 0x40000000, 0xc6080fe4, 0x00000fde, // zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR2_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00f20000, 0x00000000, 0x00000000, 
  0x00000000, 0x20000000, 0xca040eec, 0x00000f4e, // zsub0_zsub1
  0x00000000, 0x40000000, 0xc6080fe4, 0x00000fde, // zsub1_zsub2
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub2_zsub3
};

static const uint32_t ZPR2_with_zsub_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00b40000, 0x00000000, 0x00000000, 
  0x00000000, 0x80000000, 0x88200c68, 0x00000d06, // zsub0_zsub1
  0x00000000, 0x20000000, 0xca040eec, 0x00000f4e, // zsub1_zsub2
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub2_zsub3
};

static const uint32_t QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00080000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x10400011, 0x00000001, // qsub0_qsub1
  0x00000000, 0x00000000, 0x30800012, 0x00000001, // qsub1_qsub2
  0x00000000, 0x00000000, 0x21000000, 0x00000001, // qsub2_qsub3
  0x00000000, 0x00b00000, 0x88000c68, 0x00000d06, // zsub_zsub1_then_zsub
  0x00000000, 0x00000000, 0xc2000ee4, 0x00000f4e, // zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, // zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00b00000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x88000c68, 0x00000d06, // zsub0_zsub1
  0x00000000, 0x00000000, 0xc2000ee4, 0x00000f4e, // zsub1_zsub2
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, // zsub2_zsub3
};

static const uint32_t ZPR2_with_zsub0_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00a00000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x00000c40, 0x00000d04, // zsub0_zsub1
  0x00000000, 0x00000000, 0x00000e80, 0x00000f48, // zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000ed0, // zsub2_zsub3
};

static const uint32_t ZPR2_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00c00000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x00000e80, 0x00000f48, // zsub0_zsub1
  0x00000000, 0x00000000, 0x00000b00, 0x00000ed0, // zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000aa0, // zsub2_zsub3
};

static const uint32_t ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00800000, 0x00000000, 0x00000000, 
  0x00000000, 0x00000000, 0x00000c00, 0x00000d00, // zsub0_zsub1
  0x00000000, 0x00000000, 0x00000a00, 0x00000e40, // zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000a80, // zsub2_zsub3
};

static const uint32_t QQQSubClassMask[] = {
  0x00000000, 0x1d000000, 0x00000013, 0x00000000, 
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0xe2000000, 0xce3c2fec, 0x00000ffe, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR3SubClassMask[] = {
  0x00000000, 0xe2000000, 0x00000fec, 0x00000000, 
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub1_zsub2_zsub3
};

static const uint32_t QQQ_with_qsub0_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x04000000, 0x00000011, 0x00000000, 
  0x00000000, 0x00000000, 0x10404000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x30c08000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x80000000, 0x88200c68, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xca040000, 0x00000f4e, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x08000000, 0x00000013, 0x00000000, 
  0x00000000, 0x00000000, 0x30c08000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x31810000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x20000000, 0xca040eec, 0x00000f4e, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x10000000, 0x00000012, 0x00000000, 
  0x00000000, 0x00000000, 0x31810000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x21020000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x40000000, 0xc6080fe4, 0x00000fde, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR3_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x20000000, 0x00000eec, 0x00000000, 
  0x00000000, 0x00000000, 0xca040000, 0x00000f4e, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x40000000, 0x00000fe4, 0x00000000, 
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x80000000, 0x00000c68, 0x00000000, 
  0x00000000, 0x00000000, 0x88200000, 0x00000d06, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0xca040000, 0x00000f4e, // zsub1_zsub2_zsub3
};

static const uint32_t QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000011, 0x00000000, 
  0x00000000, 0x00000000, 0x10400000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x30800000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x00000000, 0x88000c68, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0xc2000000, 0x00000f4e, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000012, 0x00000000, 
  0x00000000, 0x00000000, 0x30800000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x21000000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x00000000, 0xc2000ee4, 0x00000f4e, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000ee4, 0x00000000, 
  0x00000000, 0x00000000, 0xc2000000, 0x00000f4e, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000c68, 0x00000000, 
  0x00000000, 0x00000000, 0x88000000, 0x00000d06, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0xc2000000, 0x00000f4e, // zsub1_zsub2_zsub3
};

static const uint32_t QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000010, 0x00000000, 
  0x00000000, 0x00000000, 0x10000000, 0x00000001, // qsub0_qsub1_qsub2
  0x00000000, 0x00000000, 0x20000000, 0x00000001, // qsub1_qsub2_qsub3
  0x00000000, 0x00000000, 0x80000c60, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub
  0x00000000, 0x00000000, 0x40000000, 0x00000f4e, // zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000c60, 0x00000000, 
  0x00000000, 0x00000000, 0x80000000, 0x00000d06, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x40000000, 0x00000f4e, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub0_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000c40, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000d04, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000f48, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000e80, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000f48, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000ed0, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000b00, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000ed0, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000aa0, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000a00, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000e40, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000a80, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000c00, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000d00, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000e40, // zsub1_zsub2_zsub3
};

static const uint32_t ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000800, 0x00000000, 
  0x00000000, 0x00000000, 0x00000000, 0x00000c00, // zsub0_zsub1_zsub2
  0x00000000, 0x00000000, 0x00000000, 0x00000a00, // zsub1_zsub2_zsub3
};

static const uint32_t QQQQSubClassMask[] = {
  0x00000000, 0x00000000, 0x31c3d000, 0x00000001, 
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR4SubClassMask[] = {
  0x00000000, 0x00000000, 0xce3c2000, 0x00000ffe, 
};

static const uint32_t QQQQ_with_qsub0_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x10404000, 0x00000001, 
  0x00000000, 0x00000000, 0x88200000, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x30c08000, 0x00000001, 
  0x00000000, 0x00000000, 0xca040000, 0x00000f4e, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x31810000, 0x00000001, 
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub3_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x21020000, 0x00000001, 
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0xca040000, 0x00000f4e, 
};

static const uint32_t ZPR4_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0xc6080000, 0x00000fde, 
};

static const uint32_t ZPR4_with_zsub3_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x44100000, 0x00000ffe, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x88200000, 0x00000d06, 
};

static const uint32_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x10400000, 0x00000001, 
  0x00000000, 0x00000000, 0x88000000, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x30800000, 0x00000001, 
  0x00000000, 0x00000000, 0xc2000000, 0x00000f4e, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x21000000, 0x00000001, 
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0xc2000000, 0x00000f4e, 
};

static const uint32_t ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x44000000, 0x00000fde, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x88000000, 0x00000d06, 
};

static const uint32_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x10000000, 0x00000001, 
  0x00000000, 0x00000000, 0x80000000, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x20000000, 0x00000001, 
  0x00000000, 0x00000000, 0x40000000, 0x00000f4e, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x40000000, 0x00000f4e, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x80000000, 0x00000d06, 
};

static const uint32_t QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000001, 
  0x00000000, 0x00000000, 0x00000000, 0x00000d06, // zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000d06, 
};

static const uint32_t ZPR4_with_zsub0_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000d04, 
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000f48, 
};

static const uint32_t ZPR4_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000ed0, 
};

static const uint32_t ZPR4_with_zsub3_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000aa0, 
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000e40, 
};

static const uint32_t ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000a80, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000d00, 
};

static const uint32_t ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000a00, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000c00, 
};

static const uint32_t ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000800, 
};

static const uint16_t SuperRegIdxSeqs[] = {
  /* 0 */ 15, 0,
  /* 2 */ 17, 19, 0,
  /* 5 */ 21, 22, 23, 24, 0,
  /* 10 */ 15, 16, 18, 47, 0,
  /* 15 */ 1, 26, 29, 32, 35, 39, 43, 48, 54, 60, 0,
  /* 26 */ 2, 3, 4, 5, 6, 36, 40, 44, 49, 55, 61, 0,
  /* 38 */ 7, 27, 30, 33, 37, 41, 45, 50, 56, 62, 0,
  /* 49 */ 14, 28, 31, 34, 38, 42, 46, 51, 57, 63, 0,
  /* 60 */ 10, 11, 12, 13, 20, 52, 58, 64, 0,
  /* 69 */ 82, 0,
  /* 71 */ 72, 85, 0,
  /* 74 */ 87, 0,
  /* 76 */ 90, 92, 0,
  /* 79 */ 89, 91, 93, 0,
  /* 83 */ 67, 69, 73, 80, 86, 95, 0,
  /* 90 */ 75, 77, 88, 97, 0,
  /* 95 */ 66, 68, 70, 71, 79, 81, 83, 94, 98, 0,
  /* 105 */ 74, 76, 78, 84, 96, 99, 0,
};

static const TargetRegisterClass *const PPR_3bSuperclasses[] = {
  &AArch64::PPRRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR32Superclasses[] = {
  &AArch64::GPR32allRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR32spSuperclasses[] = {
  &AArch64::GPR32allRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR32commonSuperclasses[] = {
  &AArch64::GPR32allRegClass,
  &AArch64::GPR32RegClass,
  &AArch64::GPR32spRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR32argSuperclasses[] = {
  &AArch64::GPR32allRegClass,
  &AArch64::GPR32RegClass,
  &AArch64::GPR32spRegClass,
  &AArch64::GPR32commonRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR32sponlySuperclasses[] = {
  &AArch64::GPR32allRegClass,
  &AArch64::GPR32spRegClass,
  nullptr
};

static const TargetRegisterClass *const WSeqPairsClass_with_subo32_in_GPR32commonSuperclasses[] = {
  &AArch64::WSeqPairsClassRegClass,
  nullptr
};

static const TargetRegisterClass *const WSeqPairsClass_with_sube32_in_GPR32argSuperclasses[] = {
  &AArch64::WSeqPairsClassRegClass,
  &AArch64::WSeqPairsClass_with_subo32_in_GPR32commonRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64Superclasses[] = {
  &AArch64::GPR64allRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64spSuperclasses[] = {
  &AArch64::GPR64allRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64commonSuperclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64noipSuperclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64common_and_GPR64noipSuperclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  &AArch64::GPR64commonRegClass,
  &AArch64::GPR64noipRegClass,
  nullptr
};

static const TargetRegisterClass *const tcGPR64Superclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  &AArch64::GPR64commonRegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64noip_and_tcGPR64Superclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  &AArch64::GPR64commonRegClass,
  &AArch64::GPR64noipRegClass,
  &AArch64::GPR64common_and_GPR64noipRegClass,
  &AArch64::tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64argSuperclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  &AArch64::GPR64commonRegClass,
  &AArch64::GPR64noipRegClass,
  &AArch64::GPR64common_and_GPR64noipRegClass,
  &AArch64::tcGPR64RegClass,
  &AArch64::GPR64noip_and_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const rtcGPR64Superclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64RegClass,
  &AArch64::GPR64spRegClass,
  &AArch64::GPR64commonRegClass,
  &AArch64::tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const GPR64sponlySuperclasses[] = {
  &AArch64::GPR64allRegClass,
  &AArch64::GPR64spRegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_subo64_in_GPR64commonSuperclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_subo64_in_GPR64noipSuperclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_sube64_in_GPR64noipSuperclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_sube64_in_tcGPR64Superclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64Superclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_subo64_in_tcGPR64Superclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64Superclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_sub_32_in_GPR32argSuperclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_GPR64noipRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const XSeqPairsClass_with_sube64_in_rtcGPR64Superclasses[] = {
  &AArch64::XSeqPairsClassRegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
  &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
  &AArch64::XSeqPairsClass_with_subo64_in_tcGPR64RegClass,
  nullptr
};

static const TargetRegisterClass *const FPR128_loSuperclasses[] = {
  &AArch64::FPR128RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR_4bSuperclasses[] = {
  &AArch64::ZPRRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR_3bSuperclasses[] = {
  &AArch64::ZPRRegClass,
  &AArch64::ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const QQ_with_qsub0_in_FPR128_loSuperclasses[] = {
  &AArch64::QQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub_in_FPR128_loSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  nullptr
};

static const TargetRegisterClass *const QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQRegClass,
  &AArch64::QQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQ_with_qsub1_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  &AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR2_with_zsub_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub0_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  &AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR2_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  &AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR2RegClass,
  &AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR2_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR2_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR2_with_zsub1_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub0_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub_in_FPR128_loSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  &AArch64::QQQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub1_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  &AArch64::QQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub2_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQRegClass,
  &AArch64::QQQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub0_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR3RegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR3_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub0_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub3_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub3_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_loSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub3_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub3_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses[] = {
  &AArch64::QQQQRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub3_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
  &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub0_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub3_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClass,
  nullptr
};

static const TargetRegisterClass *const ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses[] = {
  &AArch64::ZPR4RegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
  &AArch64::ZPR4_with_zsub0_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub3_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
  &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
  nullptr
};


static inline unsigned GPR32AltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR32GetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WZR, AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR32RegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR32AltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

static inline unsigned GPR32spAltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR32spGetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WSP, AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR32spRegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR32spAltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

static inline unsigned GPR32commonAltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR32commonGetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR32commonRegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR32commonAltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

static inline unsigned GPR64AltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR64GetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::XZR, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR64RegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR64AltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

static inline unsigned GPR64spAltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR64spGetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::SP, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR64spRegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR64spAltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

static inline unsigned GPR64commonAltOrderSelect(const MachineFunction &MF) { return 1; }

static ArrayRef<MCPhysReg> GPR64commonGetRawAllocationOrder(const MachineFunction &MF) {
  static const MCPhysReg AltOrder1[] = { AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7 };
  const MCRegisterClass &MCR = AArch64MCRegisterClasses[AArch64::GPR64commonRegClassID];
  const ArrayRef<MCPhysReg> Order[] = {
    makeArrayRef(MCR.begin(), MCR.getNumRegs()),
    makeArrayRef(AltOrder1)
  };
  const unsigned Select = GPR64commonAltOrderSelect(MF);
  assert(Select < 2);
  return Order[Select];
}

namespace AArch64 {   // Register class instances
  extern const TargetRegisterClass FPR8RegClass = {
    &AArch64MCRegisterClasses[FPR8RegClassID],
    FPR8SubClassMask,
    SuperRegIdxSeqs + 15,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR16RegClass = {
    &AArch64MCRegisterClasses[FPR16RegClassID],
    FPR16SubClassMask,
    SuperRegIdxSeqs + 38,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass PPRRegClass = {
    &AArch64MCRegisterClasses[PPRRegClassID],
    PPRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass PPR_3bRegClass = {
    &AArch64MCRegisterClasses[PPR_3bRegClassID],
    PPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    PPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR32allRegClass = {
    &AArch64MCRegisterClasses[GPR32allRegClassID],
    GPR32allSubClassMask,
    SuperRegIdxSeqs + 10,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR32RegClass = {
    &AArch64MCRegisterClasses[FPR32RegClassID],
    FPR32SubClassMask,
    SuperRegIdxSeqs + 49,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPR32RegClass = {
    &AArch64MCRegisterClasses[GPR32RegClassID],
    GPR32SubClassMask,
    SuperRegIdxSeqs + 10,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR32Superclasses,
    GPR32GetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR32spRegClass = {
    &AArch64MCRegisterClasses[GPR32spRegClassID],
    GPR32spSubClassMask,
    SuperRegIdxSeqs + 10,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR32spSuperclasses,
    GPR32spGetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR32commonRegClass = {
    &AArch64MCRegisterClasses[GPR32commonRegClassID],
    GPR32commonSubClassMask,
    SuperRegIdxSeqs + 10,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR32commonSuperclasses,
    GPR32commonGetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR32argRegClass = {
    &AArch64MCRegisterClasses[GPR32argRegClassID],
    GPR32argSubClassMask,
    SuperRegIdxSeqs + 10,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR32argSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass CCRRegClass = {
    &AArch64MCRegisterClasses[CCRRegClassID],
    CCRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPR32sponlyRegClass = {
    &AArch64MCRegisterClasses[GPR32sponlyRegClassID],
    GPR32sponlySubClassMask,
    SuperRegIdxSeqs + 0,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR32sponlySuperclasses,
    nullptr
  };

  extern const TargetRegisterClass WSeqPairsClassRegClass = {
    &AArch64MCRegisterClasses[WSeqPairsClassRegClassID],
    WSeqPairsClassSubClassMask,
    SuperRegIdxSeqs + 69,
    LaneBitmask(0x00000030),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass WSeqPairsClass_with_subo32_in_GPR32commonRegClass = {
    &AArch64MCRegisterClasses[WSeqPairsClass_with_subo32_in_GPR32commonRegClassID],
    WSeqPairsClass_with_subo32_in_GPR32commonSubClassMask,
    SuperRegIdxSeqs + 69,
    LaneBitmask(0x00000030),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    WSeqPairsClass_with_subo32_in_GPR32commonSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass WSeqPairsClass_with_sube32_in_GPR32argRegClass = {
    &AArch64MCRegisterClasses[WSeqPairsClass_with_sube32_in_GPR32argRegClassID],
    WSeqPairsClass_with_sube32_in_GPR32argSubClassMask,
    SuperRegIdxSeqs + 69,
    LaneBitmask(0x00000030),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    WSeqPairsClass_with_sube32_in_GPR32argSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64allRegClass = {
    &AArch64MCRegisterClasses[GPR64allRegClassID],
    GPR64allSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR64RegClass = {
    &AArch64MCRegisterClasses[FPR64RegClassID],
    FPR64SubClassMask,
    SuperRegIdxSeqs + 26,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64RegClass = {
    &AArch64MCRegisterClasses[GPR64RegClassID],
    GPR64SubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64Superclasses,
    GPR64GetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR64spRegClass = {
    &AArch64MCRegisterClasses[GPR64spRegClassID],
    GPR64spSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64spSuperclasses,
    GPR64spGetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR64commonRegClass = {
    &AArch64MCRegisterClasses[GPR64commonRegClassID],
    GPR64commonSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64commonSuperclasses,
    GPR64commonGetRawAllocationOrder
  };

  extern const TargetRegisterClass GPR64noipRegClass = {
    &AArch64MCRegisterClasses[GPR64noipRegClassID],
    GPR64noipSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64noipSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64common_and_GPR64noipRegClass = {
    &AArch64MCRegisterClasses[GPR64common_and_GPR64noipRegClassID],
    GPR64common_and_GPR64noipSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64common_and_GPR64noipSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass tcGPR64RegClass = {
    &AArch64MCRegisterClasses[tcGPR64RegClassID],
    tcGPR64SubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64noip_and_tcGPR64RegClass = {
    &AArch64MCRegisterClasses[GPR64noip_and_tcGPR64RegClassID],
    GPR64noip_and_tcGPR64SubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64noip_and_tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64argRegClass = {
    &AArch64MCRegisterClasses[GPR64argRegClassID],
    GPR64argSubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64argSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass rtcGPR64RegClass = {
    &AArch64MCRegisterClasses[rtcGPR64RegClassID],
    rtcGPR64SubClassMask,
    SuperRegIdxSeqs + 2,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    rtcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass GPR64sponlyRegClass = {
    &AArch64MCRegisterClasses[GPR64sponlyRegClassID],
    GPR64sponlySubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000008),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPR64sponlySuperclasses,
    nullptr
  };

  extern const TargetRegisterClass DDRegClass = {
    &AArch64MCRegisterClasses[DDRegClassID],
    DDSubClassMask,
    SuperRegIdxSeqs + 95,
    LaneBitmask(0x00000081),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClassRegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClassRegClassID],
    XSeqPairsClassSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64commonRegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_subo64_in_GPR64commonRegClassID],
    XSeqPairsClass_with_subo64_in_GPR64commonSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_subo64_in_GPR64commonSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64noipRegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_subo64_in_GPR64noipRegClassID],
    XSeqPairsClass_with_subo64_in_GPR64noipSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_subo64_in_GPR64noipSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_GPR64noipRegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_sube64_in_GPR64noipRegClassID],
    XSeqPairsClass_with_sube64_in_GPR64noipSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_sube64_in_GPR64noipSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_tcGPR64RegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_sube64_in_tcGPR64RegClassID],
    XSeqPairsClass_with_sube64_in_tcGPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_sube64_in_tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClassID],
    XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_tcGPR64RegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_subo64_in_tcGPR64RegClassID],
    XSeqPairsClass_with_subo64_in_tcGPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_subo64_in_tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClassID],
    XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_sub_32_in_GPR32argRegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_sub_32_in_GPR32argRegClassID],
    XSeqPairsClass_with_sub_32_in_GPR32argSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_sub_32_in_GPR32argSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass XSeqPairsClass_with_sube64_in_rtcGPR64RegClass = {
    &AArch64MCRegisterClasses[XSeqPairsClass_with_sube64_in_rtcGPR64RegClassID],
    XSeqPairsClass_with_sube64_in_rtcGPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00002008),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    XSeqPairsClass_with_sube64_in_rtcGPR64Superclasses,
    nullptr
  };

  extern const TargetRegisterClass FPR128RegClass = {
    &AArch64MCRegisterClasses[FPR128RegClassID],
    FPR128SubClassMask,
    SuperRegIdxSeqs + 60,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass ZPRRegClass = {
    &AArch64MCRegisterClasses[ZPRRegClassID],
    ZPRSubClassMask,
    SuperRegIdxSeqs + 5,
    LaneBitmask(0x00000041),
    0,
    true, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR128_loRegClass = {
    &AArch64MCRegisterClasses[FPR128_loRegClassID],
    FPR128_loSubClassMask,
    SuperRegIdxSeqs + 60,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR_4bRegClassID],
    ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 5,
    LaneBitmask(0x00000041),
    0,
    true, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR_3bRegClassID],
    ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 5,
    LaneBitmask(0x00000041),
    0,
    true, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass DDDRegClass = {
    &AArch64MCRegisterClasses[DDDRegClassID],
    DDDSubClassMask,
    SuperRegIdxSeqs + 83,
    LaneBitmask(0x00000281),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass DDDDRegClass = {
    &AArch64MCRegisterClasses[DDDDRegClassID],
    DDDDSubClassMask,
    SuperRegIdxSeqs + 71,
    LaneBitmask(0x00000381),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass QQRegClass = {
    &AArch64MCRegisterClasses[QQRegClassID],
    QQSubClassMask,
    SuperRegIdxSeqs + 105,
    LaneBitmask(0x00000401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2RegClass = {
    &AArch64MCRegisterClasses[ZPR2RegClassID],
    ZPR2SubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass QQ_with_qsub0_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQ_with_qsub0_in_FPR128_loRegClassID],
    QQ_with_qsub0_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 105,
    LaneBitmask(0x00000401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQ_with_qsub0_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQ_with_qsub1_in_FPR128_loRegClassID],
    QQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 105,
    LaneBitmask(0x00000401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub1_in_ZPR_4bRegClassID],
    ZPR2_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub_in_FPR128_loRegClassID],
    ZPR2_with_zsub_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClassID],
    QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 105,
    LaneBitmask(0x00000401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClassID],
    ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub0_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub0_in_ZPR_3bRegClassID],
    ZPR2_with_zsub0_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub0_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub1_in_ZPR_3bRegClassID],
    ZPR2_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClassID],
    ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 79,
    LaneBitmask(0x0000C041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQRegClass = {
    &AArch64MCRegisterClasses[QQQRegClassID],
    QQQSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3RegClass = {
    &AArch64MCRegisterClasses[ZPR3RegClassID],
    ZPR3SubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub0_in_FPR128_loRegClassID],
    QQQ_with_qsub0_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub0_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub1_in_FPR128_loRegClassID],
    QQQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub1_in_ZPR_4bRegClassID],
    ZPR3_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub2_in_ZPR_4bRegClassID],
    ZPR3_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub_in_FPR128_loRegClassID],
    ZPR3_with_zsub_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClassID],
    QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID],
    ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClassID],
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 90,
    LaneBitmask(0x00001401),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClassID],
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub0_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub0_in_ZPR_3bRegClassID],
    ZPR3_with_zsub0_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub0_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub1_in_ZPR_3bRegClassID],
    ZPR3_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub2_in_ZPR_3bRegClassID],
    ZPR3_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID],
    ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClassID],
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClassID],
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 76,
    LaneBitmask(0x000CC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQRegClass = {
    &AArch64MCRegisterClasses[QQQQRegClassID],
    QQQQSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4RegClass = {
    &AArch64MCRegisterClasses[ZPR4RegClassID],
    ZPR4SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub0_in_FPR128_loRegClassID],
    QQQQ_with_qsub0_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub0_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub1_in_FPR128_loRegClassID],
    QQQQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub3_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub3_in_FPR128_loRegClassID],
    QQQQ_with_qsub3_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub3_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_4bRegClassID],
    ZPR4_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub2_in_ZPR_4bRegClassID],
    ZPR4_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub3_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub3_in_ZPR_4bRegClassID],
    ZPR4_with_zsub3_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub3_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_loRegClassID],
    ZPR4_with_zsub_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClassID],
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID],
    QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID],
    ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID],
    ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClassID],
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID],
    QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID],
    ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass = {
    &AArch64MCRegisterClasses[QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClassID],
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSubClassMask,
    SuperRegIdxSeqs + 74,
    LaneBitmask(0x00001C01),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub0_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub0_in_ZPR_3bRegClassID],
    ZPR4_with_zsub0_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub0_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_3bRegClassID],
    ZPR4_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub2_in_ZPR_3bRegClassID],
    ZPR4_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub3_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub3_in_ZPR_3bRegClassID],
    ZPR4_with_zsub3_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub3_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID],
    ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID],
    ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID],
    ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClass = {
    &AArch64MCRegisterClasses[ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClassID],
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x000FC041),
    0,
    true, /* HasDisjunctSubRegs */
    true, /* CoveredBySubRegs */
    ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bSuperclasses,
    nullptr
  };

} // end namespace AArch64

namespace {
  const TargetRegisterClass* const RegisterClasses[] = {
    &AArch64::FPR8RegClass,
    &AArch64::FPR16RegClass,
    &AArch64::PPRRegClass,
    &AArch64::PPR_3bRegClass,
    &AArch64::GPR32allRegClass,
    &AArch64::FPR32RegClass,
    &AArch64::GPR32RegClass,
    &AArch64::GPR32spRegClass,
    &AArch64::GPR32commonRegClass,
    &AArch64::GPR32argRegClass,
    &AArch64::CCRRegClass,
    &AArch64::GPR32sponlyRegClass,
    &AArch64::WSeqPairsClassRegClass,
    &AArch64::WSeqPairsClass_with_subo32_in_GPR32commonRegClass,
    &AArch64::WSeqPairsClass_with_sube32_in_GPR32argRegClass,
    &AArch64::GPR64allRegClass,
    &AArch64::FPR64RegClass,
    &AArch64::GPR64RegClass,
    &AArch64::GPR64spRegClass,
    &AArch64::GPR64commonRegClass,
    &AArch64::GPR64noipRegClass,
    &AArch64::GPR64common_and_GPR64noipRegClass,
    &AArch64::tcGPR64RegClass,
    &AArch64::GPR64noip_and_tcGPR64RegClass,
    &AArch64::GPR64argRegClass,
    &AArch64::rtcGPR64RegClass,
    &AArch64::GPR64sponlyRegClass,
    &AArch64::DDRegClass,
    &AArch64::XSeqPairsClassRegClass,
    &AArch64::XSeqPairsClass_with_subo64_in_GPR64commonRegClass,
    &AArch64::XSeqPairsClass_with_subo64_in_GPR64noipRegClass,
    &AArch64::XSeqPairsClass_with_sube64_in_GPR64noipRegClass,
    &AArch64::XSeqPairsClass_with_sube64_in_tcGPR64RegClass,
    &AArch64::XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64RegClass,
    &AArch64::XSeqPairsClass_with_subo64_in_tcGPR64RegClass,
    &AArch64::XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64RegClass,
    &AArch64::XSeqPairsClass_with_sub_32_in_GPR32argRegClass,
    &AArch64::XSeqPairsClass_with_sube64_in_rtcGPR64RegClass,
    &AArch64::FPR128RegClass,
    &AArch64::ZPRRegClass,
    &AArch64::FPR128_loRegClass,
    &AArch64::ZPR_4bRegClass,
    &AArch64::ZPR_3bRegClass,
    &AArch64::DDDRegClass,
    &AArch64::DDDDRegClass,
    &AArch64::QQRegClass,
    &AArch64::ZPR2RegClass,
    &AArch64::QQ_with_qsub0_in_FPR128_loRegClass,
    &AArch64::QQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::ZPR2_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::ZPR2_with_zsub_in_FPR128_loRegClass,
    &AArch64::QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::ZPR2_with_zsub0_in_ZPR_3bRegClass,
    &AArch64::ZPR2_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::QQQRegClass,
    &AArch64::ZPR3RegClass,
    &AArch64::QQQ_with_qsub0_in_FPR128_loRegClass,
    &AArch64::QQQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::QQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::ZPR3_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::ZPR3_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::ZPR3_with_zsub_in_FPR128_loRegClass,
    &AArch64::QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::ZPR3_with_zsub0_in_ZPR_3bRegClass,
    &AArch64::ZPR3_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::ZPR3_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::QQQQRegClass,
    &AArch64::ZPR4RegClass,
    &AArch64::QQQQ_with_qsub0_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub3_in_FPR128_loRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub3_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4bRegClass,
    &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_loRegClass,
    &AArch64::QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4bRegClass,
    &AArch64::QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_loRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4bRegClass,
    &AArch64::ZPR4_with_zsub0_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub3_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3bRegClass,
    &AArch64::ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3bRegClass,
  };
} // end anonymous namespace

static const TargetRegisterInfoDesc AArch64RegInfoDesc[] = { // Extra Descriptors
  { 0, false },
  { 0, false },
  { 0, true },
  { 0, true },
  { 0, false },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, false },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
};
unsigned AArch64GenRegisterInfo::composeSubRegIndicesImpl(unsigned IdxA, unsigned IdxB) const {
  static const uint8_t RowMap[99] = {
    0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0, 1, 0, 0, 7, 8, 9, 0, 0, 1, 1, 0, 3, 3, 0, 2, 2, 0, 4, 4, 4, 0, 6, 6, 6, 0, 5, 5, 5, 0, 0, 7, 7, 7, 7, 0, 0, 9, 9, 9, 9, 0, 0, 8, 8, 8, 8, 0, 0, 0, 1, 1, 2, 10, 10, 10, 0, 0, 4, 4, 5, 4, 4, 5, 0, 11, 10, 11, 11, 10, 10, 0, 0, 7, 7, 8, 7, 7, 7, 7, 8, 8, 
  };
  static const uint8_t Rows[12][99] = {
    { AArch64::bsub, AArch64::dsub, AArch64::dsub0, AArch64::dsub1, AArch64::dsub2, 0, AArch64::hsub, 0, 0, AArch64::qsub0, AArch64::qsub1, AArch64::qsub2, 0, AArch64::ssub, AArch64::sub_32, AArch64::sub_32, 0, AArch64::subo64_then_sub_32, 0, AArch64::zsub, AArch64::zsub0, AArch64::zsub1, AArch64::zsub2, 0, AArch64::zsub_hi, AArch64::dsub1_then_bsub, AArch64::dsub1_then_hsub, AArch64::dsub1_then_ssub, 0, 0, 0, AArch64::dsub2_then_bsub, AArch64::dsub2_then_hsub, AArch64::dsub2_then_ssub, AArch64::qsub1_then_bsub, AArch64::qsub1_then_dsub, AArch64::qsub1_then_hsub, AArch64::qsub1_then_ssub, 0, 0, 0, 0, AArch64::qsub2_then_bsub, AArch64::qsub2_then_dsub, AArch64::qsub2_then_hsub, AArch64::qsub2_then_ssub, 0, AArch64::zsub1_then_bsub, AArch64::zsub1_then_dsub, AArch64::zsub1_then_hsub, AArch64::zsub1_then_ssub, AArch64::zsub1_then_zsub, AArch64::zsub1_then_zsub_hi, 0, 0, 0, 0, 0, 0, AArch64::zsub2_then_bsub, AArch64::zsub2_then_dsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, AArch64::zsub2_then_zsub, AArch64::zsub2_then_zsub_hi, AArch64::dsub0_dsub1, 0, AArch64::dsub1_dsub2, 0, 0, AArch64::dsub_qsub1_then_dsub, 0, AArch64::dsub_qsub1_then_dsub_qsub2_then_dsub, AArch64::qsub0_qsub1, 0, AArch64::qsub1_qsub2, 0, 0, AArch64::qsub1_then_dsub_qsub2_then_dsub, 0, 0, 0, AArch64::dsub_zsub1_then_dsub, AArch64::zsub_zsub1_then_zsub, 0, AArch64::dsub_zsub1_then_dsub_zsub2_then_dsub, 0, AArch64::zsub_zsub1_then_zsub_zsub2_then_zsub, AArch64::zsub0_zsub1, 0, AArch64::zsub1_zsub2, 0, 0, AArch64::zsub1_then_dsub_zsub2_then_dsub, 0, AArch64::zsub1_then_zsub_zsub2_then_zsub, 0, 0, 0, },
    { AArch64::dsub1_then_bsub, 0, AArch64::dsub1, AArch64::dsub2, AArch64::dsub3, 0, AArch64::dsub1_then_hsub, 0, 0, 0, 0, 0, 0, AArch64::dsub1_then_ssub, AArch64::subo64_then_sub_32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::dsub2_then_bsub, AArch64::dsub2_then_hsub, AArch64::dsub2_then_ssub, 0, 0, 0, AArch64::dsub3_then_bsub, AArch64::dsub3_then_hsub, AArch64::dsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::dsub1_dsub2, 0, AArch64::dsub2_dsub3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::dsub2_then_bsub, 0, AArch64::dsub2, AArch64::dsub3, 0, 0, AArch64::dsub2_then_hsub, 0, 0, 0, 0, 0, 0, AArch64::dsub2_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::dsub3_then_bsub, AArch64::dsub3_then_hsub, AArch64::dsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::dsub3_then_bsub, 0, 0, 0, 0, 0, AArch64::dsub3_then_hsub, 0, 0, 0, 0, 0, 0, AArch64::dsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::qsub1_then_bsub, AArch64::qsub1_then_dsub, AArch64::qsub1_then_dsub, AArch64::qsub2_then_dsub, AArch64::qsub3_then_dsub, 0, AArch64::qsub1_then_hsub, 0, 0, AArch64::qsub1, AArch64::qsub2, AArch64::qsub3, 0, AArch64::qsub1_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::qsub2_then_bsub, AArch64::qsub2_then_hsub, AArch64::qsub2_then_ssub, 0, 0, 0, AArch64::qsub3_then_bsub, AArch64::qsub3_then_hsub, AArch64::qsub3_then_ssub, AArch64::qsub2_then_bsub, AArch64::qsub2_then_dsub, AArch64::qsub2_then_hsub, AArch64::qsub2_then_ssub, 0, 0, 0, 0, AArch64::qsub3_then_bsub, AArch64::qsub3_then_dsub, AArch64::qsub3_then_hsub, AArch64::qsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::qsub1_then_dsub_qsub2_then_dsub, 0, AArch64::qsub2_then_dsub_qsub3_then_dsub, 0, 0, AArch64::qsub1_then_dsub_qsub2_then_dsub, 0, AArch64::qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub, AArch64::qsub1_qsub2, 0, AArch64::qsub2_qsub3, 0, 0, AArch64::qsub2_then_dsub_qsub3_then_dsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::qsub2_then_bsub, AArch64::qsub2_then_dsub, AArch64::qsub2_then_dsub, AArch64::qsub3_then_dsub, 0, 0, AArch64::qsub2_then_hsub, 0, 0, AArch64::qsub2, AArch64::qsub3, 0, 0, AArch64::qsub2_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::qsub3_then_bsub, AArch64::qsub3_then_hsub, AArch64::qsub3_then_ssub, 0, 0, 0, 0, 0, 0, AArch64::qsub3_then_bsub, AArch64::qsub3_then_dsub, AArch64::qsub3_then_hsub, AArch64::qsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::qsub2_then_dsub_qsub3_then_dsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::qsub3_then_bsub, AArch64::qsub3_then_dsub, 0, 0, 0, 0, AArch64::qsub3_then_hsub, 0, 0, 0, 0, 0, 0, AArch64::qsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::zsub1_then_bsub, AArch64::zsub1_then_dsub, AArch64::zsub1_then_dsub, AArch64::zsub2_then_dsub, AArch64::zsub3_then_dsub, 0, AArch64::zsub1_then_hsub, 0, 0, AArch64::zsub1_then_zsub, AArch64::zsub2_then_zsub, AArch64::zsub3_then_zsub, 0, AArch64::zsub1_then_ssub, 0, 0, 0, 0, 0, AArch64::zsub1_then_zsub, AArch64::zsub1, AArch64::zsub2, AArch64::zsub3, 0, AArch64::zsub1_then_zsub_hi, AArch64::zsub2_then_bsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, 0, 0, 0, AArch64::zsub3_then_bsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, AArch64::zsub2_then_bsub, AArch64::zsub2_then_dsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, 0, 0, 0, 0, AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, 0, AArch64::zsub2_then_bsub, AArch64::zsub2_then_dsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, AArch64::zsub2_then_zsub, AArch64::zsub2_then_zsub_hi, 0, 0, 0, 0, 0, 0, AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, AArch64::zsub3_then_zsub, AArch64::zsub3_then_zsub_hi, AArch64::zsub1_then_dsub_zsub2_then_dsub, 0, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, 0, AArch64::zsub1_then_dsub_zsub2_then_dsub, 0, AArch64::zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub, AArch64::zsub1_then_zsub_zsub2_then_zsub, 0, AArch64::zsub2_then_zsub_zsub3_then_zsub, 0, 0, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, 0, 0, AArch64::zsub1_then_dsub_zsub2_then_dsub, AArch64::zsub1_then_zsub_zsub2_then_zsub, 0, AArch64::zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub, 0, AArch64::zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub, AArch64::zsub1_zsub2, 0, AArch64::zsub2_zsub3, 0, 0, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, AArch64::zsub2_then_zsub_zsub3_then_zsub, 0, 0, 0, },
    { AArch64::zsub2_then_bsub, AArch64::zsub2_then_dsub, AArch64::zsub2_then_dsub, AArch64::zsub3_then_dsub, 0, 0, AArch64::zsub2_then_hsub, 0, 0, AArch64::zsub2_then_zsub, AArch64::zsub3_then_zsub, 0, 0, AArch64::zsub2_then_ssub, 0, 0, 0, 0, 0, AArch64::zsub2_then_zsub, AArch64::zsub2, AArch64::zsub3, 0, 0, AArch64::zsub2_then_zsub_hi, AArch64::zsub3_then_bsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, 0, 0, 0, 0, 0, 0, AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, AArch64::zsub3_then_zsub, AArch64::zsub3_then_zsub_hi, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::zsub2_then_dsub_zsub3_then_dsub, AArch64::zsub2_then_zsub_zsub3_then_zsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, 0, 0, 0, 0, AArch64::zsub3_then_hsub, 0, 0, 0, 0, 0, 0, AArch64::zsub3_then_ssub, 0, 0, 0, 0, 0, AArch64::zsub3_then_zsub, 0, 0, 0, 0, AArch64::zsub3_then_zsub_hi, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::bsub, AArch64::dsub, AArch64::dsub, AArch64::qsub1_then_dsub, AArch64::qsub2_then_dsub, AArch64::qsub3_then_dsub, AArch64::hsub, 0, 0, AArch64::zsub, AArch64::zsub1_then_zsub, AArch64::zsub2_then_zsub, AArch64::zsub3_then_zsub, AArch64::ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::qsub1_then_bsub, AArch64::qsub1_then_hsub, AArch64::qsub1_then_ssub, AArch64::qsub3_then_bsub, AArch64::qsub3_then_hsub, AArch64::qsub3_then_ssub, AArch64::qsub2_then_bsub, AArch64::qsub2_then_hsub, AArch64::qsub2_then_ssub, AArch64::zsub1_then_bsub, AArch64::zsub1_then_dsub, AArch64::zsub1_then_hsub, AArch64::zsub1_then_ssub, AArch64::zsub3_then_bsub, AArch64::zsub3_then_dsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, AArch64::zsub2_then_bsub, AArch64::zsub2_then_dsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::dsub_qsub1_then_dsub, AArch64::dsub_qsub1_then_dsub_qsub2_then_dsub, AArch64::qsub1_then_dsub_qsub2_then_dsub, AArch64::qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub, AArch64::qsub2_then_dsub_qsub3_then_dsub, AArch64::dsub_zsub1_then_dsub, AArch64::dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub, AArch64::dsub_zsub1_then_dsub_zsub2_then_dsub, AArch64::zsub_zsub1_then_zsub, AArch64::zsub_zsub1_then_zsub_zsub2_then_zsub, AArch64::zsub1_then_zsub_zsub2_then_zsub, AArch64::zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub, AArch64::zsub2_then_zsub_zsub3_then_zsub, AArch64::zsub1_then_dsub_zsub2_then_dsub, AArch64::zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { AArch64::bsub, 0, AArch64::dsub, AArch64::zsub1_then_dsub, AArch64::zsub2_then_dsub, AArch64::zsub3_then_dsub, AArch64::hsub, 0, 0, 0, 0, 0, 0, AArch64::ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::zsub1_then_bsub, AArch64::zsub1_then_hsub, AArch64::zsub1_then_ssub, AArch64::zsub3_then_bsub, AArch64::zsub3_then_hsub, AArch64::zsub3_then_ssub, AArch64::zsub2_then_bsub, AArch64::zsub2_then_hsub, AArch64::zsub2_then_ssub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AArch64::dsub_zsub1_then_dsub, AArch64::dsub_zsub1_then_dsub_zsub2_then_dsub, AArch64::zsub1_then_dsub_zsub2_then_dsub, AArch64::zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub, AArch64::zsub2_then_dsub_zsub3_then_dsub, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  };

  --IdxA; assert(IdxA < 99);
  --IdxB; assert(IdxB < 99);
  return Rows[RowMap[IdxA]][IdxB];
}

  struct MaskRolOp {
    LaneBitmask Mask;
    uint8_t  RotateLeft;
  };
  static const MaskRolOp LaneMaskComposeSequences[] = {
    { LaneBitmask(0xFFFFFFFF),  0 }, { LaneBitmask::getNone(), 0 },   // Sequence 0
    { LaneBitmask(0xFFFFFFFF),  7 }, { LaneBitmask::getNone(), 0 },   // Sequence 2
    { LaneBitmask(0xFFFFFFFF),  9 }, { LaneBitmask::getNone(), 0 },   // Sequence 4
    { LaneBitmask(0xFFFFFFFF),  8 }, { LaneBitmask::getNone(), 0 },   // Sequence 6
    { LaneBitmask(0xFFFFFFFF),  1 }, { LaneBitmask::getNone(), 0 },   // Sequence 8
    { LaneBitmask(0xFFFFFFFF),  2 }, { LaneBitmask::getNone(), 0 },   // Sequence 10
    { LaneBitmask(0xFFFFFFFF), 10 }, { LaneBitmask::getNone(), 0 },   // Sequence 12
    { LaneBitmask(0xFFFFFFFF), 12 }, { LaneBitmask::getNone(), 0 },   // Sequence 14
    { LaneBitmask(0xFFFFFFFF), 11 }, { LaneBitmask::getNone(), 0 },   // Sequence 16
    { LaneBitmask(0xFFFFFFFF),  3 }, { LaneBitmask::getNone(), 0 },   // Sequence 18
    { LaneBitmask(0xFFFFFFFF),  4 }, { LaneBitmask::getNone(), 0 },   // Sequence 20
    { LaneBitmask(0xFFFFFFFF),  5 }, { LaneBitmask::getNone(), 0 },   // Sequence 22
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000040),  9 }, { LaneBitmask::getNone(), 0 },   // Sequence 24
    { LaneBitmask(0x00000001), 18 }, { LaneBitmask(0x00000040), 13 }, { LaneBitmask::getNone(), 0 },   // Sequence 27
    { LaneBitmask(0x00000001), 16 }, { LaneBitmask(0x00000040), 11 }, { LaneBitmask::getNone(), 0 },   // Sequence 30
    { LaneBitmask(0xFFFFFFFF),  6 }, { LaneBitmask::getNone(), 0 },   // Sequence 33
    { LaneBitmask(0xFFFFFFFF), 13 }, { LaneBitmask::getNone(), 0 },   // Sequence 35
    { LaneBitmask(0xFFFFFFFF), 14 }, { LaneBitmask::getNone(), 0 },   // Sequence 37
    { LaneBitmask(0xFFFFFFFF), 15 }, { LaneBitmask::getNone(), 0 },   // Sequence 39
    { LaneBitmask(0xFFFFFFFF), 16 }, { LaneBitmask::getNone(), 0 },   // Sequence 41
    { LaneBitmask(0xFFFFFFFF), 17 }, { LaneBitmask::getNone(), 0 },   // Sequence 43
    { LaneBitmask(0xFFFFFFFF), 18 }, { LaneBitmask::getNone(), 0 },   // Sequence 45
    { LaneBitmask(0xFFFFFFFF), 19 }, { LaneBitmask::getNone(), 0 },   // Sequence 47
    { LaneBitmask(0x00000001),  7 }, { LaneBitmask(0x00000080),  2 }, { LaneBitmask::getNone(), 0 },   // Sequence 49
    { LaneBitmask(0x00000001),  7 }, { LaneBitmask(0x00000080),  2 }, { LaneBitmask(0x00000200), 31 }, { LaneBitmask::getNone(), 0 },   // Sequence 52
    { LaneBitmask(0x00000001),  9 }, { LaneBitmask(0x00000080),  1 }, { LaneBitmask::getNone(), 0 },   // Sequence 56
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000080),  3 }, { LaneBitmask::getNone(), 0 },   // Sequence 59
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000380),  3 }, { LaneBitmask::getNone(), 0 },   // Sequence 62
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000280),  3 }, { LaneBitmask::getNone(), 0 },   // Sequence 65
    { LaneBitmask(0x00000001), 10 }, { LaneBitmask(0x00000400),  2 }, { LaneBitmask::getNone(), 0 },   // Sequence 68
    { LaneBitmask(0x00000001), 10 }, { LaneBitmask(0x00000400),  2 }, { LaneBitmask(0x00001000), 31 }, { LaneBitmask::getNone(), 0 },   // Sequence 71
    { LaneBitmask(0x00000001), 12 }, { LaneBitmask(0x00000400),  1 }, { LaneBitmask::getNone(), 0 },   // Sequence 75
    { LaneBitmask(0x00000001), 10 }, { LaneBitmask(0x00000080),  5 }, { LaneBitmask::getNone(), 0 },   // Sequence 78
    { LaneBitmask(0x00000001), 10 }, { LaneBitmask(0x00000080),  5 }, { LaneBitmask(0x00000200),  2 }, { LaneBitmask::getNone(), 0 },   // Sequence 81
    { LaneBitmask(0x00000001), 12 }, { LaneBitmask(0x00000080),  4 }, { LaneBitmask::getNone(), 0 },   // Sequence 85
    { LaneBitmask(0x00000010), 31 }, { LaneBitmask(0x00000020),  8 }, { LaneBitmask::getNone(), 0 },   // Sequence 88
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000080),  7 }, { LaneBitmask::getNone(), 0 },   // Sequence 91
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000400),  4 }, { LaneBitmask::getNone(), 0 },   // Sequence 94
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000080),  7 }, { LaneBitmask(0x00000100),  8 }, { LaneBitmask(0x00000200),  9 }, { LaneBitmask::getNone(), 0 },   // Sequence 97
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000080),  7 }, { LaneBitmask(0x00000200),  9 }, { LaneBitmask::getNone(), 0 },   // Sequence 102
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000400),  4 }, { LaneBitmask(0x00000800),  5 }, { LaneBitmask(0x00001000),  6 }, { LaneBitmask::getNone(), 0 },   // Sequence 106
    { LaneBitmask(0x00000001),  0 }, { LaneBitmask(0x00000400),  4 }, { LaneBitmask(0x00001000),  6 }, { LaneBitmask::getNone(), 0 },   // Sequence 111
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000040),  9 }, { LaneBitmask(0x0000C000),  4 }, { LaneBitmask::getNone(), 0 },   // Sequence 115
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000040),  9 }, { LaneBitmask(0x0000C000),  4 }, { LaneBitmask(0x000C0000), 30 }, { LaneBitmask::getNone(), 0 },   // Sequence 119
    { LaneBitmask(0x00000001), 18 }, { LaneBitmask(0x00000040), 13 }, { LaneBitmask(0x0000C000),  2 }, { LaneBitmask::getNone(), 0 },   // Sequence 124
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000080), 11 }, { LaneBitmask::getNone(), 0 },   // Sequence 128
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000080), 11 }, { LaneBitmask(0x00000200),  7 }, { LaneBitmask::getNone(), 0 },   // Sequence 131
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000400),  8 }, { LaneBitmask::getNone(), 0 },   // Sequence 135
    { LaneBitmask(0x00000001), 14 }, { LaneBitmask(0x00000400),  8 }, { LaneBitmask(0x00001000),  4 }, { LaneBitmask::getNone(), 0 },   // Sequence 138
    { LaneBitmask(0x00000001), 18 }, { LaneBitmask(0x00000080),  9 }, { LaneBitmask::getNone(), 0 },   // Sequence 142
    { LaneBitmask(0x00000001), 18 }, { LaneBitmask(0x00000400),  6 }, { LaneBitmask::getNone(), 0 }  // Sequence 145
  };
  static const MaskRolOp *const CompositeSequences[] = {
    &LaneMaskComposeSequences[0], // to bsub
    &LaneMaskComposeSequences[0], // to dsub
    &LaneMaskComposeSequences[0], // to dsub0
    &LaneMaskComposeSequences[2], // to dsub1
    &LaneMaskComposeSequences[4], // to dsub2
    &LaneMaskComposeSequences[6], // to dsub3
    &LaneMaskComposeSequences[0], // to hsub
    &LaneMaskComposeSequences[8], // to qhisub
    &LaneMaskComposeSequences[10], // to qsub
    &LaneMaskComposeSequences[0], // to qsub0
    &LaneMaskComposeSequences[12], // to qsub1
    &LaneMaskComposeSequences[14], // to qsub2
    &LaneMaskComposeSequences[16], // to qsub3
    &LaneMaskComposeSequences[0], // to ssub
    &LaneMaskComposeSequences[18], // to sub_32
    &LaneMaskComposeSequences[20], // to sube32
    &LaneMaskComposeSequences[0], // to sube64
    &LaneMaskComposeSequences[22], // to subo32
    &LaneMaskComposeSequences[12], // to subo64
    &LaneMaskComposeSequences[0], // to zsub
    &LaneMaskComposeSequences[0], // to zsub0
    &LaneMaskComposeSequences[24], // to zsub1
    &LaneMaskComposeSequences[27], // to zsub2
    &LaneMaskComposeSequences[30], // to zsub3
    &LaneMaskComposeSequences[33], // to zsub_hi
    &LaneMaskComposeSequences[2], // to dsub1_then_bsub
    &LaneMaskComposeSequences[2], // to dsub1_then_hsub
    &LaneMaskComposeSequences[2], // to dsub1_then_ssub
    &LaneMaskComposeSequences[6], // to dsub3_then_bsub
    &LaneMaskComposeSequences[6], // to dsub3_then_hsub
    &LaneMaskComposeSequences[6], // to dsub3_then_ssub
    &LaneMaskComposeSequences[4], // to dsub2_then_bsub
    &LaneMaskComposeSequences[4], // to dsub2_then_hsub
    &LaneMaskComposeSequences[4], // to dsub2_then_ssub
    &LaneMaskComposeSequences[12], // to qsub1_then_bsub
    &LaneMaskComposeSequences[12], // to qsub1_then_dsub
    &LaneMaskComposeSequences[12], // to qsub1_then_hsub
    &LaneMaskComposeSequences[12], // to qsub1_then_ssub
    &LaneMaskComposeSequences[16], // to qsub3_then_bsub
    &LaneMaskComposeSequences[16], // to qsub3_then_dsub
    &LaneMaskComposeSequences[16], // to qsub3_then_hsub
    &LaneMaskComposeSequences[16], // to qsub3_then_ssub
    &LaneMaskComposeSequences[14], // to qsub2_then_bsub
    &LaneMaskComposeSequences[14], // to qsub2_then_dsub
    &LaneMaskComposeSequences[14], // to qsub2_then_hsub
    &LaneMaskComposeSequences[14], // to qsub2_then_ssub
    &LaneMaskComposeSequences[35], // to subo64_then_sub_32
    &LaneMaskComposeSequences[37], // to zsub1_then_bsub
    &LaneMaskComposeSequences[37], // to zsub1_then_dsub
    &LaneMaskComposeSequences[37], // to zsub1_then_hsub
    &LaneMaskComposeSequences[37], // to zsub1_then_ssub
    &LaneMaskComposeSequences[37], // to zsub1_then_zsub
    &LaneMaskComposeSequences[39], // to zsub1_then_zsub_hi
    &LaneMaskComposeSequences[41], // to zsub3_then_bsub
    &LaneMaskComposeSequences[41], // to zsub3_then_dsub
    &LaneMaskComposeSequences[41], // to zsub3_then_hsub
    &LaneMaskComposeSequences[41], // to zsub3_then_ssub
    &LaneMaskComposeSequences[41], // to zsub3_then_zsub
    &LaneMaskComposeSequences[43], // to zsub3_then_zsub_hi
    &LaneMaskComposeSequences[45], // to zsub2_then_bsub
    &LaneMaskComposeSequences[45], // to zsub2_then_dsub
    &LaneMaskComposeSequences[45], // to zsub2_then_hsub
    &LaneMaskComposeSequences[45], // to zsub2_then_ssub
    &LaneMaskComposeSequences[45], // to zsub2_then_zsub
    &LaneMaskComposeSequences[47], // to zsub2_then_zsub_hi
    &LaneMaskComposeSequences[0], // to dsub0_dsub1
    &LaneMaskComposeSequences[0], // to dsub0_dsub1_dsub2
    &LaneMaskComposeSequences[49], // to dsub1_dsub2
    &LaneMaskComposeSequences[52], // to dsub1_dsub2_dsub3
    &LaneMaskComposeSequences[56], // to dsub2_dsub3
    &LaneMaskComposeSequences[59], // to dsub_qsub1_then_dsub
    &LaneMaskComposeSequences[62], // to dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
    &LaneMaskComposeSequences[65], // to dsub_qsub1_then_dsub_qsub2_then_dsub
    &LaneMaskComposeSequences[0], // to qsub0_qsub1
    &LaneMaskComposeSequences[0], // to qsub0_qsub1_qsub2
    &LaneMaskComposeSequences[68], // to qsub1_qsub2
    &LaneMaskComposeSequences[71], // to qsub1_qsub2_qsub3
    &LaneMaskComposeSequences[75], // to qsub2_qsub3
    &LaneMaskComposeSequences[78], // to qsub1_then_dsub_qsub2_then_dsub
    &LaneMaskComposeSequences[81], // to qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
    &LaneMaskComposeSequences[85], // to qsub2_then_dsub_qsub3_then_dsub
    &LaneMaskComposeSequences[88], // to sub_32_subo64_then_sub_32
    &LaneMaskComposeSequences[91], // to dsub_zsub1_then_dsub
    &LaneMaskComposeSequences[94], // to zsub_zsub1_then_zsub
    &LaneMaskComposeSequences[97], // to dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
    &LaneMaskComposeSequences[102], // to dsub_zsub1_then_dsub_zsub2_then_dsub
    &LaneMaskComposeSequences[106], // to zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
    &LaneMaskComposeSequences[111], // to zsub_zsub1_then_zsub_zsub2_then_zsub
    &LaneMaskComposeSequences[0], // to zsub0_zsub1
    &LaneMaskComposeSequences[0], // to zsub0_zsub1_zsub2
    &LaneMaskComposeSequences[115], // to zsub1_zsub2
    &LaneMaskComposeSequences[119], // to zsub1_zsub2_zsub3
    &LaneMaskComposeSequences[124], // to zsub2_zsub3
    &LaneMaskComposeSequences[128], // to zsub1_then_dsub_zsub2_then_dsub
    &LaneMaskComposeSequences[131], // to zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
    &LaneMaskComposeSequences[135], // to zsub1_then_zsub_zsub2_then_zsub
    &LaneMaskComposeSequences[138], // to zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
    &LaneMaskComposeSequences[142], // to zsub2_then_dsub_zsub3_then_dsub
    &LaneMaskComposeSequences[145] // to zsub2_then_zsub_zsub3_then_zsub
  };

LaneBitmask AArch64GenRegisterInfo::composeSubRegIndexLaneMaskImpl(unsigned IdxA, LaneBitmask LaneMask) const {
  --IdxA; assert(IdxA < 99 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops = CompositeSequences[IdxA]; Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger() & Ops->Mask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M << S) | (M >> (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

LaneBitmask AArch64GenRegisterInfo::reverseComposeSubRegIndexLaneMaskImpl(unsigned IdxA,  LaneBitmask LaneMask) const {
  LaneMask &= getSubRegIndexLaneMask(IdxA);
  --IdxA; assert(IdxA < 99 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops = CompositeSequences[IdxA]; Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M >> S) | (M << (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

const TargetRegisterClass *AArch64GenRegisterInfo::getSubClassWithSubReg(const TargetRegisterClass *RC, unsigned Idx) const {
  static const uint8_t Table[108][99] = {
    {	// FPR8
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// FPR16
      2,	// bsub -> FPR16
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// PPR
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// PPR_3b
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32all
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// FPR32
      6,	// bsub -> FPR32
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      6,	// hsub -> FPR32
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32sp
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32common
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32arg
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// CCR
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR32sponly
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// WSeqPairsClass
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      13,	// sube32 -> WSeqPairsClass
      0,	// sube64
      13,	// subo32 -> WSeqPairsClass
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// WSeqPairsClass_with_subo32_in_GPR32common
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      14,	// sube32 -> WSeqPairsClass_with_subo32_in_GPR32common
      0,	// sube64
      14,	// subo32 -> WSeqPairsClass_with_subo32_in_GPR32common
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// WSeqPairsClass_with_sube32_in_GPR32arg
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      0,	// sub_32
      15,	// sube32 -> WSeqPairsClass_with_sube32_in_GPR32arg
      0,	// sube64
      15,	// subo32 -> WSeqPairsClass_with_sube32_in_GPR32arg
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64all
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      16,	// sub_32 -> GPR64all
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// FPR64
      17,	// bsub -> FPR64
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      17,	// hsub -> FPR64
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      17,	// ssub -> FPR64
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      18,	// sub_32 -> GPR64
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64sp
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      19,	// sub_32 -> GPR64sp
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64common
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      20,	// sub_32 -> GPR64common
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64noip
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      21,	// sub_32 -> GPR64noip
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64common_and_GPR64noip
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      22,	// sub_32 -> GPR64common_and_GPR64noip
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      23,	// sub_32 -> tcGPR64
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64noip_and_tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      24,	// sub_32 -> GPR64noip_and_tcGPR64
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64arg
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      25,	// sub_32 -> GPR64arg
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// rtcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      26,	// sub_32 -> rtcGPR64
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// GPR64sponly
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      27,	// sub_32 -> GPR64sponly
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// DD
      28,	// bsub -> DD
      0,	// dsub
      28,	// dsub0 -> DD
      28,	// dsub1 -> DD
      0,	// dsub2
      0,	// dsub3
      28,	// hsub -> DD
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      28,	// ssub -> DD
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      28,	// dsub1_then_bsub -> DD
      28,	// dsub1_then_hsub -> DD
      28,	// dsub1_then_ssub -> DD
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      29,	// sub_32 -> XSeqPairsClass
      0,	// sube32
      29,	// sube64 -> XSeqPairsClass
      0,	// subo32
      29,	// subo64 -> XSeqPairsClass
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      29,	// subo64_then_sub_32 -> XSeqPairsClass
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      29,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_subo64_in_GPR64common
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      30,	// sub_32 -> XSeqPairsClass_with_subo64_in_GPR64common
      0,	// sube32
      30,	// sube64 -> XSeqPairsClass_with_subo64_in_GPR64common
      0,	// subo32
      30,	// subo64 -> XSeqPairsClass_with_subo64_in_GPR64common
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      30,	// subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64common
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      30,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64common
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      31,	// sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// sube32
      31,	// sube64 -> XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// subo32
      31,	// subo64 -> XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      31,	// subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      31,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      32,	// sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// sube32
      32,	// sube64 -> XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// subo32
      32,	// subo64 -> XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      32,	// subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      32,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      33,	// sub_32 -> XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// sube32
      33,	// sube64 -> XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// subo32
      33,	// subo64 -> XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      33,	// subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      33,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_tcGPR64
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      34,	// sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// sube32
      34,	// sube64 -> XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// subo32
      34,	// subo64 -> XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      34,	// subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      34,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      35,	// sub_32 -> XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// sube32
      35,	// sube64 -> XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// subo32
      35,	// subo64 -> XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      35,	// subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      35,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_tcGPR64
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      36,	// sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// sube32
      36,	// sube64 -> XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// subo32
      36,	// subo64 -> XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      36,	// subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      36,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      37,	// sub_32 -> XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// sube32
      37,	// sube64 -> XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// subo32
      37,	// subo64 -> XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      37,	// subo64_then_sub_32 -> XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      37,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_sub_32_in_GPR32arg
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// bsub
      0,	// dsub
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      0,	// hsub
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      0,	// ssub
      38,	// sub_32 -> XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// sube32
      38,	// sube64 -> XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// subo32
      38,	// subo64 -> XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      38,	// subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      38,	// sub_32_subo64_then_sub_32 -> XSeqPairsClass_with_sube64_in_rtcGPR64
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// FPR128
      39,	// bsub -> FPR128
      39,	// dsub -> FPR128
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      39,	// hsub -> FPR128
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      39,	// ssub -> FPR128
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR
      40,	// bsub -> ZPR
      40,	// dsub -> ZPR
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      40,	// hsub -> ZPR
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      40,	// ssub -> ZPR
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      40,	// zsub -> ZPR
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      40,	// zsub_hi -> ZPR
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// FPR128_lo
      41,	// bsub -> FPR128_lo
      41,	// dsub -> FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      41,	// hsub -> FPR128_lo
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      41,	// ssub -> FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR_4b
      42,	// bsub -> ZPR_4b
      42,	// dsub -> ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      42,	// hsub -> ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      42,	// ssub -> ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      42,	// zsub -> ZPR_4b
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      42,	// zsub_hi -> ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR_3b
      43,	// bsub -> ZPR_3b
      43,	// dsub -> ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      43,	// hsub -> ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      43,	// ssub -> ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      43,	// zsub -> ZPR_3b
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      43,	// zsub_hi -> ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// DDD
      44,	// bsub -> DDD
      0,	// dsub
      44,	// dsub0 -> DDD
      44,	// dsub1 -> DDD
      44,	// dsub2 -> DDD
      0,	// dsub3
      44,	// hsub -> DDD
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      44,	// ssub -> DDD
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      44,	// dsub1_then_bsub -> DDD
      44,	// dsub1_then_hsub -> DDD
      44,	// dsub1_then_ssub -> DDD
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      44,	// dsub2_then_bsub -> DDD
      44,	// dsub2_then_hsub -> DDD
      44,	// dsub2_then_ssub -> DDD
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      44,	// dsub0_dsub1 -> DDD
      0,	// dsub0_dsub1_dsub2
      44,	// dsub1_dsub2 -> DDD
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// DDDD
      45,	// bsub -> DDDD
      0,	// dsub
      45,	// dsub0 -> DDDD
      45,	// dsub1 -> DDDD
      45,	// dsub2 -> DDDD
      45,	// dsub3 -> DDDD
      45,	// hsub -> DDDD
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      45,	// ssub -> DDDD
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      45,	// dsub1_then_bsub -> DDDD
      45,	// dsub1_then_hsub -> DDDD
      45,	// dsub1_then_ssub -> DDDD
      45,	// dsub3_then_bsub -> DDDD
      45,	// dsub3_then_hsub -> DDDD
      45,	// dsub3_then_ssub -> DDDD
      45,	// dsub2_then_bsub -> DDDD
      45,	// dsub2_then_hsub -> DDDD
      45,	// dsub2_then_ssub -> DDDD
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      45,	// dsub0_dsub1 -> DDDD
      45,	// dsub0_dsub1_dsub2 -> DDDD
      45,	// dsub1_dsub2 -> DDDD
      45,	// dsub1_dsub2_dsub3 -> DDDD
      45,	// dsub2_dsub3 -> DDDD
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQ
      46,	// bsub -> QQ
      46,	// dsub -> QQ
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      46,	// hsub -> QQ
      0,	// qhisub
      0,	// qsub
      46,	// qsub0 -> QQ
      46,	// qsub1 -> QQ
      0,	// qsub2
      0,	// qsub3
      46,	// ssub -> QQ
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      46,	// qsub1_then_bsub -> QQ
      46,	// qsub1_then_dsub -> QQ
      46,	// qsub1_then_hsub -> QQ
      46,	// qsub1_then_ssub -> QQ
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      46,	// dsub_qsub1_then_dsub -> QQ
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2
      47,	// bsub -> ZPR2
      47,	// dsub -> ZPR2
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      47,	// hsub -> ZPR2
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      47,	// ssub -> ZPR2
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      47,	// zsub -> ZPR2
      47,	// zsub0 -> ZPR2
      47,	// zsub1 -> ZPR2
      0,	// zsub2
      0,	// zsub3
      47,	// zsub_hi -> ZPR2
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      47,	// zsub1_then_bsub -> ZPR2
      47,	// zsub1_then_dsub -> ZPR2
      47,	// zsub1_then_hsub -> ZPR2
      47,	// zsub1_then_ssub -> ZPR2
      47,	// zsub1_then_zsub -> ZPR2
      47,	// zsub1_then_zsub_hi -> ZPR2
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      47,	// dsub_zsub1_then_dsub -> ZPR2
      47,	// zsub_zsub1_then_zsub -> ZPR2
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQ_with_qsub0_in_FPR128_lo
      48,	// bsub -> QQ_with_qsub0_in_FPR128_lo
      48,	// dsub -> QQ_with_qsub0_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      48,	// hsub -> QQ_with_qsub0_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      48,	// qsub0 -> QQ_with_qsub0_in_FPR128_lo
      48,	// qsub1 -> QQ_with_qsub0_in_FPR128_lo
      0,	// qsub2
      0,	// qsub3
      48,	// ssub -> QQ_with_qsub0_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      48,	// qsub1_then_bsub -> QQ_with_qsub0_in_FPR128_lo
      48,	// qsub1_then_dsub -> QQ_with_qsub0_in_FPR128_lo
      48,	// qsub1_then_hsub -> QQ_with_qsub0_in_FPR128_lo
      48,	// qsub1_then_ssub -> QQ_with_qsub0_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      48,	// dsub_qsub1_then_dsub -> QQ_with_qsub0_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQ_with_qsub1_in_FPR128_lo
      49,	// bsub -> QQ_with_qsub1_in_FPR128_lo
      49,	// dsub -> QQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      49,	// hsub -> QQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      49,	// qsub0 -> QQ_with_qsub1_in_FPR128_lo
      49,	// qsub1 -> QQ_with_qsub1_in_FPR128_lo
      0,	// qsub2
      0,	// qsub3
      49,	// ssub -> QQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      49,	// qsub1_then_bsub -> QQ_with_qsub1_in_FPR128_lo
      49,	// qsub1_then_dsub -> QQ_with_qsub1_in_FPR128_lo
      49,	// qsub1_then_hsub -> QQ_with_qsub1_in_FPR128_lo
      49,	// qsub1_then_ssub -> QQ_with_qsub1_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      49,	// dsub_qsub1_then_dsub -> QQ_with_qsub1_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub1_in_ZPR_4b
      50,	// bsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// dsub -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      50,	// hsub -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      50,	// ssub -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      50,	// zsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub0 -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1 -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// zsub2
      0,	// zsub3
      50,	// zsub_hi -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      50,	// zsub1_then_bsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1_then_dsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1_then_hsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1_then_ssub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1_then_zsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub1_then_zsub_hi -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      50,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub1_in_ZPR_4b
      50,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub_in_FPR128_lo
      51,	// bsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// dsub -> ZPR2_with_zsub_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      51,	// hsub -> ZPR2_with_zsub_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      51,	// ssub -> ZPR2_with_zsub_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      51,	// zsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub0 -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1 -> ZPR2_with_zsub_in_FPR128_lo
      0,	// zsub2
      0,	// zsub3
      51,	// zsub_hi -> ZPR2_with_zsub_in_FPR128_lo
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      51,	// zsub1_then_bsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1_then_hsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1_then_ssub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub1_then_zsub_hi -> ZPR2_with_zsub_in_FPR128_lo
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      51,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo
      51,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// bsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// dsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      52,	// hsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      52,	// qsub0 -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// qsub1 -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// qsub2
      0,	// qsub3
      52,	// ssub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      52,	// qsub1_then_bsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// qsub1_then_dsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// qsub1_then_hsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      52,	// qsub1_then_ssub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      52,	// dsub_qsub1_then_dsub -> QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// bsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      53,	// hsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      53,	// ssub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      53,	// zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub0 -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1 -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// zsub2
      0,	// zsub3
      53,	// zsub_hi -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      53,	// zsub1_then_bsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1_then_hsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1_then_ssub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub1_then_zsub_hi -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      53,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      53,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub0_in_ZPR_3b
      54,	// bsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// dsub -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      54,	// hsub -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      54,	// ssub -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      54,	// zsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub0 -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1 -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// zsub2
      0,	// zsub3
      54,	// zsub_hi -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      54,	// zsub1_then_bsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1_then_dsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1_then_hsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1_then_ssub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1_then_zsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub1_then_zsub_hi -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      54,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub0_in_ZPR_3b
      54,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub0_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub1_in_ZPR_3b
      55,	// bsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// dsub -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      55,	// hsub -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      55,	// ssub -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      55,	// zsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub0 -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1 -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// zsub2
      0,	// zsub3
      55,	// zsub_hi -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      55,	// zsub1_then_bsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1_then_dsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1_then_hsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1_then_ssub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1_then_zsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub1_then_zsub_hi -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      55,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub1_in_ZPR_3b
      55,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// bsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      56,	// hsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      56,	// ssub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      56,	// zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub0 -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1 -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// zsub2
      0,	// zsub3
      56,	// zsub_hi -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      56,	// zsub1_then_bsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1_then_hsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1_then_ssub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub1_then_zsub_hi -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      56,	// dsub_zsub1_then_dsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      56,	// zsub_zsub1_then_zsub -> ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ
      57,	// bsub -> QQQ
      57,	// dsub -> QQQ
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      57,	// hsub -> QQQ
      0,	// qhisub
      0,	// qsub
      57,	// qsub0 -> QQQ
      57,	// qsub1 -> QQQ
      57,	// qsub2 -> QQQ
      0,	// qsub3
      57,	// ssub -> QQQ
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      57,	// qsub1_then_bsub -> QQQ
      57,	// qsub1_then_dsub -> QQQ
      57,	// qsub1_then_hsub -> QQQ
      57,	// qsub1_then_ssub -> QQQ
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      57,	// qsub2_then_bsub -> QQQ
      57,	// qsub2_then_dsub -> QQQ
      57,	// qsub2_then_hsub -> QQQ
      57,	// qsub2_then_ssub -> QQQ
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      57,	// dsub_qsub1_then_dsub -> QQQ
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      57,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ
      57,	// qsub0_qsub1 -> QQQ
      0,	// qsub0_qsub1_qsub2
      57,	// qsub1_qsub2 -> QQQ
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      57,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3
      58,	// bsub -> ZPR3
      58,	// dsub -> ZPR3
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      58,	// hsub -> ZPR3
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      58,	// ssub -> ZPR3
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      58,	// zsub -> ZPR3
      58,	// zsub0 -> ZPR3
      58,	// zsub1 -> ZPR3
      58,	// zsub2 -> ZPR3
      0,	// zsub3
      58,	// zsub_hi -> ZPR3
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      58,	// zsub1_then_bsub -> ZPR3
      58,	// zsub1_then_dsub -> ZPR3
      58,	// zsub1_then_hsub -> ZPR3
      58,	// zsub1_then_ssub -> ZPR3
      58,	// zsub1_then_zsub -> ZPR3
      58,	// zsub1_then_zsub_hi -> ZPR3
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      58,	// zsub2_then_bsub -> ZPR3
      58,	// zsub2_then_dsub -> ZPR3
      58,	// zsub2_then_hsub -> ZPR3
      58,	// zsub2_then_ssub -> ZPR3
      58,	// zsub2_then_zsub -> ZPR3
      58,	// zsub2_then_zsub_hi -> ZPR3
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      58,	// dsub_zsub1_then_dsub -> ZPR3
      58,	// zsub_zsub1_then_zsub -> ZPR3
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      58,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      58,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3
      58,	// zsub0_zsub1 -> ZPR3
      0,	// zsub0_zsub1_zsub2
      58,	// zsub1_zsub2 -> ZPR3
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      58,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      58,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub0_in_FPR128_lo
      59,	// bsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// dsub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      59,	// hsub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      59,	// qsub0 -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub1 -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub2 -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qsub3
      59,	// ssub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      59,	// qsub1_then_bsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub1_then_hsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub1_then_ssub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      59,	// qsub2_then_bsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub2_then_hsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub2_then_ssub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      59,	// dsub_qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      59,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo
      59,	// qsub0_qsub1 -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      59,	// qsub1_qsub2 -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      59,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub1_in_FPR128_lo
      60,	// bsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// dsub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      60,	// hsub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      60,	// qsub0 -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub1 -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub2 -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub3
      60,	// ssub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      60,	// qsub1_then_bsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub1_then_dsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub1_then_hsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub1_then_ssub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      60,	// qsub2_then_bsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub2_then_hsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub2_then_ssub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      60,	// dsub_qsub1_then_dsub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      60,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo
      60,	// qsub0_qsub1 -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      60,	// qsub1_qsub2 -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      60,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub2_in_FPR128_lo
      61,	// bsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// dsub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      61,	// hsub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      61,	// qsub0 -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub1 -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub2 -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3
      61,	// ssub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      61,	// qsub1_then_bsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub1_then_dsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub1_then_hsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub1_then_ssub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      61,	// qsub2_then_bsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub2_then_dsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub2_then_hsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub2_then_ssub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      61,	// dsub_qsub1_then_dsub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      61,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub2_in_FPR128_lo
      61,	// qsub0_qsub1 -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      61,	// qsub1_qsub2 -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      61,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub1_in_ZPR_4b
      62,	// bsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// dsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      62,	// hsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      62,	// ssub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      62,	// zsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub0 -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1 -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2 -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub3
      62,	// zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      62,	// zsub1_then_bsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1_then_hsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1_then_ssub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub1_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      62,	// zsub2_then_bsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2_then_hsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2_then_ssub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub2_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      62,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      62,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      62,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b
      62,	// zsub0_zsub1 -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub0_zsub1_zsub2
      62,	// zsub1_zsub2 -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      62,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      62,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub2_in_ZPR_4b
      63,	// bsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// dsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      63,	// hsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      63,	// ssub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      63,	// zsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub0 -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1 -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2 -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3
      63,	// zsub_hi -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      63,	// zsub1_then_bsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1_then_dsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1_then_hsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1_then_ssub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1_then_zsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub1_then_zsub_hi -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      63,	// zsub2_then_bsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2_then_hsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2_then_ssub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub2_then_zsub_hi -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      63,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      63,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      63,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_4b
      63,	// zsub0_zsub1 -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub0_zsub1_zsub2
      63,	// zsub1_zsub2 -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      63,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      63,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub_in_FPR128_lo
      64,	// bsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// dsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      64,	// hsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      64,	// ssub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      64,	// zsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub0 -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1 -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2 -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub3
      64,	// zsub_hi -> ZPR3_with_zsub_in_FPR128_lo
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      64,	// zsub1_then_bsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1_then_hsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1_then_ssub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub1_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      64,	// zsub2_then_bsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2_then_hsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2_then_ssub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub2_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      64,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      64,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      64,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo
      64,	// zsub0_zsub1 -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub0_zsub1_zsub2
      64,	// zsub1_zsub2 -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      64,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      64,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      65,	// hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      65,	// qsub0 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub1 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub2 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub3
      65,	// ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      65,	// qsub1_then_bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub1_then_hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub1_then_ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      65,	// qsub2_then_bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub2_then_hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub2_then_ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      65,	// dsub_qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      65,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      65,	// qsub0_qsub1 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      65,	// qsub1_qsub2 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      65,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// bsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      66,	// hsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      66,	// qsub0 -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub1 -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub2 -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3
      66,	// ssub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      66,	// qsub1_then_bsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub1_then_dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub1_then_hsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub1_then_ssub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      66,	// qsub2_then_bsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub2_then_hsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub2_then_ssub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      66,	// dsub_qsub1_then_dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      66,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      66,	// qsub0_qsub1 -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      66,	// qsub1_qsub2 -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      66,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// bsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      67,	// hsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      67,	// ssub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      67,	// zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub0 -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1 -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2 -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3
      67,	// zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      67,	// zsub1_then_bsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1_then_hsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1_then_ssub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub1_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      67,	// zsub2_then_bsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2_then_hsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2_then_ssub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub2_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      67,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      67,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      67,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      67,	// zsub0_zsub1 -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub0_zsub1_zsub2
      67,	// zsub1_zsub2 -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      67,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      67,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      68,	// hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      68,	// ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      68,	// zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub0 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub3
      68,	// zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      68,	// zsub1_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub1_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      68,	// zsub2_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub2_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      68,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      68,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      68,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      68,	// zsub0_zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub0_zsub1_zsub2
      68,	// zsub1_zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      68,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      68,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      69,	// hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      69,	// qsub0 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub1 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub2 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3
      69,	// ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      69,	// qsub1_then_bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub1_then_hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub1_then_ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      69,	// qsub2_then_bsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub2_then_hsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub2_then_ssub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      69,	// dsub_qsub1_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      69,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      69,	// qsub0_qsub1 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub0_qsub1_qsub2
      69,	// qsub1_qsub2 -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      69,	// qsub1_then_dsub_qsub2_then_dsub -> QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      70,	// hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      70,	// ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      70,	// zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub0 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3
      70,	// zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      70,	// zsub1_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub1_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      70,	// zsub2_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub2_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      70,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      70,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      70,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      70,	// zsub0_zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub0_zsub1_zsub2
      70,	// zsub1_zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      70,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      70,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub0_in_ZPR_3b
      71,	// bsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// dsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      71,	// hsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      71,	// ssub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      71,	// zsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub0 -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1 -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2 -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub3
      71,	// zsub_hi -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      71,	// zsub1_then_bsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1_then_dsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1_then_hsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1_then_ssub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1_then_zsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub1_then_zsub_hi -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      71,	// zsub2_then_bsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2_then_dsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2_then_hsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2_then_ssub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2_then_zsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub2_then_zsub_hi -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      71,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      71,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      71,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub0_in_ZPR_3b
      71,	// zsub0_zsub1 -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      71,	// zsub1_zsub2 -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      71,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      71,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub0_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub1_in_ZPR_3b
      72,	// bsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// dsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      72,	// hsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      72,	// ssub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      72,	// zsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub0 -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1 -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2 -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub3
      72,	// zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      72,	// zsub1_then_bsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1_then_hsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1_then_ssub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub1_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      72,	// zsub2_then_bsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2_then_hsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2_then_ssub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub2_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      72,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      72,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      72,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b
      72,	// zsub0_zsub1 -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      72,	// zsub1_zsub2 -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      72,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      72,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub2_in_ZPR_3b
      73,	// bsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// dsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      73,	// hsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      73,	// ssub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      73,	// zsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub0 -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1 -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2 -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3
      73,	// zsub_hi -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      73,	// zsub1_then_bsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1_then_dsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1_then_hsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1_then_ssub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1_then_zsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub1_then_zsub_hi -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      73,	// zsub2_then_bsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2_then_hsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2_then_ssub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub2_then_zsub_hi -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      73,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      73,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      73,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_3b
      73,	// zsub0_zsub1 -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      73,	// zsub1_zsub2 -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      73,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      73,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// bsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      74,	// hsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      74,	// ssub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      74,	// zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub0 -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1 -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2 -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3
      74,	// zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      74,	// zsub1_then_bsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1_then_hsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1_then_ssub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub1_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      74,	// zsub2_then_bsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2_then_hsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2_then_ssub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub2_then_zsub_hi -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      74,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      74,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      74,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      74,	// zsub0_zsub1 -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      74,	// zsub1_zsub2 -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      74,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      74,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      75,	// hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      75,	// ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      75,	// zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub0 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub3
      75,	// zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      75,	// zsub1_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub1_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      75,	// zsub2_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub2_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      75,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      75,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      75,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      75,	// zsub0_zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      75,	// zsub1_zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      75,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      75,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      76,	// hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      76,	// ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      76,	// zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub0 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3
      76,	// zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      76,	// zsub1_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub1_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      76,	// zsub2_then_bsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2_then_hsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2_then_ssub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub2_then_zsub_hi -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      76,	// dsub_zsub1_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub_zsub1_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      76,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      76,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      76,	// zsub0_zsub1 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub0_zsub1_zsub2
      76,	// zsub1_zsub2 -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      76,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      76,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ
      77,	// bsub -> QQQQ
      77,	// dsub -> QQQQ
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      77,	// hsub -> QQQQ
      0,	// qhisub
      0,	// qsub
      77,	// qsub0 -> QQQQ
      77,	// qsub1 -> QQQQ
      77,	// qsub2 -> QQQQ
      77,	// qsub3 -> QQQQ
      77,	// ssub -> QQQQ
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      77,	// qsub1_then_bsub -> QQQQ
      77,	// qsub1_then_dsub -> QQQQ
      77,	// qsub1_then_hsub -> QQQQ
      77,	// qsub1_then_ssub -> QQQQ
      77,	// qsub3_then_bsub -> QQQQ
      77,	// qsub3_then_dsub -> QQQQ
      77,	// qsub3_then_hsub -> QQQQ
      77,	// qsub3_then_ssub -> QQQQ
      77,	// qsub2_then_bsub -> QQQQ
      77,	// qsub2_then_dsub -> QQQQ
      77,	// qsub2_then_hsub -> QQQQ
      77,	// qsub2_then_ssub -> QQQQ
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      77,	// dsub_qsub1_then_dsub -> QQQQ
      77,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ
      77,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ
      77,	// qsub0_qsub1 -> QQQQ
      77,	// qsub0_qsub1_qsub2 -> QQQQ
      77,	// qsub1_qsub2 -> QQQQ
      77,	// qsub1_qsub2_qsub3 -> QQQQ
      77,	// qsub2_qsub3 -> QQQQ
      77,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ
      77,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ
      77,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR4
      78,	// bsub -> ZPR4
      78,	// dsub -> ZPR4
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      78,	// hsub -> ZPR4
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      78,	// ssub -> ZPR4
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      78,	// zsub -> ZPR4
      78,	// zsub0 -> ZPR4
      78,	// zsub1 -> ZPR4
      78,	// zsub2 -> ZPR4
      78,	// zsub3 -> ZPR4
      78,	// zsub_hi -> ZPR4
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      78,	// zsub1_then_bsub -> ZPR4
      78,	// zsub1_then_dsub -> ZPR4
      78,	// zsub1_then_hsub -> ZPR4
      78,	// zsub1_then_ssub -> ZPR4
      78,	// zsub1_then_zsub -> ZPR4
      78,	// zsub1_then_zsub_hi -> ZPR4
      78,	// zsub3_then_bsub -> ZPR4
      78,	// zsub3_then_dsub -> ZPR4
      78,	// zsub3_then_hsub -> ZPR4
      78,	// zsub3_then_ssub -> ZPR4
      78,	// zsub3_then_zsub -> ZPR4
      78,	// zsub3_then_zsub_hi -> ZPR4
      78,	// zsub2_then_bsub -> ZPR4
      78,	// zsub2_then_dsub -> ZPR4
      78,	// zsub2_then_hsub -> ZPR4
      78,	// zsub2_then_ssub -> ZPR4
      78,	// zsub2_then_zsub -> ZPR4
      78,	// zsub2_then_zsub_hi -> ZPR4
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      78,	// dsub_zsub1_then_dsub -> ZPR4
      78,	// zsub_zsub1_then_zsub -> ZPR4
      78,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4
      78,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4
      78,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4
      78,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4
      78,	// zsub0_zsub1 -> ZPR4
      78,	// zsub0_zsub1_zsub2 -> ZPR4
      78,	// zsub1_zsub2 -> ZPR4
      78,	// zsub1_zsub2_zsub3 -> ZPR4
      78,	// zsub2_zsub3 -> ZPR4
      78,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4
      78,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4
      78,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4
      78,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4
      78,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4
      78,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4
    },
    {	// QQQQ_with_qsub0_in_FPR128_lo
      79,	// bsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// dsub -> QQQQ_with_qsub0_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      79,	// hsub -> QQQQ_with_qsub0_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      79,	// qsub0 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub3 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// ssub -> QQQQ_with_qsub0_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      79,	// qsub1_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub3_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub3_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub3_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      79,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub0_qsub1 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      79,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub1_in_FPR128_lo
      80,	// bsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// dsub -> QQQQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      80,	// hsub -> QQQQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      80,	// qsub0 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub3 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// ssub -> QQQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      80,	// qsub1_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub3_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub3_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub3_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      80,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub0_qsub1 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      80,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub2_in_FPR128_lo
      81,	// bsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// dsub -> QQQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      81,	// hsub -> QQQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      81,	// qsub0 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub3 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// ssub -> QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      81,	// qsub1_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub3_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub3_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub3_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      81,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub0_qsub1 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_qsub2 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_qsub3 -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      81,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub3_in_FPR128_lo
      82,	// bsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// dsub -> QQQQ_with_qsub3_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      82,	// hsub -> QQQQ_with_qsub3_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      82,	// qsub0 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub3 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// ssub -> QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      82,	// qsub1_then_bsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_then_hsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_then_ssub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub3_then_bsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub3_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub3_then_hsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub3_then_ssub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_then_bsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_then_hsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_then_ssub -> QQQQ_with_qsub3_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      82,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub0_qsub1 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_qsub2 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_qsub3 -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      82,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR4_with_zsub1_in_ZPR_4b
      83,	// bsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// dsub -> ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      83,	// hsub -> ZPR4_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      83,	// ssub -> ZPR4_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      83,	// zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      83,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      83,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b
      83,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b
    },
    {	// ZPR4_with_zsub2_in_ZPR_4b
      84,	// bsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// dsub -> ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      84,	// hsub -> ZPR4_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      84,	// ssub -> ZPR4_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      84,	// zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub0 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      84,	// zsub1_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub3_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      84,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub0_zsub1 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b
      84,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b
    },
    {	// ZPR4_with_zsub3_in_ZPR_4b
      85,	// bsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// dsub -> ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      85,	// hsub -> ZPR4_with_zsub3_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      85,	// ssub -> ZPR4_with_zsub3_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      85,	// zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub0 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub_hi -> ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      85,	// zsub1_then_bsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_hsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_ssub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_bsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_hsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_ssub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub3_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_bsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_hsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_ssub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      85,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub0_zsub1 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_zsub2 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_zsub3 -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_4b
      85,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_4b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo
      86,	// bsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// dsub -> ZPR4_with_zsub_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      86,	// hsub -> ZPR4_with_zsub_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      86,	// ssub -> ZPR4_with_zsub_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      86,	// zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      86,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      86,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo
      86,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo
    },
    {	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      87,	// hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      87,	// qsub0 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      87,	// qsub1_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub3_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub3_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub3_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      87,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub0_qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      87,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      88,	// hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      88,	// qsub0 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      88,	// qsub1_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub3_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub3_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub3_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      88,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub0_qsub1 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      88,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// bsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      89,	// hsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      89,	// qsub0 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub3 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// ssub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      89,	// qsub1_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub3_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub3_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub3_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_then_bsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_then_hsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_then_ssub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      89,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub0_qsub1 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_qsub2 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_qsub3 -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      89,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      90,	// hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      90,	// ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      90,	// zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      90,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      90,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
      90,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
    },
    {	// ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// bsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      91,	// hsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      91,	// ssub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      91,	// zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub0 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      91,	// zsub1_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub3_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_bsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_hsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_ssub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      91,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub0_zsub1 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      91,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      92,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      92,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      92,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      92,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      92,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
      92,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
    },
    {	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      93,	// hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      93,	// qsub0 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      93,	// qsub1_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub3_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub3_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub3_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      93,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub0_qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      93,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      94,	// hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      94,	// qsub0 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      94,	// qsub1_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub3_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub3_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub3_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_then_bsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_then_hsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_then_ssub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      94,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub0_qsub1 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_qsub2 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_qsub3 -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      94,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      95,	// hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      95,	// ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      95,	// zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      95,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      95,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
      95,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      96,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      96,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      96,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      96,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      96,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
      96,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
    },
    {	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      97,	// hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// qhisub
      0,	// qsub
      97,	// qsub0 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      0,	// zsub
      0,	// zsub0
      0,	// zsub1
      0,	// zsub2
      0,	// zsub3
      0,	// zsub_hi
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      97,	// qsub1_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub3_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub3_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub3_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_then_bsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_then_hsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_then_ssub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// subo64_then_sub_32
      0,	// zsub1_then_bsub
      0,	// zsub1_then_dsub
      0,	// zsub1_then_hsub
      0,	// zsub1_then_ssub
      0,	// zsub1_then_zsub
      0,	// zsub1_then_zsub_hi
      0,	// zsub3_then_bsub
      0,	// zsub3_then_dsub
      0,	// zsub3_then_hsub
      0,	// zsub3_then_ssub
      0,	// zsub3_then_zsub
      0,	// zsub3_then_zsub_hi
      0,	// zsub2_then_bsub
      0,	// zsub2_then_dsub
      0,	// zsub2_then_hsub
      0,	// zsub2_then_ssub
      0,	// zsub2_then_zsub
      0,	// zsub2_then_zsub_hi
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      97,	// dsub_qsub1_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// dsub_qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub0_qsub1 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub0_qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_qsub2 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_qsub3 -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_then_dsub_qsub2_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      97,	// qsub2_then_dsub_qsub3_then_dsub -> QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
      0,	// sub_32_subo64_then_sub_32
      0,	// dsub_zsub1_then_dsub
      0,	// zsub_zsub1_then_zsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// dsub_zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub_zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub0_zsub1
      0,	// zsub0_zsub1_zsub2
      0,	// zsub1_zsub2
      0,	// zsub1_zsub2_zsub3
      0,	// zsub2_zsub3
      0,	// zsub1_then_dsub_zsub2_then_dsub
      0,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub1_then_zsub_zsub2_then_zsub
      0,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub
      0,	// zsub2_then_dsub_zsub3_then_dsub
      0,	// zsub2_then_zsub_zsub3_then_zsub
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      98,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      98,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      98,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      98,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      98,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
      98,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
    },
    {	// ZPR4_with_zsub0_in_ZPR_3b
      99,	// bsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// dsub -> ZPR4_with_zsub0_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      99,	// hsub -> ZPR4_with_zsub0_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      99,	// ssub -> ZPR4_with_zsub0_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      99,	// zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub0 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub_hi -> ZPR4_with_zsub0_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      99,	// zsub1_then_bsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_hsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_ssub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_zsub_hi -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_bsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_hsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_ssub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub3_then_zsub_hi -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_bsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_hsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_ssub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_zsub_hi -> ZPR4_with_zsub0_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      99,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub0_zsub1 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_zsub2 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_zsub3 -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub0_in_ZPR_3b
      99,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub0_in_ZPR_3b
    },
    {	// ZPR4_with_zsub1_in_ZPR_3b
      100,	// bsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// dsub -> ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      100,	// hsub -> ZPR4_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      100,	// ssub -> ZPR4_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      100,	// zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      100,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      100,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b
      100,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b
    },
    {	// ZPR4_with_zsub2_in_ZPR_3b
      101,	// bsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// dsub -> ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      101,	// hsub -> ZPR4_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      101,	// ssub -> ZPR4_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      101,	// zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub0 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      101,	// zsub1_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub3_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      101,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub0_zsub1 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b
      101,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b
    },
    {	// ZPR4_with_zsub3_in_ZPR_3b
      102,	// bsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// dsub -> ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      102,	// hsub -> ZPR4_with_zsub3_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      102,	// ssub -> ZPR4_with_zsub3_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      102,	// zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub0 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub_hi -> ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      102,	// zsub1_then_bsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_hsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_ssub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_bsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_hsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_ssub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub3_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_bsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_hsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_ssub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_zsub_hi -> ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      102,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub0_zsub1 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_zsub2 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_zsub3 -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub3_in_ZPR_3b
      102,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub3_in_ZPR_3b
    },
    {	// ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      103,	// hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      103,	// ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      103,	// zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      103,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      103,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
      103,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
    },
    {	// ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// bsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      104,	// hsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      104,	// ssub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      104,	// zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub0 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      104,	// zsub1_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub3_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_bsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_hsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_ssub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_zsub_hi -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      104,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub0_zsub1 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_zsub2 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_zsub3 -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      104,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      105,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      105,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      105,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      105,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      105,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
      105,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
    },
    {	// ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      106,	// hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      106,	// ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      106,	// zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub0 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      106,	// zsub1_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub3_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_bsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_hsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_ssub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_zsub_hi -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      106,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub0_zsub1 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_zsub2 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_zsub3 -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
      106,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      107,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      107,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      107,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      107,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      107,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
      107,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
    },
    {	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0
      0,	// dsub1
      0,	// dsub2
      0,	// dsub3
      108,	// hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// qhisub
      0,	// qsub
      0,	// qsub0
      0,	// qsub1
      0,	// qsub2
      0,	// qsub3
      108,	// ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// sub_32
      0,	// sube32
      0,	// sube64
      0,	// subo32
      0,	// subo64
      108,	// zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub0 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub1_then_bsub
      0,	// dsub1_then_hsub
      0,	// dsub1_then_ssub
      0,	// dsub3_then_bsub
      0,	// dsub3_then_hsub
      0,	// dsub3_then_ssub
      0,	// dsub2_then_bsub
      0,	// dsub2_then_hsub
      0,	// dsub2_then_ssub
      0,	// qsub1_then_bsub
      0,	// qsub1_then_dsub
      0,	// qsub1_then_hsub
      0,	// qsub1_then_ssub
      0,	// qsub3_then_bsub
      0,	// qsub3_then_dsub
      0,	// qsub3_then_hsub
      0,	// qsub3_then_ssub
      0,	// qsub2_then_bsub
      0,	// qsub2_then_dsub
      0,	// qsub2_then_hsub
      0,	// qsub2_then_ssub
      0,	// subo64_then_sub_32
      108,	// zsub1_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub3_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_bsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_hsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_ssub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_zsub_hi -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      0,	// dsub0_dsub1
      0,	// dsub0_dsub1_dsub2
      0,	// dsub1_dsub2
      0,	// dsub1_dsub2_dsub3
      0,	// dsub2_dsub3
      0,	// dsub_qsub1_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// dsub_qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub0_qsub1
      0,	// qsub0_qsub1_qsub2
      0,	// qsub1_qsub2
      0,	// qsub1_qsub2_qsub3
      0,	// qsub2_qsub3
      0,	// qsub1_then_dsub_qsub2_then_dsub
      0,	// qsub1_then_dsub_qsub2_then_dsub_qsub3_then_dsub
      0,	// qsub2_then_dsub_qsub3_then_dsub
      0,	// sub_32_subo64_then_sub_32
      108,	// dsub_zsub1_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub_zsub1_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// dsub_zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// dsub_zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub_zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub_zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub0_zsub1 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub0_zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_zsub2 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_zsub3 -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_dsub_zsub2_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_dsub_zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_zsub_zsub2_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub1_then_zsub_zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_dsub_zsub3_then_dsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
      108,	// zsub2_then_zsub_zsub3_then_zsub -> ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
    },
  };
  assert(RC && "Missing regclass");
  if (!Idx) return RC;
  --Idx;
  assert(Idx < 99 && "Bad subreg");
  unsigned TV = Table[RC->getID()][Idx];
  return TV ? getRegClass(TV - 1) : nullptr;
}

/// Get the weight in units of pressure for this register class.
const RegClassWeight &AArch64GenRegisterInfo::
getRegClassWeight(const TargetRegisterClass *RC) const {
  static const RegClassWeight RCWeightTable[] = {
    {1, 32},  	// FPR8
    {1, 32},  	// FPR16
    {1, 16},  	// PPR
    {1, 8},  	// PPR_3b
    {1, 33},  	// GPR32all
    {1, 32},  	// FPR32
    {1, 32},  	// GPR32
    {1, 32},  	// GPR32sp
    {1, 31},  	// GPR32common
    {1, 8},  	// GPR32arg
    {0, 0},  	// CCR
    {1, 1},  	// GPR32sponly
    {2, 32},  	// WSeqPairsClass
    {2, 30},  	// WSeqPairsClass_with_subo32_in_GPR32common
    {2, 8},  	// WSeqPairsClass_with_sube32_in_GPR32arg
    {1, 33},  	// GPR64all
    {1, 32},  	// FPR64
    {1, 32},  	// GPR64
    {1, 32},  	// GPR64sp
    {1, 31},  	// GPR64common
    {1, 29},  	// GPR64noip
    {1, 28},  	// GPR64common_and_GPR64noip
    {1, 19},  	// tcGPR64
    {1, 17},  	// GPR64noip_and_tcGPR64
    {1, 8},  	// GPR64arg
    {1, 2},  	// rtcGPR64
    {1, 1},  	// GPR64sponly
    {2, 32},  	// DD
    {2, 32},  	// XSeqPairsClass
    {2, 30},  	// XSeqPairsClass_with_subo64_in_GPR64common
    {2, 30},  	// XSeqPairsClass_with_subo64_in_GPR64noip
    {2, 28},  	// XSeqPairsClass_with_sube64_in_GPR64noip
    {2, 20},  	// XSeqPairsClass_with_sube64_in_tcGPR64
    {2, 18},  	// XSeqPairsClass_with_sube64_in_GPR64noip_and_tcGPR64
    {2, 18},  	// XSeqPairsClass_with_subo64_in_tcGPR64
    {2, 16},  	// XSeqPairsClass_with_subo64_in_GPR64noip_and_tcGPR64
    {2, 8},  	// XSeqPairsClass_with_sub_32_in_GPR32arg
    {2, 2},  	// XSeqPairsClass_with_sube64_in_rtcGPR64
    {1, 32},  	// FPR128
    {2, 64},  	// ZPR
    {1, 16},  	// FPR128_lo
    {2, 32},  	// ZPR_4b
    {2, 16},  	// ZPR_3b
    {3, 32},  	// DDD
    {4, 32},  	// DDDD
    {2, 32},  	// QQ
    {4, 64},  	// ZPR2
    {2, 17},  	// QQ_with_qsub0_in_FPR128_lo
    {2, 17},  	// QQ_with_qsub1_in_FPR128_lo
    {4, 34},  	// ZPR2_with_zsub1_in_ZPR_4b
    {4, 34},  	// ZPR2_with_zsub_in_FPR128_lo
    {2, 16},  	// QQ_with_qsub0_in_FPR128_lo_and_QQ_with_qsub1_in_FPR128_lo
    {4, 32},  	// ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_4b
    {4, 18},  	// ZPR2_with_zsub0_in_ZPR_3b
    {4, 18},  	// ZPR2_with_zsub1_in_ZPR_3b
    {4, 16},  	// ZPR2_with_zsub_in_FPR128_lo_and_ZPR2_with_zsub1_in_ZPR_3b
    {3, 32},  	// QQQ
    {6, 64},  	// ZPR3
    {3, 18},  	// QQQ_with_qsub0_in_FPR128_lo
    {3, 18},  	// QQQ_with_qsub1_in_FPR128_lo
    {3, 18},  	// QQQ_with_qsub2_in_FPR128_lo
    {6, 36},  	// ZPR3_with_zsub1_in_ZPR_4b
    {6, 36},  	// ZPR3_with_zsub2_in_ZPR_4b
    {6, 36},  	// ZPR3_with_zsub_in_FPR128_lo
    {3, 17},  	// QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub1_in_FPR128_lo
    {3, 17},  	// QQQ_with_qsub1_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
    {6, 34},  	// ZPR3_with_zsub1_in_ZPR_4b_and_ZPR3_with_zsub2_in_ZPR_4b
    {6, 34},  	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_4b
    {3, 16},  	// QQQ_with_qsub0_in_FPR128_lo_and_QQQ_with_qsub2_in_FPR128_lo
    {6, 32},  	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_4b
    {6, 20},  	// ZPR3_with_zsub0_in_ZPR_3b
    {6, 20},  	// ZPR3_with_zsub1_in_ZPR_3b
    {6, 20},  	// ZPR3_with_zsub2_in_ZPR_3b
    {6, 18},  	// ZPR3_with_zsub1_in_ZPR_3b_and_ZPR3_with_zsub2_in_ZPR_3b
    {6, 18},  	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub1_in_ZPR_3b
    {6, 16},  	// ZPR3_with_zsub_in_FPR128_lo_and_ZPR3_with_zsub2_in_ZPR_3b
    {4, 32},  	// QQQQ
    {8, 64},  	// ZPR4
    {4, 19},  	// QQQQ_with_qsub0_in_FPR128_lo
    {4, 19},  	// QQQQ_with_qsub1_in_FPR128_lo
    {4, 19},  	// QQQQ_with_qsub2_in_FPR128_lo
    {4, 19},  	// QQQQ_with_qsub3_in_FPR128_lo
    {8, 38},  	// ZPR4_with_zsub1_in_ZPR_4b
    {8, 38},  	// ZPR4_with_zsub2_in_ZPR_4b
    {8, 38},  	// ZPR4_with_zsub3_in_ZPR_4b
    {8, 38},  	// ZPR4_with_zsub_in_FPR128_lo
    {4, 18},  	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub1_in_FPR128_lo
    {4, 18},  	// QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
    {4, 18},  	// QQQQ_with_qsub2_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
    {8, 36},  	// ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub2_in_ZPR_4b
    {8, 36},  	// ZPR4_with_zsub2_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
    {8, 36},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_4b
    {4, 17},  	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub2_in_FPR128_lo
    {4, 17},  	// QQQQ_with_qsub1_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
    {8, 34},  	// ZPR4_with_zsub1_in_ZPR_4b_and_ZPR4_with_zsub3_in_ZPR_4b
    {8, 34},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_4b
    {4, 16},  	// QQQQ_with_qsub0_in_FPR128_lo_and_QQQQ_with_qsub3_in_FPR128_lo
    {8, 32},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_4b
    {8, 22},  	// ZPR4_with_zsub0_in_ZPR_3b
    {8, 22},  	// ZPR4_with_zsub1_in_ZPR_3b
    {8, 22},  	// ZPR4_with_zsub2_in_ZPR_3b
    {8, 22},  	// ZPR4_with_zsub3_in_ZPR_3b
    {8, 20},  	// ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub2_in_ZPR_3b
    {8, 20},  	// ZPR4_with_zsub2_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
    {8, 20},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub1_in_ZPR_3b
    {8, 18},  	// ZPR4_with_zsub1_in_ZPR_3b_and_ZPR4_with_zsub3_in_ZPR_3b
    {8, 18},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub2_in_ZPR_3b
    {8, 16},  	// ZPR4_with_zsub_in_FPR128_lo_and_ZPR4_with_zsub3_in_ZPR_3b
  };
  return RCWeightTable[RC->getID()];
}

/// Get the weight in units of pressure for this register unit.
unsigned AArch64GenRegisterInfo::
getRegUnitWeight(unsigned RegUnit) const {
  assert(RegUnit < 115 && "invalid register unit");
  // All register units have unit weight.
  return 1;
}


// Get the number of dimensions of register pressure.
unsigned AArch64GenRegisterInfo::getNumRegPressureSets() const {
  return 32;
}

// Get the name of this register unit pressure set.
const char *AArch64GenRegisterInfo::
getRegPressureSetName(unsigned Idx) const {
  static const char *const PressureNameTable[] = {
    "GPR32sponly",
    "rtcGPR64",
    "PPR_3b",
    "GPR32arg",
    "PPR",
    "tcGPR64",
    "FPR128_lo",
    "ZPR_3b",
    "FPR128_lo+ZPR_3b",
    "QQ_with_qsub1_in_FPR128_lo+ZPR_3b",
    "QQQ_with_qsub2_in_FPR128_lo+ZPR_3b",
    "QQQ_with_qsub2_in_FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b",
    "QQQQ_with_qsub3_in_FPR128_lo+ZPR_3b",
    "QQQQ_with_qsub3_in_FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b",
    "QQQQ_with_qsub3_in_FPR128_lo+ZPR4_with_zsub2_in_ZPR_3b",
    "FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b",
    "FPR8",
    "FPR128_lo+ZPR4_with_zsub2_in_ZPR_3b",
    "GPR32",
    "FPR128_lo+ZPR4_with_zsub3_in_ZPR_3b",
    "ZPR4_with_zsub3_in_ZPR_4b",
    "ZPR4_with_zsub_in_FPR128_lo",
    "FPR8+ZPR_3b",
    "FPR8+ZPR4_with_zsub1_in_ZPR_3b",
    "FPR8+ZPR4_with_zsub2_in_ZPR_3b",
    "FPR8+ZPR4_with_zsub3_in_ZPR_3b",
    "ZPR_4b",
    "FPR8+ZPR_4b",
    "FPR8+ZPR4_with_zsub2_in_ZPR_4b",
    "FPR8+ZPR4_with_zsub3_in_ZPR_4b",
    "FPR8+ZPR4_with_zsub_in_FPR128_lo",
    "ZPR",
  };
  return PressureNameTable[Idx];
}

// Get the register unit pressure limit for this dimension.
// This limit must be adjusted dynamically for reserved registers.
unsigned AArch64GenRegisterInfo::
getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const {
  static const uint8_t PressureLimitTable[] = {
    1,  	// 0: GPR32sponly
    2,  	// 1: rtcGPR64
    8,  	// 2: PPR_3b
    8,  	// 3: GPR32arg
    16,  	// 4: PPR
    20,  	// 5: tcGPR64
    22,  	// 6: FPR128_lo
    28,  	// 7: ZPR_3b
    30,  	// 8: FPR128_lo+ZPR_3b
    30,  	// 9: QQ_with_qsub1_in_FPR128_lo+ZPR_3b
    30,  	// 10: QQQ_with_qsub2_in_FPR128_lo+ZPR_3b
    30,  	// 11: QQQ_with_qsub2_in_FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b
    30,  	// 12: QQQQ_with_qsub3_in_FPR128_lo+ZPR_3b
    30,  	// 13: QQQQ_with_qsub3_in_FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b
    30,  	// 14: QQQQ_with_qsub3_in_FPR128_lo+ZPR4_with_zsub2_in_ZPR_3b
    31,  	// 15: FPR128_lo+ZPR4_with_zsub1_in_ZPR_3b
    32,  	// 16: FPR8
    32,  	// 17: FPR128_lo+ZPR4_with_zsub2_in_ZPR_3b
    33,  	// 18: GPR32
    33,  	// 19: FPR128_lo+ZPR4_with_zsub3_in_ZPR_3b
    41,  	// 20: ZPR4_with_zsub3_in_ZPR_4b
    41,  	// 21: ZPR4_with_zsub_in_FPR128_lo
    43,  	// 22: FPR8+ZPR_3b
    43,  	// 23: FPR8+ZPR4_with_zsub1_in_ZPR_3b
    43,  	// 24: FPR8+ZPR4_with_zsub2_in_ZPR_3b
    43,  	// 25: FPR8+ZPR4_with_zsub3_in_ZPR_3b
    44,  	// 26: ZPR_4b
    51,  	// 27: FPR8+ZPR_4b
    51,  	// 28: FPR8+ZPR4_with_zsub2_in_ZPR_4b
    51,  	// 29: FPR8+ZPR4_with_zsub3_in_ZPR_4b
    51,  	// 30: FPR8+ZPR4_with_zsub_in_FPR128_lo
    64,  	// 31: ZPR
  };
  return PressureLimitTable[Idx];
}

/// Table of pressure sets per register class or unit.
static const int RCSetsTable[] = {
  /* 0 */ 2, 4, -1,
  /* 3 */ 0, 18, -1,
  /* 6 */ 1, 5, 18, -1,
  /* 10 */ 3, 5, 18, -1,
  /* 14 */ 26, 27, 31, -1,
  /* 18 */ 26, 28, 31, -1,
  /* 22 */ 26, 27, 28, 31, -1,
  /* 27 */ 20, 26, 29, 31, -1,
  /* 32 */ 7, 19, 20, 25, 26, 29, 31, -1,
  /* 40 */ 20, 26, 28, 29, 31, -1,
  /* 46 */ 7, 14, 17, 20, 24, 26, 28, 29, 31, -1,
  /* 56 */ 7, 14, 17, 19, 20, 24, 25, 26, 28, 29, 31, -1,
  /* 68 */ 20, 26, 27, 28, 29, 31, -1,
  /* 75 */ 7, 11, 13, 15, 20, 23, 26, 27, 28, 29, 31, -1,
  /* 87 */ 7, 11, 13, 14, 15, 17, 20, 23, 24, 26, 27, 28, 29, 31, -1,
  /* 102 */ 7, 11, 13, 14, 15, 17, 19, 20, 23, 24, 25, 26, 27, 28, 29, 31, -1,
  /* 119 */ 21, 26, 30, 31, -1,
  /* 124 */ 21, 26, 27, 30, 31, -1,
  /* 130 */ 21, 26, 27, 28, 30, 31, -1,
  /* 137 */ 16, 22, 23, 24, 25, 27, 28, 29, 30, 31, -1,
  /* 148 */ 20, 21, 26, 27, 28, 29, 30, 31, -1,
  /* 157 */ 7, 8, 9, 10, 12, 20, 21, 22, 26, 27, 28, 29, 30, 31, -1,
  /* 172 */ 7, 8, 9, 10, 11, 12, 13, 15, 20, 21, 22, 23, 26, 27, 28, 29, 30, 31, -1,
  /* 191 */ 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31, -1,
  /* 213 */ 6, 12, 13, 14, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 232 */ 6, 7, 12, 13, 14, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 252 */ 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 276 */ 6, 10, 11, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 295 */ 6, 10, 11, 12, 13, 14, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 317 */ 6, 7, 10, 11, 12, 13, 14, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 340 */ 6, 8, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 359 */ 6, 9, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 378 */ 6, 8, 9, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 398 */ 6, 9, 10, 11, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 419 */ 6, 8, 9, 10, 11, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 441 */ 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 465 */ 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 490 */ 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  /* 515 */ 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
};

/// Get the dimensions of register pressure impacted by this register class.
/// Returns a -1 terminated array of pressure set IDs
const int* AArch64GenRegisterInfo::
getRegClassPressureSets(const TargetRegisterClass *RC) const {
  static const uint16_t RCSetStartTable[] = {
    137,137,1,0,4,137,4,4,4,10,2,3,4,4,10,4,137,4,4,4,4,4,7,7,10,6,3,137,4,4,4,4,7,7,7,7,10,6,137,16,490,148,252,137,137,137,16,419,441,68,130,490,148,191,102,252,137,16,378,398,295,22,40,124,419,441,68,130,490,148,172,87,56,102,191,252,137,16,340,359,276,213,14,18,27,119,378,398,295,22,40,124,419,441,68,130,490,148,157,75,46,32,87,56,172,102,191,252,};
  return &RCSetsTable[RCSetStartTable[RC->getID()]];
}

/// Get the dimensions of register pressure impacted by this register unit.
/// Returns a -1 terminated array of pressure set IDs
const int* AArch64GenRegisterInfo::
getRegUnitPressureSets(unsigned RegUnit) const {
  assert(RegUnit < 115 && "invalid register unit");
  static const uint16_t RUSetStartTable[] = {
    2,4,4,2,3,4,515,515,515,515,515,515,515,515,515,515,515,490,490,490,490,490,419,378,340,137,137,137,137,137,137,137,137,137,137,232,317,465,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,10,10,10,10,10,10,10,10,7,7,7,7,7,7,7,7,6,6,7,7,4,4,4,4,4,4,4,4,4,252,252,252,252,252,252,252,252,191,172,157,148,148,148,148,148,130,124,119,16,16,16,16,16,16,16,16,16,16,32,56,102,};
  return &RCSetsTable[RUSetStartTable[RegUnit]];
}

extern const MCRegisterDesc AArch64RegDesc[];
extern const MCPhysReg AArch64RegDiffLists[];
extern const LaneBitmask AArch64LaneMaskLists[];
extern const char AArch64RegStrings[];
extern const char AArch64RegClassStrings[];
extern const MCPhysReg AArch64RegUnitRoots[][2];
extern const uint16_t AArch64SubRegIdxLists[];
extern const MCRegisterInfo::SubRegCoveredBits AArch64SubRegIdxRanges[];
extern const uint16_t AArch64RegEncodingTable[];
// AArch64 Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair AArch64DwarfFlavour0Dwarf2L[];
extern const unsigned AArch64DwarfFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64EHFlavour0Dwarf2L[];
extern const unsigned AArch64EHFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64DwarfFlavour0L2Dwarf[];
extern const unsigned AArch64DwarfFlavour0L2DwarfSize;

extern const MCRegisterInfo::DwarfLLVMRegPair AArch64EHFlavour0L2Dwarf[];
extern const unsigned AArch64EHFlavour0L2DwarfSize;

AArch64GenRegisterInfo::
AArch64GenRegisterInfo(unsigned RA, unsigned DwarfFlavour, unsigned EHFlavour,
      unsigned PC, unsigned HwMode)
  : TargetRegisterInfo(AArch64RegInfoDesc, RegisterClasses, RegisterClasses+108,
             SubRegIndexNameTable, SubRegIndexLaneMaskTable,
             LaneBitmask(0xFFFFFFB6), RegClassInfos, HwMode) {
  InitMCRegisterInfo(AArch64RegDesc, 629, RA, PC,
                     AArch64MCRegisterClasses, 108,
                     AArch64RegUnitRoots,
                     115,
                     AArch64RegDiffLists,
                     AArch64LaneMaskLists,
                     AArch64RegStrings,
                     AArch64RegClassStrings,
                     AArch64SubRegIdxLists,
                     100,
                     AArch64SubRegIdxRanges,
                     AArch64RegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(AArch64DwarfFlavour0Dwarf2L, AArch64DwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(AArch64EHFlavour0Dwarf2L, AArch64EHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(AArch64DwarfFlavour0L2Dwarf, AArch64DwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(AArch64EHFlavour0L2Dwarf, AArch64EHFlavour0L2DwarfSize, true);
    break;
  }
}

static const MCPhysReg CSR_AArch64_AAPCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, 0 };
static const uint32_t CSR_AArch64_AAPCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff000, 0x001ff800, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x001e005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAPCS_SCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_AAPCS_SCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff800, 0x001ffc00, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x80000000, 0x001f005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAPCS_SwiftError_SaveList[] = { AArch64::LR, AArch64::FP, AArch64::X19, AArch64::X20, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, 0 };
static const uint32_t CSR_AArch64_AAPCS_SwiftError_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00ffb000, 0x001fd800, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x001c005e, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAPCS_SwiftError_SCS_SaveList[] = { AArch64::LR, AArch64::FP, AArch64::X19, AArch64::X20, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_AAPCS_SwiftError_SCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00ffb800, 0x001fdc00, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x80000000, 0x001d005e, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAPCS_ThisReturn_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X0, 0 };
static const uint32_t CSR_AArch64_AAPCS_ThisReturn_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x020001fe, 0x01fff000, 0x001ff800, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x001e005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAVPCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, 0 };
static const uint32_t CSR_AArch64_AAVPCS_RegMask[] = { 0xfffe000c, 0xfffe0001, 0xfffe0001, 0x00000001, 0x0001fffe, 0x0001fffe, 0x00fff000, 0x001ff800, 0x00000000, 0xe0000000, 0xe0000fff, 0xe00003ff, 0xe00007ff, 0xe0000fff, 0xe00003ff, 0x000007ff, 0x001e005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AAVPCS_SCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_AAVPCS_SCS_RegMask[] = { 0xfffe000c, 0xfffe0001, 0xfffe0001, 0x00000001, 0x0001fffe, 0x0001fffe, 0x00fff800, 0x001ffc00, 0x00000000, 0xe0000000, 0xe0000fff, 0xe00003ff, 0xe00007ff, 0xe0000fff, 0xe00003ff, 0x800007ff, 0x001f005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AllRegs_SaveList[] = { AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WSP, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::SP, AArch64::B0, AArch64::B1, AArch64::B2, AArch64::B3, AArch64::B4, AArch64::B5, AArch64::B6, AArch64::B7, AArch64::B8, AArch64::B9, AArch64::B10, AArch64::B11, AArch64::B12, AArch64::B13, AArch64::B14, AArch64::B15, AArch64::B16, AArch64::B17, AArch64::B18, AArch64::B19, AArch64::B20, AArch64::B21, AArch64::B22, AArch64::B23, AArch64::B24, AArch64::B25, AArch64::B26, AArch64::B27, AArch64::B28, AArch64::B29, AArch64::B30, AArch64::B31, AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7, AArch64::H8, AArch64::H9, AArch64::H10, AArch64::H11, AArch64::H12, AArch64::H13, AArch64::H14, AArch64::H15, AArch64::H16, AArch64::H17, AArch64::H18, AArch64::H19, AArch64::H20, AArch64::H21, AArch64::H22, AArch64::H23, AArch64::H24, AArch64::H25, AArch64::H26, AArch64::H27, AArch64::H28, AArch64::H29, AArch64::H30, AArch64::H31, AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7, AArch64::S8, AArch64::S9, AArch64::S10, AArch64::S11, AArch64::S12, AArch64::S13, AArch64::S14, AArch64::S15, AArch64::S16, AArch64::S17, AArch64::S18, AArch64::S19, AArch64::S20, AArch64::S21, AArch64::S22, AArch64::S23, AArch64::S24, AArch64::S25, AArch64::S26, AArch64::S27, AArch64::S28, AArch64::S29, AArch64::S30, AArch64::S31, AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 0 };
static const uint32_t CSR_AArch64_AllRegs_RegMask[] = { 0xfffffe6c, 0xffffffff, 0xffffffff, 0xfe0001ff, 0xffffffff, 0xffffffff, 0xffffffff, 0x001fffff, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffdfffff, 0x001fffdf, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_AllRegs_SCS_SaveList[] = { AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7, AArch64::W8, AArch64::W9, AArch64::W10, AArch64::W11, AArch64::W12, AArch64::W13, AArch64::W14, AArch64::W15, AArch64::W16, AArch64::W17, AArch64::W18, AArch64::W19, AArch64::W20, AArch64::W21, AArch64::W22, AArch64::W23, AArch64::W24, AArch64::W25, AArch64::W26, AArch64::W27, AArch64::W28, AArch64::W29, AArch64::W30, AArch64::WSP, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::SP, AArch64::B0, AArch64::B1, AArch64::B2, AArch64::B3, AArch64::B4, AArch64::B5, AArch64::B6, AArch64::B7, AArch64::B8, AArch64::B9, AArch64::B10, AArch64::B11, AArch64::B12, AArch64::B13, AArch64::B14, AArch64::B15, AArch64::B16, AArch64::B17, AArch64::B18, AArch64::B19, AArch64::B20, AArch64::B21, AArch64::B22, AArch64::B23, AArch64::B24, AArch64::B25, AArch64::B26, AArch64::B27, AArch64::B28, AArch64::B29, AArch64::B30, AArch64::B31, AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7, AArch64::H8, AArch64::H9, AArch64::H10, AArch64::H11, AArch64::H12, AArch64::H13, AArch64::H14, AArch64::H15, AArch64::H16, AArch64::H17, AArch64::H18, AArch64::H19, AArch64::H20, AArch64::H21, AArch64::H22, AArch64::H23, AArch64::H24, AArch64::H25, AArch64::H26, AArch64::H27, AArch64::H28, AArch64::H29, AArch64::H30, AArch64::H31, AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7, AArch64::S8, AArch64::S9, AArch64::S10, AArch64::S11, AArch64::S12, AArch64::S13, AArch64::S14, AArch64::S15, AArch64::S16, AArch64::S17, AArch64::S18, AArch64::S19, AArch64::S20, AArch64::S21, AArch64::S22, AArch64::S23, AArch64::S24, AArch64::S25, AArch64::S26, AArch64::S27, AArch64::S28, AArch64::S29, AArch64::S30, AArch64::S31, AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 0 };
static const uint32_t CSR_AArch64_AllRegs_SCS_RegMask[] = { 0xfffffe6c, 0xffffffff, 0xffffffff, 0xfe0001ff, 0xffffffff, 0xffffffff, 0xffffffff, 0x001fffff, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffdfffff, 0x001fffdf, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_CXX_TLS_Darwin_SaveList[] = { AArch64::LR, AArch64::FP, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, 0 };
static const uint32_t CSR_AArch64_CXX_TLS_Darwin_RegMask[] = { 0xfffffe0c, 0xffffffff, 0xffffffff, 0x000001ff, 0xfe000000, 0xfdffffff, 0xfefff0ff, 0x001ff87f, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0x001fffff, 0x00000000, 0x00000000, 0x1f800000, 0x001e3f5f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_CXX_TLS_Darwin_PE_SaveList[] = { AArch64::LR, AArch64::FP, 0 };
static const uint32_t CSR_AArch64_CXX_TLS_Darwin_PE_RegMask[] = { 0x0000000c, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00c00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_CXX_TLS_Darwin_SCS_SaveList[] = { AArch64::LR, AArch64::FP, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_CXX_TLS_Darwin_SCS_RegMask[] = { 0xfffffe0c, 0xffffffff, 0xffffffff, 0x000001ff, 0xfe000000, 0xfdffffff, 0xfefff8ff, 0x001ffc7f, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0x001fffff, 0x00000000, 0x00000000, 0x9f800000, 0x001f3f5f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_CXX_TLS_Darwin_ViaCopy_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7, AArch64::D16, AArch64::D17, AArch64::D18, AArch64::D19, AArch64::D20, AArch64::D21, AArch64::D22, AArch64::D23, AArch64::D24, AArch64::D25, AArch64::D26, AArch64::D27, AArch64::D28, AArch64::D29, AArch64::D30, AArch64::D31, 0 };
static const uint32_t CSR_AArch64_CXX_TLS_Darwin_ViaCopy_RegMask[] = { 0xfffffe00, 0xffffffff, 0xffffffff, 0x000001ff, 0xfe000000, 0xfdffffff, 0xfe3ff0ff, 0x001ff87f, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0x001fffff, 0x00000000, 0x00000000, 0x1f800000, 0x001e3f0f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_NoRegs_SaveList[] = { 0 };
static const uint32_t CSR_AArch64_NoRegs_RegMask[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_NoRegs_SCS_SaveList[] = { AArch64::X18, 0 };
static const uint32_t CSR_AArch64_NoRegs_SCS_RegMask[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000800, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_RT_MostRegs_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, 0 };
static const uint32_t CSR_AArch64_RT_MostRegs_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff1fc, 0x001ff8fe, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x38000000, 0x001e705f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_RT_MostRegs_SCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_RT_MostRegs_SCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff9fc, 0x001ffcfe, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0xb8000000, 0x001f705f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_SVE_AAPCS_SaveList[] = { AArch64::Z8, AArch64::Z9, AArch64::Z10, AArch64::Z11, AArch64::Z12, AArch64::Z13, AArch64::Z14, AArch64::Z15, AArch64::Z16, AArch64::Z17, AArch64::Z18, AArch64::Z19, AArch64::Z20, AArch64::Z21, AArch64::Z22, AArch64::Z23, AArch64::P4, AArch64::P5, AArch64::P6, AArch64::P7, AArch64::P8, AArch64::P9, AArch64::P10, AArch64::P11, AArch64::P12, AArch64::P13, AArch64::P14, AArch64::P15, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, 0 };
static const uint32_t CSR_AArch64_SVE_AAPCS_RegMask[] = { 0xfffe000c, 0xfffe0001, 0xfffe0001, 0x01ffe001, 0x0001fffe, 0x0001fffe, 0x00fff000, 0xe01ff800, 0xe0001fff, 0xe0001fff, 0xe0000fff, 0xe00003ff, 0xe00007ff, 0xe0000fff, 0xe00003ff, 0x000007ff, 0xe01e005f, 0xe0000fff, 0xe00003ff, 0x000007ff, };
static const MCPhysReg CSR_AArch64_SVE_AAPCS_SCS_SaveList[] = { AArch64::Z8, AArch64::Z9, AArch64::Z10, AArch64::Z11, AArch64::Z12, AArch64::Z13, AArch64::Z14, AArch64::Z15, AArch64::Z16, AArch64::Z17, AArch64::Z18, AArch64::Z19, AArch64::Z20, AArch64::Z21, AArch64::Z22, AArch64::Z23, AArch64::P4, AArch64::P5, AArch64::P6, AArch64::P7, AArch64::P8, AArch64::P9, AArch64::P10, AArch64::P11, AArch64::P12, AArch64::P13, AArch64::P14, AArch64::P15, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::LR, AArch64::FP, AArch64::X18, 0 };
static const uint32_t CSR_AArch64_SVE_AAPCS_SCS_RegMask[] = { 0xfffe000c, 0xfffe0001, 0xfffe0001, 0x01ffe001, 0x0001fffe, 0x0001fffe, 0x00fff800, 0xe01ffc00, 0xe0001fff, 0xe0001fff, 0xe0000fff, 0xe00003ff, 0xe00007ff, 0xe0000fff, 0xe00003ff, 0x800007ff, 0xe01f005f, 0xe0000fff, 0xe00003ff, 0x000007ff, };
static const MCPhysReg CSR_AArch64_StackProbe_Windows_SaveList[] = { AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::SP, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 0 };
static const uint32_t CSR_AArch64_StackProbe_Windows_RegMask[] = { 0xfffffe64, 0xffffffff, 0xffffffff, 0xfe0001ff, 0xffffffff, 0xffffffff, 0xff7ff9ff, 0x001ffcff, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xbfdfffff, 0x001f7fdf, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_TLS_Darwin_SaveList[] = { AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 0 };
static const uint32_t CSR_AArch64_TLS_Darwin_RegMask[] = { 0xfffffe04, 0xffffffff, 0xffffffff, 0xfe0001ff, 0xffffffff, 0xfdffffff, 0xfe7ff9ff, 0x001ffcff, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xbf9fffff, 0x001f7f5f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_AArch64_TLS_ELF_SaveList[] = { AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::X9, AArch64::X10, AArch64::X11, AArch64::X12, AArch64::X13, AArch64::X14, AArch64::X15, AArch64::X16, AArch64::X17, AArch64::X18, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, AArch64::Q8, AArch64::Q9, AArch64::Q10, AArch64::Q11, AArch64::Q12, AArch64::Q13, AArch64::Q14, AArch64::Q15, AArch64::Q16, AArch64::Q17, AArch64::Q18, AArch64::Q19, AArch64::Q20, AArch64::Q21, AArch64::Q22, AArch64::Q23, AArch64::Q24, AArch64::Q25, AArch64::Q26, AArch64::Q27, AArch64::Q28, AArch64::Q29, AArch64::Q30, AArch64::Q31, 0 };
static const uint32_t CSR_AArch64_TLS_ELF_RegMask[] = { 0xfffffe04, 0xffffffff, 0xffffffff, 0xfe0001ff, 0xffffffff, 0xfdffffff, 0xfe7fffff, 0x001fffff, 0x00000000, 0xffe00000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xff9fffff, 0x001fff5f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_Darwin_AArch64_AAPCS_SaveList[] = { AArch64::LR, AArch64::FP, AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, 0 };
static const uint32_t CSR_Darwin_AArch64_AAPCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff000, 0x001ff800, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x001e005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_Win_AArch64_AAPCS_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, 0 };
static const uint32_t CSR_Win_AArch64_AAPCS_RegMask[] = { 0x01fe000c, 0x01fe0000, 0x01fe0000, 0x00000000, 0x00000000, 0x000001fe, 0x00fff000, 0x001ff800, 0x00000000, 0xe0000000, 0xe000000f, 0xe0000003, 0x00000007, 0x00000000, 0x00000000, 0x00000000, 0x001e005f, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_Win_AArch64_CFGuard_Check_SaveList[] = { AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28, AArch64::FP, AArch64::LR, AArch64::D8, AArch64::D9, AArch64::D10, AArch64::D11, AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15, AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7, AArch64::X8, AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7, 0 };
static const uint32_t CSR_Win_AArch64_CFGuard_Check_RegMask[] = { 0x01fffe0c, 0x01fffe00, 0x01fffe00, 0xfe000000, 0xfe000001, 0xfe0001ff, 0xfffff003, 0x001ff801, 0x00000000, 0xffe00000, 0xffe0000f, 0xffe00003, 0x0fe00007, 0x03e00000, 0x07e00000, 0x03c00000, 0x001e07df, 0x00000000, 0x00000000, 0x00000000, };


ArrayRef<const uint32_t *> AArch64GenRegisterInfo::getRegMasks() const {
  static const uint32_t *const Masks[] = {
    CSR_AArch64_AAPCS_RegMask,
    CSR_AArch64_AAPCS_SCS_RegMask,
    CSR_AArch64_AAPCS_SwiftError_RegMask,
    CSR_AArch64_AAPCS_SwiftError_SCS_RegMask,
    CSR_AArch64_AAPCS_ThisReturn_RegMask,
    CSR_AArch64_AAVPCS_RegMask,
    CSR_AArch64_AAVPCS_SCS_RegMask,
    CSR_AArch64_AllRegs_RegMask,
    CSR_AArch64_AllRegs_SCS_RegMask,
    CSR_AArch64_CXX_TLS_Darwin_RegMask,
    CSR_AArch64_CXX_TLS_Darwin_PE_RegMask,
    CSR_AArch64_CXX_TLS_Darwin_SCS_RegMask,
    CSR_AArch64_CXX_TLS_Darwin_ViaCopy_RegMask,
    CSR_AArch64_NoRegs_RegMask,
    CSR_AArch64_NoRegs_SCS_RegMask,
    CSR_AArch64_RT_MostRegs_RegMask,
    CSR_AArch64_RT_MostRegs_SCS_RegMask,
    CSR_AArch64_SVE_AAPCS_RegMask,
    CSR_AArch64_SVE_AAPCS_SCS_RegMask,
    CSR_AArch64_StackProbe_Windows_RegMask,
    CSR_AArch64_TLS_Darwin_RegMask,
    CSR_AArch64_TLS_ELF_RegMask,
    CSR_Darwin_AArch64_AAPCS_RegMask,
    CSR_Win_AArch64_AAPCS_RegMask,
    CSR_Win_AArch64_CFGuard_Check_RegMask,
  };
  return makeArrayRef(Masks);
}

ArrayRef<const char *> AArch64GenRegisterInfo::getRegMaskNames() const {
  static const char *const Names[] = {
    "CSR_AArch64_AAPCS",
    "CSR_AArch64_AAPCS_SCS",
    "CSR_AArch64_AAPCS_SwiftError",
    "CSR_AArch64_AAPCS_SwiftError_SCS",
    "CSR_AArch64_AAPCS_ThisReturn",
    "CSR_AArch64_AAVPCS",
    "CSR_AArch64_AAVPCS_SCS",
    "CSR_AArch64_AllRegs",
    "CSR_AArch64_AllRegs_SCS",
    "CSR_AArch64_CXX_TLS_Darwin",
    "CSR_AArch64_CXX_TLS_Darwin_PE",
    "CSR_AArch64_CXX_TLS_Darwin_SCS",
    "CSR_AArch64_CXX_TLS_Darwin_ViaCopy",
    "CSR_AArch64_NoRegs",
    "CSR_AArch64_NoRegs_SCS",
    "CSR_AArch64_RT_MostRegs",
    "CSR_AArch64_RT_MostRegs_SCS",
    "CSR_AArch64_SVE_AAPCS",
    "CSR_AArch64_SVE_AAPCS_SCS",
    "CSR_AArch64_StackProbe_Windows",
    "CSR_AArch64_TLS_Darwin",
    "CSR_AArch64_TLS_ELF",
    "CSR_Darwin_AArch64_AAPCS",
    "CSR_Win_AArch64_AAPCS",
    "CSR_Win_AArch64_CFGuard_Check",
  };
  return makeArrayRef(Names);
}

const AArch64FrameLowering *
AArch64GenRegisterInfo::getFrameLowering(const MachineFunction &MF) {
  return static_cast<const AArch64FrameLowering *>(
      MF.getSubtarget().getFrameLowering());
}

} // end namespace llvm

#endif // GET_REGINFO_TARGET_DESC

