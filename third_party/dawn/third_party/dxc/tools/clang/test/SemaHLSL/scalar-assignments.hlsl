// RUN: %dxc -Tlib_6_3   -verify %s
// RUN: %dxc -Tvs_6_0 -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 scalar-assignments.hlsl
// with vs_2_0 (the default) min16float usage produces a complaint that it's not supported

[shader("vertex")]
void main() {

snorm snorm float ssf; // expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'snorm'}}
snorm unorm float suf; // expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}}
snorm bool sb;         // expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3085: snorm can not be used with type}}

// Used to generate this undesirable error:
// cannot initialize a variable of type 'min16float' (aka 'half') with an lvalue of type 'const char [4]'
min16float foobar = "foo"; // expected-error {{cannot initialize a variable of type 'min16float' with an lvalue of type 'literal string'}} fxc-error {{X3017: cannot implicitly convert from 'const string' to 'min16float'}}

/*
(let (
      (values '(bool int uint dword half float double min16float min10float min16int min12int min16uint))
      (vi 0)
      )
  (dolist (v0 values)
        (dolist (v1 values)
              (progn
                (princ (format "%s left%d; %s right%d; left%d = right%d;\n" v0 vi v1 vi vi vi))
                (set 'vi (+ 1 vi))
                ))))
*/

// fxc errors look like this:
// warning X3205: conversion from larger type to smaller, possible loss of data

bool left0; bool right0; left0 = right0;
bool left1; int right1; left1 = right1;
bool left2; uint right2; left2 = right2;
bool left3; dword right3; left3 = right3;
bool left4; half right4; left4 = right4;
bool left5; float right5; left5 = right5;
bool left6; double right6; left6 = right6;
bool left7; min16float right7; left7 = right7;
bool left8; min10float right8; left8 = right8;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
bool left9; min16int right9; left9 = right9;
bool left10; min12int right10; left10 = right10;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
bool left11; min16uint right11; left11 = right11;
int left12; bool right12; left12 = right12;
int left13; int right13; left13 = right13;
int left14; uint right14; left14 = right14;
int left15; dword right15; left15 = right15;
int left16; half right16; left16 = right16;
int left17; float right17; left17 = right17;
int left18; double right18; left18 = right18; // expected-warning {{conversion from larger type 'double' to smaller type 'int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
int left19; min16float right19; left19 = right19;
int left20; min10float right20; left20 = right20;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
int left21; min16int right21; left21 = right21;
int left22; min12int right22; left22 = right22;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
int left23; min16uint right23; left23 = right23;
uint left24; bool right24; left24 = right24;
uint left25; int right25; left25 = right25;
uint left26; uint right26; left26 = right26;
uint left27; dword right27; left27 = right27;
uint left28; half right28; left28 = right28;
uint left29; float right29; left29 = right29;
uint left30; double right30; left30 = right30; // expected-warning {{conversion from larger type 'double' to smaller type 'uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
uint left31; min16float right31; left31 = right31;
uint left32; min10float right32; left32 = right32;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
uint left33; min16int right33; left33 = right33;
uint left34; min12int right34; left34 = right34;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
uint left35; min16uint right35; left35 = right35;
dword left36; bool right36; left36 = right36;
dword left37; int right37; left37 = right37;
dword left38; uint right38; left38 = right38;
dword left39; dword right39; left39 = right39;
dword left40; half right40; left40 = right40;
dword left41; float right41; left41 = right41;
dword left42; double right42; left42 = right42; // expected-warning {{conversion from larger type 'double' to smaller type 'dword', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
dword left43; min16float right43; left43 = right43;
dword left44; min10float right44; left44 = right44;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
dword left45; min16int right45; left45 = right45;
dword left46; min12int right46; left46 = right46;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
dword left47; min16uint right47; left47 = right47;
half left48; bool right48; left48 = right48;
half left49; int right49; left49 = right49;
half left50; uint right50; left50 = right50;
half left51; dword right51; left51 = right51;
half left52; half right52; left52 = right52;
half left53; float right53; left53 = right53;
half left54; double right54; left54 = right54; // expected-warning {{conversion from larger type 'double' to smaller type 'half', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
half left55; min16float right55; left55 = right55;
half left56; min10float right56; left56 = right56;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
half left57; min16int right57; left57 = right57;
half left58; min12int right58; left58 = right58;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
half left59; min16uint right59; left59 = right59;
float left60; bool right60; left60 = right60;
float left61; int right61; left61 = right61;
float left62; uint right62; left62 = right62;
float left63; dword right63; left63 = right63;
float left64; half right64; left64 = right64;
float left65; float right65; left65 = right65;
float left66; double right66; left66 = right66; // expected-warning {{conversion from larger type 'double' to smaller type 'float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
float left67; min16float right67; left67 = right67;
float left68; min10float right68; left68 = right68;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
float left69; min16int right69; left69 = right69;
float left70; min12int right70; left70 = right70;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
float left71; min16uint right71; left71 = right71;
double left72; bool right72; left72 = right72;
double left73; int right73; left73 = right73;
double left74; uint right74; left74 = right74;
double left75; dword right75; left75 = right75;
double left76; half right76; left76 = right76;
double left77; float right77; left77 = right77;
double left78; double right78; left78 = right78;
double left79; min16float right79; left79 = right79;
double left80; min10float right80; left80 = right80;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
double left81; min16int right81; left81 = right81;
double left82; min12int right82; left82 = right82;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
double left83; min16uint right83; left83 = right83;
min16float left84; bool right84; left84 = right84;
min16float left85; int right85; left85 = right85; // expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left86; uint right86; left86 = right86; // expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left87; dword right87; left87 = right87; // expected-warning {{conversion from larger type 'dword' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left88; half right88; left88 = right88; // expected-warning {{conversion from larger type 'half' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left89; float right89; left89 = right89; // expected-warning {{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left90; double right90; left90 = right90; // expected-warning {{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left91; min16float right91; left91 = right91;
min16float left92; min10float right92; left92 = right92;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
min16float left93; min16int right93; left93 = right93;
min16float left94; min12int right94; left94 = right94;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
min16float left95; min16uint right95; left95 = right95;
min10float left96; bool right96; left96 = right96;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
min10float left97; int right97; left97 = right97; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left98; uint right98; left98 = right98; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left99; dword right99; left99 = right99; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left100; half right100; left100 = right100; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'half' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left101; float right101; left101 = right101; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left102; double right102; left102 = right102; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'double' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left103; min16float right103; left103 = right103; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'min16float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min10float left104; min10float right104; left104 = right104;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
min10float left105; min16int right105; left105 = right105; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min10float left106; min12int right106; left106 = right106; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min10float left107; min16uint right107; left107 = right107; // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left108; bool right108; left108 = right108;
min16int left109; int right109; left109 = right109; // expected-warning {{conversion from larger type 'int' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left110; uint right110; left110 = right110; // expected-warning {{conversion from larger type 'uint' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left111; dword right111; left111 = right111; // expected-warning {{conversion from larger type 'dword' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left112; half right112; left112 = right112; // expected-warning {{conversion from larger type 'half' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left113; float right113; left113 = right113; // expected-warning {{conversion from larger type 'float' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left114; double right114; left114 = right114; // expected-warning {{conversion from larger type 'double' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left115; min16float right115; left115 = right115;
min16int left116; min10float right116; left116 = right116;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
min16int left117; min16int right117; left117 = right117;
min16int left118; min12int right118; left118 = right118;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
min16int left119; min16uint right119; left119 = right119;
min12int left120; bool right120; left120 = right120;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
min12int left121; int right121; left121 = right121; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'int' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left122; uint right122; left122 = right122; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left123; dword right123; left123 = right123; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left124; half right124; left124 = right124; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'half' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left125; float right125; left125 = right125; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'float' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left126; double right126; left126 = right126; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'double' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left127; min16float right127; left127 = right127; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'min16float' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min12int left128; min10float right128; left128 = right128;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} //
min12int left129; min16int right129; left129 = right129; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'min16int' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min12int left130; min12int right130; left130 = right130;  // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} //
min12int left131; min16uint right131; left131 = right131; // expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'min16uint' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left132; bool right132; left132 = right132;
min16uint left133; int right133; left133 = right133; // expected-warning {{conversion from larger type 'int' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left134; uint right134; left134 = right134; // expected-warning {{conversion from larger type 'uint' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left135; dword right135; left135 = right135; // expected-warning {{conversion from larger type 'dword' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left136; half right136; left136 = right136; // expected-warning {{conversion from larger type 'half' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left137; float right137; left137 = right137; // expected-warning {{conversion from larger type 'float' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left138; double right138; left138 = right138; // expected-warning {{conversion from larger type 'double' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left139; min16float right139; left139 = right139;
min16uint left140; min10float right140; left140 = right140;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
min16uint left141; min16int right141; left141 = right141;
min16uint left142; min12int right142; left142 = right142;  // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
min16uint left143; min16uint right143; left143 = right143;

// Now with unorm and snorm modifiers.
/*
(let (
      (floats '(float double min16float min10float))
      (modifiers '("" "snorm" "unorm"))
      (vi 1000)
      )
  (loop for v0 in floats do
        (loop for m0 in modifiers do
              (loop for v1 in floats do
                    (loop for m1 in modifiers do
                          (when (not (and (eq m0 "") (eq m1 "")))
                            (princ (format "%s %s left%d; %s %s right%d; left%d = right%d;\n" m0 v0 vi m1 v1 vi vi vi))
                            (set 'vi (+ 1 vi))
                            ))))))
*/

 float left1000; snorm float right1000; left1000 = right1000;
 float left1001; unorm float right1001; left1001 = right1001;
// float left1002; snorm double right1002; left1002 = right1002;
// float left1003; unorm double right1003; left1003 = right1003;
 float left1004; snorm min16float right1004; left1004 = right1004;
 float left1005; unorm min16float right1005; left1005 = right1005;
 float left1006; snorm min10float right1006; left1006 = right1006;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
 float left1007; unorm min10float right1007; left1007 = right1007;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm float left1008;  float right1008; left1008 = right1008;
snorm float left1009; snorm float right1009; left1009 = right1009;
snorm float left1010; unorm float right1010; left1010 = right1010;
// snorm float left1011;  double right1011; left1011 = right1011;
// snorm float left1012; snorm double right1012; left1012 = right1012;
// snorm float left1013; unorm double right1013; left1013 = right1013;
snorm float left1014;  min16float right1014; left1014 = right1014;
snorm float left1015; snorm min16float right1015; left1015 = right1015;
snorm float left1016; unorm min16float right1016; left1016 = right1016;
snorm float left1017;  min10float right1017; left1017 = right1017;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm float left1018; snorm min10float right1018; left1018 = right1018;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm float left1019; unorm min10float right1019; left1019 = right1019;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm float left1020;  float right1020; left1020 = right1020;
unorm float left1021; snorm float right1021; left1021 = right1021;
unorm float left1022; unorm float right1022; left1022 = right1022;
// unorm float left1023;  double right1023; left1023 = right1023;
// unorm float left1024; snorm double right1024; left1024 = right1024;
// unorm float left1025; unorm double right1025; left1025 = right1025;
unorm float left1026;  min16float right1026; left1026 = right1026;
unorm float left1027; snorm min16float right1027; left1027 = right1027;
unorm float left1028; unorm min16float right1028; left1028 = right1028;
unorm float left1029;  min10float right1029; left1029 = right1029;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm float left1030; snorm min10float right1030; left1030 = right1030;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm float left1031; unorm min10float right1031; left1031 = right1031;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
 double left1032; snorm float right1032; left1032 = right1032;
 double left1033; unorm float right1033; left1033 = right1033;
 double left1034; snorm double right1034; left1034 = right1034;
 double left1035; unorm double right1035; left1035 = right1035;
 double left1036; snorm min16float right1036; left1036 = right1036;
 double left1037; unorm min16float right1037; left1037 = right1037;
 double left1038; snorm min10float right1038; left1038 = right1038;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
 double left1039; unorm min10float right1039; left1039 = right1039;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm double left1040;  float right1040; left1040 = right1040;
snorm double left1041; snorm float right1041; left1041 = right1041;
snorm double left1042; unorm float right1042; left1042 = right1042;
snorm double left1043;  double right1043; left1043 = right1043;
snorm double left1044; snorm double right1044; left1044 = right1044;
snorm double left1045; unorm double right1045; left1045 = right1045;
snorm double left1046;  min16float right1046; left1046 = right1046;
snorm double left1047; snorm min16float right1047; left1047 = right1047;
snorm double left1048; unorm min16float right1048; left1048 = right1048;
snorm double left1049;  min10float right1049; left1049 = right1049;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm double left1050; snorm min10float right1050; left1050 = right1050;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm double left1051; unorm min10float right1051; left1051 = right1051;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm double left1052;  float right1052; left1052 = right1052;
unorm double left1053; snorm float right1053; left1053 = right1053;
unorm double left1054; unorm float right1054; left1054 = right1054;
unorm double left1055;  double right1055; left1055 = right1055;
unorm double left1056; snorm double right1056; left1056 = right1056;
unorm double left1057; unorm double right1057; left1057 = right1057;
unorm double left1058;  min16float right1058; left1058 = right1058;
unorm double left1059; snorm min16float right1059; left1059 = right1059;
unorm double left1060; unorm min16float right1060; left1060 = right1060;
unorm double left1061;  min10float right1061; left1061 = right1061;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm double left1062; snorm min10float right1062; left1062 = right1062;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm double left1063; unorm min10float right1063; left1063 = right1063;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
// min16float left1064; snorm float right1064; left1064 = right1064;
// min16float left1065; unorm float right1065; left1065 = right1065;
// min16float left1066; snorm double right1066; left1066 = right1066;
// min16float left1067; unorm double right1067; left1067 = right1067;
 min16float left1068; snorm min16float right1068; left1068 = right1068;
 min16float left1069; unorm min16float right1069; left1069 = right1069;
 min16float left1070; snorm min10float right1070; left1070 = right1070;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
 min16float left1071; unorm min10float right1071; left1071 = right1071;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
// snorm min16float left1072;  float right1072; left1072 = right1072;
// snorm min16float left1073; snorm float right1073; left1073 = right1073;
// snorm min16float left1074; unorm float right1074; left1074 = right1074;
// snorm min16float left1075;  double right1075; left1075 = right1075;
// snorm min16float left1076; snorm double right1076; left1076 = right1076;
// snorm min16float left1077; unorm double right1077; left1077 = right1077;
snorm min16float left1078;  min16float right1078; left1078 = right1078;
snorm min16float left1079; snorm min16float right1079; left1079 = right1079;
snorm min16float left1080; unorm min16float right1080; left1080 = right1080;
snorm min16float left1081;  min10float right1081; left1081 = right1081;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm min16float left1082; snorm min10float right1082; left1082 = right1082;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
snorm min16float left1083; unorm min10float right1083; left1083 = right1083;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
// unorm min16float left1084;  float right1084; left1084 = right1084;
// unorm min16float left1085; snorm float right1085; left1085 = right1085;
// unorm min16float left1086; unorm float right1086; left1086 = right1086;
// unorm min16float left1087;  double right1087; left1087 = right1087;
// unorm min16float left1088; snorm double right1088; left1088 = right1088;
// unorm min16float left1089; unorm double right1089; left1089 = right1089;
unorm min16float left1090;  min16float right1090; left1090 = right1090;
unorm min16float left1091; snorm min16float right1091; left1091 = right1091;
unorm min16float left1092; unorm min16float right1092; left1092 = right1092;
unorm min16float left1093;  min10float right1093; left1093 = right1093;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm min16float left1094; snorm min10float right1094; left1094 = right1094;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
unorm min16float left1095; unorm min10float right1095; left1095 = right1095;  // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
// min10float left1096; snorm float right1096; left1096 = right1096;
// min10float left1097; unorm float right1097; left1097 = right1097;
// min10float left1098; snorm double right1098; left1098 = right1098;
// min10float left1099; unorm double right1099; left1099 = right1099;
// min10float left1100; snorm min16float right1100; left1100 = right1100;
// min10float left1101; unorm min16float right1101; left1101 = right1101;
 min10float left1102; snorm min10float right1102; left1102 = right1102;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
 min10float left1103; unorm min10float right1103; left1103 = right1103;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
// snorm min10float left1104;  float right1104; left1104 = right1104;
// snorm min10float left1105; snorm float right1105; left1105 = right1105;
// snorm min10float left1106; unorm float right1106; left1106 = right1106;
// snorm min10float left1107;  double right1107; left1107 = right1107;
// snorm min10float left1108; snorm double right1108; left1108 = right1108;
// snorm min10float left1109; unorm double right1109; left1109 = right1109;
// snorm min10float left1110;  min16float right1110; left1110 = right1110;
// snorm min10float left1111; snorm min16float right1111; left1111 = right1111;
// snorm min10float left1112; unorm min16float right1112; left1112 = right1112;
snorm min10float left1113;  min10float right1113; left1113 = right1113;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
snorm min10float left1114; snorm min10float right1114; left1114 = right1114;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
snorm min10float left1115; unorm min10float right1115; left1115 = right1115;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
// unorm min10float left1116;  float right1116; left1116 = right1116;
// unorm min10float left1117; snorm float right1117; left1117 = right1117;
// unorm min10float left1118; unorm float right1118; left1118 = right1118;
// unorm min10float left1119;  double right1119; left1119 = right1119;
// unorm min10float left1120; snorm double right1120; left1120 = right1120;
// unorm min10float left1121; unorm double right1121; left1121 = right1121;
// unorm min10float left1122;  min16float right1122; left1122 = right1122;
// unorm min10float left1123; snorm min16float right1123; left1123 = right1123;
// unorm min10float left1124; unorm min16float right1124; left1124 = right1124;
unorm min10float left1125;  min10float right1125; left1125 = right1125;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
unorm min10float left1126; snorm min10float right1126; left1126 = right1126;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //
unorm min10float left1127; unorm min10float right1127; left1127 = right1127;  // expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} //


// Test additional types

/*<py>
import re
rxComments = re.compile(r'(//.*|/\*.*?\*\/)')
def strip_comments(line):
    line = rxComments.sub('', line)
    return line.strip()
def save_error_comments(lines):
    saved = {}
    for line in lines:
        key = strip_comments(line)
        if key and line.strip() != key:
            saved[key] = line
    return saved
def restore_error_comments(saved, lines):
    return [saved.get(line.strip(), line) for line in lines]
def modify(lines, newlines):
    return restore_error_comments(save_error_comments(lines), newlines)
def gen_code(template, combos):
    return [
        template.format(left = left, right = right)
        for left, right in combos]
</py>*/

/*<py>
types1 = 'bool int uint dword half float double min16float min10float min16int min12int min16uint'.split()
types2 = 'int64_t uint64_t int8_t4_packed uint8_t4_packed'.split()
types = types1 + types2
new_type_combos = [(left, right) for left in types1 for right in types2]
new_type_combos += [(left, right) for left in types2 for right in types]
</py>*/

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left; {right} right; left = right; }}', new_type_combos))</py>
// GENERATED_CODE:BEGIN
{ bool left; int64_t right; left = right; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ bool left; uint64_t right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ bool left; int8_t4_packed right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ bool left; uint8_t4_packed right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ int left; int64_t right; left = right; }                                    /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ int left; uint64_t right; left = right; }                                   /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ int left; int8_t4_packed right; left = right; }                             /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ int left; uint8_t4_packed right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint left; int64_t right; left = right; }                                   /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ uint left; uint64_t right; left = right; }                                  /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint left; int8_t4_packed right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ uint left; uint8_t4_packed right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ dword left; int64_t right; left = right; }                                  /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'dword', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ dword left; uint64_t right; left = right; }                                 /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'dword', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ dword left; int8_t4_packed right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ dword left; uint8_t4_packed right; left = right; }                          /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ half left; int64_t right; left = right; }                                   /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'half', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ half left; uint64_t right; left = right; }                                  /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'half', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ half left; int8_t4_packed right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ half left; uint8_t4_packed right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ float left; int64_t right; left = right; }                                  /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ float left; uint64_t right; left = right; }                                 /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ float left; int8_t4_packed right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ float left; uint8_t4_packed right; left = right; }                          /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ double left; int64_t right; left = right; }                                 /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ double left; uint64_t right; left = right; }                                /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ double left; int8_t4_packed right; left = right; }                          /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ double left; uint8_t4_packed right; left = right; }                         /* fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ min16float left; int64_t right; left = right; }                             /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'min16float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16float left; uint64_t right; left = right; }                            /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'min16float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ min16float left; int8_t4_packed right; left = right; }                      /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'min16float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16float left; uint8_t4_packed right; left = right; }                     /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'min16float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ min10float left; int64_t right; left = right; }                             /* expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'int64_t' to smaller type 'min10float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min10float left; uint64_t right; left = right; }                            /* expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'uint64_t' to smaller type 'min10float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ min10float left; int8_t4_packed right; left = right; }                      /* expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'min10float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min10float left; uint8_t4_packed right; left = right; }                     /* expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'min10float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ min16int left; int64_t right; left = right; }                               /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'min16int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16int left; uint64_t right; left = right; }                              /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'min16int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ min16int left; int8_t4_packed right; left = right; }                        /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'min16int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16int left; uint8_t4_packed right; left = right; }                       /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'min16int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ min12int left; int64_t right; left = right; }                               /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'int64_t' to smaller type 'min12int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min12int left; uint64_t right; left = right; }                              /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'uint64_t' to smaller type 'min12int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ min12int left; int8_t4_packed right; left = right; }                        /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'min12int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min12int left; uint8_t4_packed right; left = right; }                       /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'min12int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ min16uint left; int64_t right; left = right; }                              /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'min16uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16uint left; uint64_t right; left = right; }                             /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'min16uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ min16uint left; int8_t4_packed right; left = right; }                       /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'min16uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'right'}} */
{ min16uint left; uint8_t4_packed right; left = right; }                      /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'min16uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ int64_t left; bool right; left = right; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; int right; left = right; }                                    /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; uint right; left = right; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; dword right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; half right; left = right; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; float right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; double right; left = right; }                                 /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; min16float right; left = right; }                             /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; min10float right; left = right; }                             /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; min16int right; left = right; }                               /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; min12int right; left = right; }                               /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; min16uint right; left = right; }                              /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; int64_t right; left = right; }                                /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; uint64_t right; left = right; }                               /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; int8_t4_packed right; left = right; }                         /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left; uint8_t4_packed right; left = right; }                        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint64_t left; bool right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; int right; left = right; }                                   /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; uint right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; dword right; left = right; }                                 /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; half right; left = right; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; float right; left = right; }                                 /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; double right; left = right; }                                /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; min16float right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; min10float right; left = right; }                            /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; min16int right; left = right; }                              /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; min12int right; left = right; }                              /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; min16uint right; left = right; }                             /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; int64_t right; left = right; }                               /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; uint64_t right; left = right; }                              /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; int8_t4_packed right; left = right; }                        /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left; uint8_t4_packed right; left = right; }                       /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ int8_t4_packed left; bool right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; int right; left = right; }                             /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; uint right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; dword right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; half right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; float right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; double right; left = right; }                          /* expected-warning {{conversion from larger type 'double' to smaller type 'int8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; min16float right; left = right; }                      /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; min10float right; left = right; }                      /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; min16int right; left = right; }                        /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; min12int right; left = right; }                        /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; min16uint right; left = right; }                       /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; int64_t right; left = right; }                         /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'int8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; uint64_t right; left = right; }                        /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'int8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; int8_t4_packed right; left = right; }                  /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left; uint8_t4_packed right; left = right; }                 /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint8_t4_packed left; bool right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; int right; left = right; }                            /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; uint right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; dword right; left = right; }                          /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; half right; left = right; }                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; float right; left = right; }                          /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; double right; left = right; }                         /* expected-warning {{conversion from larger type 'double' to smaller type 'uint8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; min16float right; left = right; }                     /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; min10float right; left = right; }                     /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; min16int right; left = right; }                       /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; min12int right; left = right; }                       /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; min16uint right; left = right; }                      /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; int64_t right; left = right; }                        /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'uint8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; uint64_t right; left = right; }                       /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'uint8_t4_packed', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; int8_t4_packed right; left = right; }                 /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left; uint8_t4_packed right; left = right; }                /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
// GENERATED_CODE:END

// Constant assignments

/*<py>
constant_ints = '0 -1 2U 3L 4ULL 3000000000 -3000000000 10000000000'.split()
constant_floats = '0.5 -0.5F'.split()
constants = constant_ints + constant_floats
constant_assignment_combos = [(left, right) for left in types for right in constants]
</py>*/

// Catch bugs with SemaHLSL GetUnsignedLimit/GetSignedLimit

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left = {right}; }}', constant_assignment_combos))</py>
// GENERATED_CODE:BEGIN
{ bool left = 0; }
{ bool left = -1; }
{ bool left = 2U; }
{ bool left = 3L; }
{ bool left = 4ULL; }                                                         /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ bool left = 3000000000; }
{ bool left = -3000000000; }
{ bool left = 10000000000; }                                                  /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ bool left = 0.5; }                                                          /* expected-warning {{implicit conversion from 'literal float' to 'bool' changes value from 0.5 to false}} fxc-pass {{}} */
{ bool left = -0.5F; }                                                        /* expected-warning {{implicit conversion from 'float' to 'bool' changes value from 0.5 to false}} fxc-pass {{}} */
{ int left = 0; }
{ int left = -1; }
{ int left = 2U; }
{ int left = 3L; }
{ int left = 4ULL; }                                                          /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ int left = 3000000000; }
{ int left = -3000000000; }                                                   /* expected-warning {{implicit conversion from 'literal int' to 'int' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ int left = 10000000000; }                                                   /* expected-warning {{implicit conversion from 'literal int' to 'int' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ int left = 0.5; }                                                           /* expected-warning {{implicit conversion from 'literal float' to 'int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ int left = -0.5F; }                                                         /* expected-warning {{implicit conversion from 'float' to 'int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ uint left = 0; }
{ uint left = -1; }
{ uint left = 2U; }
{ uint left = 3L; }
{ uint left = 4ULL; }                                                         /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ uint left = 3000000000; }
{ uint left = -3000000000; }                                                  /* expected-warning {{implicit conversion from 'literal int' to 'uint' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ uint left = 10000000000; }                                                  /* expected-warning {{implicit conversion from 'literal int' to 'uint' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ uint left = 0.5; }                                                          /* expected-warning {{implicit conversion from 'literal float' to 'uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ uint left = -0.5F; }                                                        /* expected-warning {{implicit conversion from 'float' to 'uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ dword left = 0; }
{ dword left = -1; }
{ dword left = 2U; }
{ dword left = 3L; }
{ dword left = 4ULL; }                                                        /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ dword left = 3000000000; }
{ dword left = -3000000000; }                                                 /* expected-warning {{implicit conversion from 'literal int' to 'dword' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ dword left = 10000000000; }                                                 /* expected-warning {{implicit conversion from 'literal int' to 'dword' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ dword left = 0.5; }                                                         /* expected-warning {{implicit conversion from 'literal float' to 'dword' changes value from 0.5 to 0}} fxc-pass {{}} */
{ dword left = -0.5F; }                                                       /* expected-warning {{implicit conversion from 'float' to 'dword' changes value from 0.5 to 0}} fxc-pass {{}} */
{ half left = 0; }
{ half left = -1; }
{ half left = 2U; }
{ half left = 3L; }
{ half left = 4ULL; }                                                         /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ half left = 3000000000; }
{ half left = -3000000000; }
{ half left = 10000000000; }                                                  /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ half left = 0.5; }
{ half left = -0.5F; }
{ float left = 0; }
{ float left = -1; }
{ float left = 2U; }
{ float left = 3L; }
{ float left = 4ULL; }                                                        /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ float left = 3000000000; }
{ float left = -3000000000; }
{ float left = 10000000000; }                                                 /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ float left = 0.5; }
{ float left = -0.5F; }
{ double left = 0; }
{ double left = -1; }
{ double left = 2U; }
{ double left = 3L; }
{ double left = 4ULL; }                                                       /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ double left = 3000000000; }
{ double left = -3000000000; }
{ double left = 10000000000; }                                                /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ double left = 0.5; }
{ double left = -0.5F; }
{ min16float left = 0; }
{ min16float left = -1; }
{ min16float left = 2U; }                                                     /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left = 3L; }
{ min16float left = 4ULL; }                                                   /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left = 3000000000; }                                             /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left = -3000000000; }                                            /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left = 10000000000; }                                            /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16float left = 0.5; }
{ min16float left = -0.5F; }                                                  /* expected-warning {{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left = 0; }                                                      /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} */
{ min10float left = -1; }                                                     /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} */
{ min10float left = 2U; }                                                     /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left = 3L; }                                                     /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} */
{ min10float left = 4ULL; }                                                   /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left = 3000000000; }                                             /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left = -3000000000; }                                            /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left = 10000000000; }                                            /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min10float left = 0.5; }                                                    /* expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}} */
{ min10float left = -0.5F; }                                                  /* expected-warning {{'min10float' is promoted to 'min16float'}} expected-warning {{conversion from larger type 'float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left = 0; }
{ min16int left = -1; }
{ min16int left = 2U; }                                                       /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left = 3L; }
{ min16int left = 4ULL; }                                                     /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left = 3000000000; }                                               /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left = -3000000000; }                                              /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from -3000000000 to -24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left = 10000000000; }                                              /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from 10000000000 to -7168}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16int left = 0.5; }                                                      /* expected-warning {{implicit conversion from 'literal float' to 'min16int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min16int left = -0.5F; }                                                    /* expected-warning {{conversion from larger type 'float' to smaller type 'min16int', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min16int' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left = 0; }                                                        /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} */
{ min12int left = -1; }                                                       /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} */
{ min12int left = 2U; }                                                       /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left = 3L; }                                                       /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} */
{ min12int left = 4ULL; }                                                     /* expected-warning {{'min12int' is promoted to 'min16int'}} fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left = 3000000000; }                                               /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left = -3000000000; }                                              /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from -3000000000 to -24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left = 10000000000; }                                              /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from 10000000000 to -7168}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min12int left = 0.5; }                                                      /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{implicit conversion from 'literal float' to 'min12int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min12int left = -0.5F; }                                                    /* expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{conversion from larger type 'float' to smaller type 'min12int', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min12int' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left = 0; }
{ min16uint left = -1; }
{ min16uint left = 2U; }                                                      /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left = 3L; }
{ min16uint left = 4ULL; }                                                    /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left = 3000000000; }                                              /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left = -3000000000; }                                             /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from -3000000000 to 41472}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left = 10000000000; }                                             /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from 10000000000 to 58368}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16uint left = 0.5; }                                                     /* expected-warning {{implicit conversion from 'literal float' to 'min16uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min16uint left = -0.5F; }                                                   /* expected-warning {{conversion from larger type 'float' to smaller type 'min16uint', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min16uint' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ int64_t left = 0; }                                                         /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = -1; }                                                        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 2U; }                                                        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 3L; }                                                        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 4ULL; }                                                      /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 3000000000; }                                                /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = -3000000000; }                                               /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 10000000000; }                                               /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = 0.5; }                                                       /* expected-warning {{implicit conversion from 'literal float' to 'int64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left = -0.5F; }                                                     /* expected-warning {{implicit conversion from 'float' to 'int64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint64_t left = 0; }                                                        /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = -1; }                                                       /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 2U; }                                                       /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 3L; }                                                       /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 4ULL; }                                                     /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 3000000000; }                                               /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = -3000000000; }                                              /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 10000000000; }                                              /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = 0.5; }                                                      /* expected-warning {{implicit conversion from 'literal float' to 'uint64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left = -0.5F; }                                                    /* expected-warning {{implicit conversion from 'float' to 'uint64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ int8_t4_packed left = 0; }                                                  /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = -1; }                                                 /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 2U; }                                                 /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 3L; }                                                 /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 4ULL; }                                               /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 3000000000; }                                         /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = -3000000000; }                                        /* expected-warning {{implicit conversion from 'literal int' to 'int8_t4_packed' changes value from -3000000000 to 1294967296}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 10000000000; }                                        /* expected-warning {{implicit conversion from 'literal int' to 'int8_t4_packed' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = 0.5; }                                                /* expected-warning {{implicit conversion from 'literal float' to 'int8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left = -0.5F; }                                              /* expected-warning {{implicit conversion from 'float' to 'int8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint8_t4_packed left = 0; }                                                 /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = -1; }                                                /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 2U; }                                                /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 3L; }                                                /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 4ULL; }                                              /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 3000000000; }                                        /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = -3000000000; }                                       /* expected-warning {{implicit conversion from 'literal int' to 'uint8_t4_packed' changes value from -3000000000 to 1294967296}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 10000000000; }                                       /* expected-warning {{implicit conversion from 'literal int' to 'uint8_t4_packed' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = 0.5; }                                               /* expected-warning {{implicit conversion from 'literal float' to 'uint8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left = -0.5F; }                                             /* expected-warning {{implicit conversion from 'float' to 'uint8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
// GENERATED_CODE:END

// Catch bugs with clang type ranges when adding custom types

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left[2]; left[1] = {right}; }}', constant_assignment_combos))</py>
// GENERATED_CODE:BEGIN
{ bool left[2]; left[1] = 0; }
{ bool left[2]; left[1] = -1; }
{ bool left[2]; left[1] = 2U; }
{ bool left[2]; left[1] = 3L; }
{ bool left[2]; left[1] = 4ULL; }                                             /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ bool left[2]; left[1] = 3000000000; }
{ bool left[2]; left[1] = -3000000000; }
{ bool left[2]; left[1] = 10000000000; }                                      /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ bool left[2]; left[1] = 0.5; }                                              /* expected-warning {{implicit conversion from 'literal float' to 'bool' changes value from 0.5 to false}} fxc-pass {{}} */
{ bool left[2]; left[1] = -0.5F; }                                            /* expected-warning {{implicit conversion from 'float' to 'bool' changes value from 0.5 to false}} fxc-pass {{}} */
{ int left[2]; left[1] = 0; }
{ int left[2]; left[1] = -1; }
{ int left[2]; left[1] = 2U; }
{ int left[2]; left[1] = 3L; }
{ int left[2]; left[1] = 4ULL; }                                              /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ int left[2]; left[1] = 3000000000; }
{ int left[2]; left[1] = -3000000000; }                                       /* expected-warning {{implicit conversion from 'literal int' to 'int' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ int left[2]; left[1] = 10000000000; }                                       /* expected-warning {{implicit conversion from 'literal int' to 'int' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ int left[2]; left[1] = 0.5; }                                               /* expected-warning {{implicit conversion from 'literal float' to 'int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ int left[2]; left[1] = -0.5F; }                                             /* expected-warning {{implicit conversion from 'float' to 'int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ uint left[2]; left[1] = 0; }
{ uint left[2]; left[1] = -1; }
{ uint left[2]; left[1] = 2U; }
{ uint left[2]; left[1] = 3L; }
{ uint left[2]; left[1] = 4ULL; }                                             /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ uint left[2]; left[1] = 3000000000; }
{ uint left[2]; left[1] = -3000000000; }                                      /* expected-warning {{implicit conversion from 'literal int' to 'uint' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ uint left[2]; left[1] = 10000000000; }                                      /* expected-warning {{implicit conversion from 'literal int' to 'uint' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ uint left[2]; left[1] = 0.5; }                                              /* expected-warning {{implicit conversion from 'literal float' to 'uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ uint left[2]; left[1] = -0.5F; }                                            /* expected-warning {{implicit conversion from 'float' to 'uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ dword left[2]; left[1] = 0; }
{ dword left[2]; left[1] = -1; }
{ dword left[2]; left[1] = 2U; }
{ dword left[2]; left[1] = 3L; }
{ dword left[2]; left[1] = 4ULL; }                                            /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ dword left[2]; left[1] = 3000000000; }
{ dword left[2]; left[1] = -3000000000; }                                     /* expected-warning {{implicit conversion from 'literal int' to 'dword' changes value from -3000000000 to 1294967296}} fxc-pass {{}} */
{ dword left[2]; left[1] = 10000000000; }                                     /* expected-warning {{implicit conversion from 'literal int' to 'dword' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ dword left[2]; left[1] = 0.5; }                                             /* expected-warning {{implicit conversion from 'literal float' to 'dword' changes value from 0.5 to 0}} fxc-pass {{}} */
{ dword left[2]; left[1] = -0.5F; }                                           /* expected-warning {{implicit conversion from 'float' to 'dword' changes value from 0.5 to 0}} fxc-pass {{}} */
{ half left[2]; left[1] = 0; }
{ half left[2]; left[1] = -1; }
{ half left[2]; left[1] = 2U; }
{ half left[2]; left[1] = 3L; }
{ half left[2]; left[1] = 4ULL; }                                             /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ half left[2]; left[1] = 3000000000; }
{ half left[2]; left[1] = -3000000000; }
{ half left[2]; left[1] = 10000000000; }                                      /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ half left[2]; left[1] = 0.5; }
{ half left[2]; left[1] = -0.5F; }
{ float left[2]; left[1] = 0; }
{ float left[2]; left[1] = -1; }
{ float left[2]; left[1] = 2U; }
{ float left[2]; left[1] = 3L; }
{ float left[2]; left[1] = 4ULL; }                                            /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ float left[2]; left[1] = 3000000000; }
{ float left[2]; left[1] = -3000000000; }
{ float left[2]; left[1] = 10000000000; }                                     /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ float left[2]; left[1] = 0.5; }
{ float left[2]; left[1] = -0.5F; }
{ double left[2]; left[1] = 0; }
{ double left[2]; left[1] = -1; }
{ double left[2]; left[1] = 2U; }
{ double left[2]; left[1] = 3L; }
{ double left[2]; left[1] = 4ULL; }                                           /* fxc-error {{X3000: syntax error: unexpected token 'L'}} */
{ double left[2]; left[1] = 3000000000; }
{ double left[2]; left[1] = -3000000000; }
{ double left[2]; left[1] = 10000000000; }                                    /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ double left[2]; left[1] = 0.5; }
{ double left[2]; left[1] = -0.5F; }
{ min16float left[2]; left[1] = 0; }
{ min16float left[2]; left[1] = -1; }
{ min16float left[2]; left[1] = 2U; }                                         /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left[2]; left[1] = 3L; }
{ min16float left[2]; left[1] = 4ULL; }                                       /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left[2]; left[1] = 3000000000; }                                 /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left[2]; left[1] = -3000000000; }                                /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16float left[2]; left[1] = 10000000000; }                                /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16float left[2]; left[1] = 0.5; }
{ min16float left[2]; left[1] = -0.5F; }                                      /* expected-warning {{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left[2]; left[1] = 0; }
{ min10float left[2]; left[1] = -1; }
{ min10float left[2]; left[1] = 2U; }                                         /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left[2]; left[1] = 3L; }
{ min10float left[2]; left[1] = 4ULL; }                                       /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left[2]; left[1] = 3000000000; }                                 /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left[2]; left[1] = -3000000000; }                                /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min10float left[2]; left[1] = 10000000000; }                                /* fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min10float left[2]; left[1] = 0.5; }
{ min10float left[2]; left[1] = -0.5F; }                                      /* expected-warning {{conversion from larger type 'float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left[2]; left[1] = 0; }
{ min16int left[2]; left[1] = -1; }
{ min16int left[2]; left[1] = 2U; }                                           /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left[2]; left[1] = 3L; }
{ min16int left[2]; left[1] = 4ULL; }                                         /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left[2]; left[1] = 3000000000; }                                   /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left[2]; left[1] = -3000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from -3000000000 to -24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16int left[2]; left[1] = 10000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'min16int' changes value from 10000000000 to -7168}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16int left[2]; left[1] = 0.5; }                                          /* expected-warning {{implicit conversion from 'literal float' to 'min16int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min16int left[2]; left[1] = -0.5F; }                                        /* expected-warning {{conversion from larger type 'float' to smaller type 'min16int', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min16int' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left[2]; left[1] = 0; }
{ min12int left[2]; left[1] = -1; }
{ min12int left[2]; left[1] = 2U; }                                           /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left[2]; left[1] = 3L; }
{ min12int left[2]; left[1] = 4ULL; }                                         /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left[2]; left[1] = 3000000000; }                                   /* expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left[2]; left[1] = -3000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from -3000000000 to -24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min12int left[2]; left[1] = 10000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'min12int' changes value from 10000000000 to -7168}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min12int left[2]; left[1] = 0.5; }                                          /* expected-warning {{implicit conversion from 'literal float' to 'min12int' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min12int left[2]; left[1] = -0.5F; }                                        /* expected-warning {{conversion from larger type 'float' to smaller type 'min12int', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min12int' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left[2]; left[1] = 0; }
{ min16uint left[2]; left[1] = -1; }
{ min16uint left[2]; left[1] = 2U; }                                          /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left[2]; left[1] = 3L; }
{ min16uint left[2]; left[1] = 4ULL; }                                        /* fxc-error {{X3000: syntax error: unexpected token 'L'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left[2]; left[1] = 3000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from 3000000000 to 24064}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left[2]; left[1] = -3000000000; }                                 /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from -3000000000 to 41472}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ min16uint left[2]; left[1] = 10000000000; }                                 /* expected-warning {{implicit conversion from 'literal int' to 'min16uint' changes value from 10000000000 to 58368}} fxc-error {{X3000: syntax error: unexpected token ';'}} */
{ min16uint left[2]; left[1] = 0.5; }                                         /* expected-warning {{implicit conversion from 'literal float' to 'min16uint' changes value from 0.5 to 0}} fxc-pass {{}} */
{ min16uint left[2]; left[1] = -0.5F; }                                       /* expected-warning {{conversion from larger type 'float' to smaller type 'min16uint', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'min16uint' changes value from 0.5 to 0}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
{ int64_t left[2]; left[1] = 0; }                                             /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = -1; }                                            /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 2U; }                                            /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 3L; }                                            /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 4ULL; }                                          /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 3000000000; }                                    /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = -3000000000; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 10000000000; }                                   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = 0.5; }                                           /* expected-warning {{implicit conversion from 'literal float' to 'int64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int64_t left[2]; left[1] = -0.5F; }                                         /* expected-warning {{implicit conversion from 'float' to 'int64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint64_t left[2]; left[1] = 0; }                                            /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = -1; }                                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 2U; }                                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 3L; }                                           /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 4ULL; }                                         /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 3000000000; }                                   /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = -3000000000; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 10000000000; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = 0.5; }                                          /* expected-warning {{implicit conversion from 'literal float' to 'uint64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ uint64_t left[2]; left[1] = -0.5F; }                                        /* expected-warning {{implicit conversion from 'float' to 'uint64_t' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
{ int8_t4_packed left[2]; left[1] = 0; }                                      /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = -1; }                                     /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 2U; }                                     /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 3L; }                                     /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 4ULL; }                                   /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 3000000000; }                             /* fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = -3000000000; }                            /* expected-warning {{implicit conversion from 'literal int' to 'int8_t4_packed' changes value from -3000000000 to 1294967296}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 10000000000; }                            /* expected-warning {{implicit conversion from 'literal int' to 'int8_t4_packed' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = 0.5; }                                    /* expected-warning {{implicit conversion from 'literal float' to 'int8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ int8_t4_packed left[2]; left[1] = -0.5F; }                                  /* expected-warning {{implicit conversion from 'float' to 'int8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'int8_t4_packed'}} fxc-error {{X3000: unrecognized identifier 'left'}} */
{ uint8_t4_packed left[2]; left[1] = 0; }                                     /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = -1; }                                    /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 2U; }                                    /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 3L; }                                    /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 4ULL; }                                  /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 3000000000; }                            /* fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = -3000000000; }                           /* expected-warning {{implicit conversion from 'literal int' to 'uint8_t4_packed' changes value from -3000000000 to 1294967296}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 10000000000; }                           /* expected-warning {{implicit conversion from 'literal int' to 'uint8_t4_packed' changes value from 10000000000 to 1410065408}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = 0.5; }                                   /* expected-warning {{implicit conversion from 'literal float' to 'uint8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
{ uint8_t4_packed left[2]; left[1] = -0.5F; }                                 /* expected-warning {{implicit conversion from 'float' to 'uint8_t4_packed' changes value from 0.5 to 0}} fxc-error {{X3000: unrecognized identifier 'left'}} fxc-error {{X3000: unrecognized identifier 'uint8_t4_packed'}} */
// GENERATED_CODE:END

}