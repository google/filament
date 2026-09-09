// RUN: %dxc -Tlib_6_3 -enable-16bit-types   -verify -HV 2018 %s
// RUN: %dxc -Tvs_6_2 -enable-16bit-types   -verify -HV 2018 %s

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
min16float foobar = "foo"; // expected-error {{cannot initialize a variable of type 'min16float' with an lvalue of type 'literal string'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3017: cannot implicitly convert from 'const string' to 'min16float'}}

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
bool left7; min16float right7; left7 = right7;              /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
bool left8; min10float right8; left8 = right8;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
bool left9; min16int right9; left9 = right9;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
bool left10; min12int right10; left10 = right10;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
bool left11; min16uint right11; left11 = right11; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
int left12; bool right12; left12 = right12;
int left13; int right13; left13 = right13;
int left14; uint right14; left14 = right14;
int left15; dword right15; left15 = right15;
int left16; half right16; left16 = right16;
int left17; float right17; left17 = right17;
int left18; double right18; left18 = right18; // expected-warning {{conversion from larger type 'double' to smaller type 'int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
int left19; min16float right19; left19 = right19;           /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
int left20; min10float right20; left20 = right20;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
int left21; min16int right21; left21 = right21;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
int left22; min12int right22; left22 = right22;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
int left23; min16uint right23; left23 = right23; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
uint left24; bool right24; left24 = right24;
uint left25; int right25; left25 = right25;
uint left26; uint right26; left26 = right26;
uint left27; dword right27; left27 = right27;
uint left28; half right28; left28 = right28;
uint left29; float right29; left29 = right29;
uint left30; double right30; left30 = right30; // expected-warning {{conversion from larger type 'double' to smaller type 'uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
uint left31; min16float right31; left31 = right31;          /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
uint left32; min10float right32; left32 = right32;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
uint left33; min16int right33; left33 = right33;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
uint left34; min12int right34; left34 = right34;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
uint left35; min16uint right35; left35 = right35; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
dword left36; bool right36; left36 = right36;
dword left37; int right37; left37 = right37;
dword left38; uint right38; left38 = right38;
dword left39; dword right39; left39 = right39;
dword left40; half right40; left40 = right40;
dword left41; float right41; left41 = right41;
dword left42; double right42; left42 = right42; // expected-warning {{conversion from larger type 'double' to smaller type 'dword', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
dword left43; min16float right43; left43 = right43;         /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
dword left44; min10float right44; left44 = right44;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
dword left45; min16int right45; left45 = right45;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
dword left46; min12int right46; left46 = right46;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
dword left47; min16uint right47; left47 = right47; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
half left48; bool right48; left48 = right48;
half left49; int right49; left49 = right49;        /* expected-warning {{conversion from larger type 'int' to smaller type 'half', possible loss of data}} fxc-pass {{}} */
half left50; uint right50; left50 = right50;       /* expected-warning {{conversion from larger type 'uint' to smaller type 'half', possible loss of data}} fxc-pass {{}} */
half left51; dword right51; left51 = right51;      /* expected-warning {{conversion from larger type 'dword' to smaller type 'half', possible loss of data}} fxc-pass {{}} */
half left52; half right52; left52 = right52;
half left53; float right53; left53 = right53;      /* expected-warning {{conversion from larger type 'float' to smaller type 'half', possible loss of data}} fxc-pass {{}} */
half left54; double right54; left54 = right54; // expected-warning {{conversion from larger type 'double' to smaller type 'half', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
half left55; min16float right55; left55 = right55;          /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
half left56; min10float right56; left56 = right56;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
half left57; min16int right57; left57 = right57;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
half left58; min12int right58; left58 = right58;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
half left59; min16uint right59; left59 = right59; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
float left60; bool right60; left60 = right60;
float left61; int right61; left61 = right61;
float left62; uint right62; left62 = right62;
float left63; dword right63; left63 = right63;
float left64; half right64; left64 = right64;
float left65; float right65; left65 = right65;
float left66; double right66; left66 = right66; // expected-warning {{conversion from larger type 'double' to smaller type 'float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
float left67; min16float right67; left67 = right67;         /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
float left68; min10float right68; left68 = right68;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
float left69; min16int right69; left69 = right69;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
float left70; min12int right70; left70 = right70;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
float left71; min16uint right71; left71 = right71; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
double left72; bool right72; left72 = right72;
double left73; int right73; left73 = right73;
double left74; uint right74; left74 = right74;
double left75; dword right75; left75 = right75;
double left76; half right76; left76 = right76;
double left77; float right77; left77 = right77;
double left78; double right78; left78 = right78;
double left79; min16float right79; left79 = right79;        /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
double left80; min10float right80; left80 = right80;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
double left81; min16int right81; left81 = right81;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
double left82; min12int right82; left82 = right82;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
double left83; min16uint right83; left83 = right83; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min16float left84; bool right84; left84 = right84;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
min16float left85; int right85; left85 = right85; // expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left86; uint right86; left86 = right86; // expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left87; dword right87; left87 = right87; // expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left88; half right88; left88 = right88; // expected-warning {{'min16float' is promoted to 'half'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left89; float right89; left89 = right89; // expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left90; double right90; left90 = right90; // expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16float left91; min16float right91; left91 = right91;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
min16float left92; min10float right92; left92 = right92;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
min16float left93; min16int right93; left93 = right93;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
min16float left94; min12int right94; left94 = right94;  // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
min16float left95; min16uint right95; left95 = right95; /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min10float left96; bool right96; left96 = right96;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
min10float left97; int right97; left97 = right97; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left98; uint right98; left98 = right98; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left99; dword right99; left99 = right99; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left100; half right100; left100 = right100; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'half' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left101; float right101; left101 = right101; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left102; double right102; left102 = right102; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{conversion from larger type 'double' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left103; min16float right103; left103 = right103; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'min16float' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left104; min10float right104; left104 = right104;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
min10float left105; min16int right105; left105 = right105; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left106; min12int right106; left106 = right106; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min10float left107; min16uint right107; left107 = right107; // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min16int left108; bool right108; left108 = right108;        /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
min16int left109; int right109; left109 = right109; // expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'int' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left110; uint right110; left110 = right110; // expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left111; dword right111; left111 = right111; // expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left112; half right112; left112 = right112; // expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left113; float right113; left113 = right113; // expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'float' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left114; double right114; left114 = right114; // expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'double' to smaller type 'min16int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16int left115; min16float right115; left115 = right115;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
min16int left116; min10float right116; left116 = right116;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}}
min16int left117; min16int right117; left117 = right117;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}} */
min16int left118; min12int right118; left118 = right118;  // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-pass {{}}
min16int left119; min16uint right119; left119 = right119; /* expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min12int left120; bool right120; left120 = right120;  // expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}}
min12int left121; int right121; left121 = right121; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'int' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left122; uint right122; left122 = right122; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left123; dword right123; left123 = right123; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left124; half right124; left124 = right124; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'half' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left125; float right125; left125 = right125; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'float' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left126; double right126; left126 = right126; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'double' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left127; min16float right127; left127 = right127; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{conversion from larger type 'min16float' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left128; min10float right128; left128 = right128;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}} //
min12int left129; min16int right129; left129 = right129; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{conversion from larger type 'min16int' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min12int left130; min12int right130; left130 = right130;  // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-pass {{}} //
min12int left131; min16uint right131; left131 = right131; // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'min16uint' to smaller type 'min12int', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} //
min16uint left132; bool right132; left132 = right132;     /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min16uint left133; int right133; left133 = right133; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'int' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left134; uint right134; left134 = right134; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'uint' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left135; dword right135; left135 = right135; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'dword' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left136; half right136; left136 = right136; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left137; float right137; left137 = right137; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'float' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left138; double right138; left138 = right138; // expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{conversion from larger type 'double' to smaller type 'min16uint', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
min16uint left139; min16float right139; left139 = right139;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min16uint left140; min10float right140; left140 = right140;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}}
min16uint left141; min16int right141; left141 = right141;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */
min16uint left142; min12int right142; left142 = right142;  // expected-warning {{'min12int' is promoted to 'int16_t'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}}
min16uint left143; min16uint right143; left143 = right143; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-pass {{}} */

// new explicit size types
int16_t left144; bool right144; left144 = right144;        /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left144'}} */
int16_t left145; int right145; left145 = right145;         /* expected-warning {{conversion from larger type 'int' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left145'}} */
int16_t left146; uint right146; left146 = right146;        /* expected-warning {{conversion from larger type 'uint' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left146'}} */
int16_t left147; dword right147; left147 = right147;       /* expected-warning {{conversion from larger type 'dword' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left147'}} */
int16_t left148; half right148; left148 = right148;        /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left148'}} */
int16_t left149; float right149; left149 = right149;       /* expected-warning {{conversion from larger type 'float' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left149'}} */
int16_t left150; double right150; left150 = right150;      /* expected-warning {{conversion from larger type 'double' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left150'}} */
int16_t left151; min16float right151; left151 = right151;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left151'}} */
int16_t left152; min10float right152; left152 = right152;  /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left152'}} */
int16_t left153; min16int right153; left153 = right153;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left153'}} */
int16_t left154; min12int right154; left154 = right154;    /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left154'}} */
int16_t left155; min16uint right155; left155 = right155;   /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left155'}} */
int16_t left156; int16_t right156; left156 = right156;     /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left156'}} */
int16_t left157; int32_t right157; left157 = right157;     /* expected-warning {{conversion from larger type 'int32_t' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left157'}} */
int16_t left158; int64_t right158; left158 = right158;     /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left158'}} */
int16_t left159; float16_t right159; left159 = right159;   /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left159'}} */
int16_t left160; float32_t right160; left160 = right160;   /* expected-warning {{conversion from larger type 'float32_t' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left160'}} */
int16_t left161; float64_t right161; left161 = right161;   /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'int16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'left161'}} */

int32_t left162; bool right162; left162 = right162;        /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left162'}} */
int32_t left163; int right163; left163 = right163;         /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left163'}} */
int32_t left165; uint right165; left165 = right165;        /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left165'}} */
int32_t left167; dword right167; left167 = right167;       /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left167'}} */
int32_t left168; half right168; left168 = right168;        /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left168'}} */
int32_t left169; float right169; left169 = right169;       /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left169'}} */
int32_t left170; double right170; left170 = right170;      /* expected-warning {{conversion from larger type 'double' to smaller type 'int32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left170'}} */
int32_t left171; min16float right171; left171 = right171;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left171'}} */
int32_t left172; min10float right172; left172 = right172;  /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left172'}} */
int32_t left173; min16int right173; left173 = right173;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left173'}} */
int32_t left174; min12int right174; left174 = right174;    /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left174'}} */
int32_t left175; min16uint right175; left175 = right175;   /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left175'}} */
int32_t left176; int16_t right176; left176 = right176;     /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left176'}} */
int32_t left177; int32_t right177; left177 = right177;     /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left177'}} */
int32_t left178; int64_t right178; left178 = right178;     /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'int32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left178'}} */
int32_t left179; float16_t right179; left179 = right179;   /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left179'}} */
int32_t left180; float32_t right180; left180 = right180;   /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left180'}} */
int32_t left181; float64_t right181; left181 = right181;   /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'int32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'left181'}} */

int64_t left182; bool right182; left182 = right182;        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left182'}} */
int64_t left183; int right183; left183 = right183;         /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left183'}} */
int64_t left185; uint right185; left185 = right185;        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left185'}} */
int64_t left187; dword right187; left187 = right187;       /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left187'}} */
int64_t left188; half right188; left188 = right188;        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left188'}} */
int64_t left189; float right189; left189 = right189;       /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left189'}} */
int64_t left190; double right190; left190 = right190;      /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left190'}} */
int64_t left191; min16float right191; left191 = right191;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left191'}} */
int64_t left192; min10float right192; left192 = right192;  /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left192'}} */
int64_t left193; min16int right193; left193 = right193;    /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left193'}} */
int64_t left194; min12int right194; left194 = right194;    /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left194'}} */
int64_t left195; min16uint right195; left195 = right195;   /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left195'}} */
int64_t left196; int16_t right196; left196 = right196;     /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left196'}} */
int64_t left197; int32_t right197; left197 = right197;     /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left197'}} */
int64_t left198; int64_t right198; left198 = right198;     /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left198'}} */
int64_t left199; float16_t right199; left199 = right199;   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left199'}} */
int64_t left200; float32_t right200; left200 = right200;   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left200'}} */
int64_t left201; float64_t right201; left201 = right201;   /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'left201'}} */

float16_t left202; bool right202; left202 = right202;      /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left202'}} */
float16_t left203; int right203; left203 = right203;       /* expected-warning {{conversion from larger type 'int' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left203'}} */
float16_t left205; uint right205; left205 = right205;      /* expected-warning {{conversion from larger type 'uint' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left205'}} */
float16_t left207; dword right207; left207 = right207;     /* expected-warning {{conversion from larger type 'dword' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left207'}} */
float16_t left208; half right208; left208 = right208;      /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left208'}} */
float16_t left209; float right209; left209 = right209;     /* expected-warning {{conversion from larger type 'float' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left209'}} */
float16_t left210; double right210; left210 = right210;    /* expected-warning {{conversion from larger type 'double' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left210'}} */
float16_t left211; min16float right211; left211 = right211;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left211'}} */
float16_t left212; min10float right212; left212 = right212;    /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left212'}} */
float16_t left213; min16int right213; left213 = right213;  /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left213'}} */
float16_t left214; min12int right214; left214 = right214;  /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left214'}} */
float16_t left215; min16uint right215; left215 = right215; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left215'}} */
float16_t left216; int16_t right216; left216 = right216;   /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left216'}} */
float16_t left217; int32_t right217; left217 = right217;   /* expected-warning {{conversion from larger type 'int32_t' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left217'}} */
float16_t left218; int64_t right218; left218 = right218;   /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left218'}} */
float16_t left219; float16_t right219; left219 = right219; /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left219'}} */
float16_t left220; float32_t right220; left220 = right220; /* expected-warning {{conversion from larger type 'float32_t' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left220'}} */
float16_t left221; float64_t right221; left221 = right221; /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'float16_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'left221'}} */

float32_t left222; bool right222; left222 = right222;      /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left222'}} */
float32_t left223; int right223; left223 = right223;       /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left223'}} */
float32_t left225; uint right225; left225 = right225;      /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left225'}} */
float32_t left227; dword right227; left227 = right227;     /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left227'}} */
float32_t left228; half right228; left228 = right228;      /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left228'}} */
float32_t left229; float right229; left229 = right229;     /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left229'}} */
float32_t left230; double right230; left230 = right230;    /* expected-warning {{conversion from larger type 'double' to smaller type 'float32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left230'}} */
float32_t left231; min16float right231; left231 = right231;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left231'}} */
float32_t left232; min10float right232; left232 = right232;    /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left232'}} */
float32_t left233; min16int right233; left233 = right233;  /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left233'}} */
float32_t left234; min12int right234; left234 = right234;  /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left234'}} */
float32_t left235; min16uint right235; left235 = right235; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left235'}} */
float32_t left236; int16_t right236; left236 = right236;   /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left236'}} */
float32_t left237; int32_t right237; left237 = right237;   /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left237'}} */
float32_t left238; int64_t right238; left238 = right238;   /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'float32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left238'}} */
float32_t left239; float16_t right239; left239 = right239; /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left239'}} */
float32_t left240; float32_t right240; left240 = right240; /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left240'}} */
float32_t left241; float64_t right241; left241 = right241; /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'float32_t', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'left241'}} */

float64_t left242; bool right242; left242 = right242;      /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left242'}} */
float64_t left243; int right243; left243 = right243;       /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left243'}} */
float64_t left245; uint right245; left245 = right245;      /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left245'}} */
float64_t left247; dword right247; left247 = right247;     /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left247'}} */
float64_t left248; half right248; left248 = right248;      /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left248'}} */
float64_t left249; float right249; left249 = right249;     /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left249'}} */
float64_t left250; double right250; left250 = right250;    /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left250'}} */
float64_t left251; min16float right251; left251 = right251;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left251'}} */
float64_t left252; min10float right252; left252 = right252;    /* expected-warning {{'min10float' is promoted to 'half'}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left252'}} */
float64_t left253; min16int right253; left253 = right253;  /* expected-warning {{'min16int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left253'}} */
float64_t left254; min12int right254; left254 = right254;  /* expected-warning {{'min12int' is promoted to 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left254'}} */
float64_t left255; min16uint right255; left255 = right255; /* expected-warning {{'min16uint' is promoted to 'uint16_t'}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left255'}} */
float64_t left256; int16_t right256; left256 = right256;   /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left256'}} */
float64_t left257; int32_t right257; left257 = right257;   /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left257'}} */
float64_t left258; int64_t right258; left258 = right258;   /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left258'}} */
float64_t left259; float16_t right259; left259 = right259; /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left259'}} */
float64_t left260; float32_t right260; left260 = right260; /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left260'}} */
float64_t left261; float64_t right261; left261 = right261; /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'left261'}} */

bool left262; int16_t right262; left262 = right262;        /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'right262'}} */
bool left263; int32_t right263; left263 = right263;        /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'right263'}} */
bool left264; int64_t right264; left264 = right264;        /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right264'}} */
bool left265; float16_t right265; left265 = right265;      /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right265'}} */
bool left266; float32_t right266; left266 = right266;      /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right266'}} */
bool left267; float64_t right267; left267 = right267;      /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right267'}} */

int left268; int16_t right268; left268 = right268;         /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'right268'}} */
int left269; int32_t right269; left269 = right269;         /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'right269'}} */
int left271; int64_t right271; left271 = right271;         /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right271'}} */
int left272; float16_t right272; left272 = right272;       /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right272'}} */
int left273; float32_t right273; left273 = right273;       /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right273'}} */
int left274; float64_t right274; left274 = right274;       /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'int', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right274'}} */

uint left275; uint16_t right275; left275 = right275;       /* fxc-error {{X3000: unrecognized identifier 'right275'}} fxc-error {{X3000: unrecognized identifier 'uint16_t'}} */
uint left276; uint32_t right276; left276 = right276;       /* fxc-error {{X3000: unrecognized identifier 'right276'}} fxc-error {{X3000: unrecognized identifier 'uint32_t'}} */
uint left278; uint64_t right278; left278 = right278;       /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'right278'}} fxc-error {{X3000: unrecognized identifier 'uint64_t'}} */
uint left279; float16_t right279; left279 = right279;      /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right279'}} */
uint left280; float32_t right280; left280 = right280;      /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right280'}} */
uint left281; float64_t right281; left281 = right281;      /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'uint', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right281'}} */

dword left282; int16_t right282; left282 = right282;       /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'right282'}} */
dword left283; int32_t right283; left283 = right283;       /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'right283'}} */
dword left284; int64_t right284; left284 = right284;       /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'dword', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right284'}} */
dword left285; float16_t right285; left285 = right285;     /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right285'}} */
dword left286; float32_t right286; left286 = right286;     /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right286'}} */
dword left287; float64_t right287; left287 = right287;     /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'dword', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right287'}} */

float left288; int16_t right288; left288 = right288;       /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'right288'}} */
float left289; int32_t right289; left289 = right289;       /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'right289'}} */
float left290; int64_t right290; left290 = right290;       /* expected-warning {{conversion from larger type 'int64_t' to smaller type 'float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right290'}} */
float left291; float16_t right291; left291 = right291;     /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right291'}} */
float left292; float32_t right292; left292 = right292;     /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right292'}} */
float left293; float64_t right293; left293 = right293;     /* expected-warning {{conversion from larger type 'float64_t' to smaller type 'float', possible loss of data}} fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right293'}} */

double left294; int16_t right294; left294 = right294;      /* fxc-error {{X3000: unrecognized identifier 'int16_t'}} fxc-error {{X3000: unrecognized identifier 'right294'}} */
double left295; int32_t right295; left295 = right295;      /* fxc-error {{X3000: unrecognized identifier 'int32_t'}} fxc-error {{X3000: unrecognized identifier 'right295'}} */
double left296; int64_t right296; left296 = right296;      /* fxc-error {{X3000: unrecognized identifier 'int64_t'}} fxc-error {{X3000: unrecognized identifier 'right296'}} */
double left297; float16_t right297; left297 = right297;    /* fxc-error {{X3000: unrecognized identifier 'float16_t'}} fxc-error {{X3000: unrecognized identifier 'right297'}} */
double left298; float32_t right298; left298 = right298;    /* fxc-error {{X3000: unrecognized identifier 'float32_t'}} fxc-error {{X3000: unrecognized identifier 'right298'}} */
double left299; float64_t right299; left299 = right299;    /* fxc-error {{X3000: unrecognized identifier 'float64_t'}} fxc-error {{X3000: unrecognized identifier 'right299'}} */

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
 float left1004; snorm min16float right1004; left1004 = right1004;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 float left1005; unorm min16float right1005; left1005 = right1005;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 float left1006; snorm min10float right1006; left1006 = right1006;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
 float left1007; unorm min10float right1007; left1007 = right1007;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm float left1008;  float right1008; left1008 = right1008;
snorm float left1009; snorm float right1009; left1009 = right1009;
snorm float left1010; unorm float right1010; left1010 = right1010;
// snorm float left1011;  double right1011; left1011 = right1011;
// snorm float left1012; snorm double right1012; left1012 = right1012;
// snorm float left1013; unorm double right1013; left1013 = right1013;
snorm float left1014;  min16float right1014; left1014 = right1014;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm float left1015; snorm min16float right1015; left1015 = right1015;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm float left1016; unorm min16float right1016; left1016 = right1016;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm float left1017;  min10float right1017; left1017 = right1017;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm float left1018; snorm min10float right1018; left1018 = right1018;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm float left1019; unorm min10float right1019; left1019 = right1019;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm float left1020;  float right1020; left1020 = right1020;
unorm float left1021; snorm float right1021; left1021 = right1021;
unorm float left1022; unorm float right1022; left1022 = right1022;
// unorm float left1023;  double right1023; left1023 = right1023;
// unorm float left1024; snorm double right1024; left1024 = right1024;
// unorm float left1025; unorm double right1025; left1025 = right1025;
unorm float left1026;  min16float right1026; left1026 = right1026;       /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm float left1027; snorm min16float right1027; left1027 = right1027;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm float left1028; unorm min16float right1028; left1028 = right1028;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm float left1029;  min10float right1029; left1029 = right1029;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm float left1030; snorm min10float right1030; left1030 = right1030;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm float left1031; unorm min10float right1031; left1031 = right1031;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
 double left1032; snorm float right1032; left1032 = right1032;
 double left1033; unorm float right1033; left1033 = right1033;
 double left1034; snorm double right1034; left1034 = right1034;
 double left1035; unorm double right1035; left1035 = right1035;
 double left1036; snorm min16float right1036; left1036 = right1036;      /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 double left1037; unorm min16float right1037; left1037 = right1037;      /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 double left1038; snorm min10float right1038; left1038 = right1038;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
 double left1039; unorm min10float right1039; left1039 = right1039;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm double left1040;  float right1040; left1040 = right1040;
snorm double left1041; snorm float right1041; left1041 = right1041;
snorm double left1042; unorm float right1042; left1042 = right1042;
snorm double left1043;  double right1043; left1043 = right1043;
snorm double left1044; snorm double right1044; left1044 = right1044;
snorm double left1045; unorm double right1045; left1045 = right1045;
snorm double left1046;  min16float right1046; left1046 = right1046;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm double left1047; snorm min16float right1047; left1047 = right1047;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm double left1048; unorm min16float right1048; left1048 = right1048;    /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm double left1049;  min10float right1049; left1049 = right1049;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm double left1050; snorm min10float right1050; left1050 = right1050;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
snorm double left1051; unorm min10float right1051; left1051 = right1051;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm double left1052;  float right1052; left1052 = right1052;
unorm double left1053; snorm float right1053; left1053 = right1053;
unorm double left1054; unorm float right1054; left1054 = right1054;
unorm double left1055;  double right1055; left1055 = right1055;
unorm double left1056; snorm double right1056; left1056 = right1056;
unorm double left1057; unorm double right1057; left1057 = right1057;
unorm double left1058;  min16float right1058; left1058 = right1058;       /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm double left1059; snorm min16float right1059; left1059 = right1059;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm double left1060; unorm min16float right1060; left1060 = right1060;  /* expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm double left1061;  min10float right1061; left1061 = right1061;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm double left1062; snorm min10float right1062; left1062 = right1062;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
unorm double left1063; unorm min10float right1063; left1063 = right1063;  // expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}}
// min16float left1064; snorm float right1064; left1064 = right1064;
// min16float left1065; unorm float right1065; left1065 = right1065;
// min16float left1066; snorm double right1066; left1066 = right1066;
// min16float left1067; unorm double right1067; left1067 = right1067;
 min16float left1068; snorm min16float right1068; left1068 = right1068;   /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 min16float left1069; unorm min16float right1069; left1069 = right1069;   /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
 min16float left1070; snorm min10float right1070; left1070 = right1070;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
 min16float left1071; unorm min10float right1071; left1071 = right1071;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
// snorm min16float left1072;  float right1072; left1072 = right1072;
// snorm min16float left1073; snorm float right1073; left1073 = right1073;
// snorm min16float left1074; unorm float right1074; left1074 = right1074;
// snorm min16float left1075;  double right1075; left1075 = right1075;
// snorm min16float left1076; snorm double right1076; left1076 = right1076;
// snorm min16float left1077; unorm double right1077; left1077 = right1077;
snorm min16float left1078;  min16float right1078; left1078 = right1078;  /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm min16float left1079; snorm min16float right1079; left1079 = right1079;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm min16float left1080; unorm min16float right1080; left1080 = right1080;    /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
snorm min16float left1081;  min10float right1081; left1081 = right1081;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
snorm min16float left1082; snorm min10float right1082; left1082 = right1082;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
snorm min16float left1083; unorm min10float right1083; left1083 = right1083;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
// unorm min16float left1084;  float right1084; left1084 = right1084;
// unorm min16float left1085; snorm float right1085; left1085 = right1085;
// unorm min16float left1086; unorm float right1086; left1086 = right1086;
// unorm min16float left1087;  double right1087; left1087 = right1087;
// unorm min16float left1088; snorm double right1088; left1088 = right1088;
// unorm min16float left1089; unorm double right1089; left1089 = right1089;
unorm min16float left1090;  min16float right1090; left1090 = right1090;       /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm min16float left1091; snorm min16float right1091; left1091 = right1091;  /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm min16float left1092; unorm min16float right1092; left1092 = right1092;  /* expected-warning {{'min16float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}} */
unorm min16float left1093;  min10float right1093; left1093 = right1093;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
unorm min16float left1094; snorm min10float right1094; left1094 = right1094;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
unorm min16float left1095; unorm min10float right1095; left1095 = right1095;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min16float' is promoted to 'half'}} fxc-pass {{}}
// min10float left1096; snorm float right1096; left1096 = right1096;
// min10float left1097; unorm float right1097; left1097 = right1097;
// min10float left1098; snorm double right1098; left1098 = right1098;
// min10float left1099; unorm double right1099; left1099 = right1099;
// min10float left1100; snorm min16float right1100; left1100 = right1100;
// min10float left1101; unorm min16float right1101; left1101 = right1101;
 min10float left1102; snorm min10float right1102; left1102 = right1102;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
 min10float left1103; unorm min10float right1103; left1103 = right1103;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
// snorm min10float left1104;  float right1104; left1104 = right1104;
// snorm min10float left1105; snorm float right1105; left1105 = right1105;
// snorm min10float left1106; unorm float right1106; left1106 = right1106;
// snorm min10float left1107;  double right1107; left1107 = right1107;
// snorm min10float left1108; snorm double right1108; left1108 = right1108;
// snorm min10float left1109; unorm double right1109; left1109 = right1109;
// snorm min10float left1110;  min16float right1110; left1110 = right1110;
// snorm min10float left1111; snorm min16float right1111; left1111 = right1111;
// snorm min10float left1112; unorm min16float right1112; left1112 = right1112;
snorm min10float left1113;  min10float right1113; left1113 = right1113;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
snorm min10float left1114; snorm min10float right1114; left1114 = right1114;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
snorm min10float left1115; unorm min10float right1115; left1115 = right1115;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
// unorm min10float left1116;  float right1116; left1116 = right1116;
// unorm min10float left1117; snorm float right1117; left1117 = right1117;
// unorm min10float left1118; unorm float right1118; left1118 = right1118;
// unorm min10float left1119;  double right1119; left1119 = right1119;
// unorm min10float left1120; snorm double right1120; left1120 = right1120;
// unorm min10float left1121; unorm double right1121; left1121 = right1121;
// unorm min10float left1122;  min16float right1122; left1122 = right1122;
// unorm min10float left1123; snorm min16float right1123; left1123 = right1123;
// unorm min10float left1124; unorm min16float right1124; left1124 = right1124;
unorm min10float left1125;  min10float right1125; left1125 = right1125;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
unorm min10float left1126; snorm min10float right1126; left1126 = right1126;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //
unorm min10float left1127; unorm min10float right1127; left1127 = right1127;  // expected-warning {{'min10float' is promoted to 'half'}} expected-warning {{'min10float' is promoted to 'half'}} fxc-pass {{}} //


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
types1 = 'uint16_t int16_t float16_t'.split()
types2 = 'uint64_t int8_t4_packed uint8_t4_packed'.split()
types = types1 + types2
new_type_combos = [(left, right) for left in types1 for right in types2]
new_type_combos += [(left, right) for left in types2 for right in types]
</py>*/

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left; {right} right; left = right; }}', new_type_combos))</py>
// GENERATED_CODE:BEGIN
{ uint16_t left; uint64_t right; left = right; }                              /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'uint16_t', possible loss of data}} */
{ uint16_t left; int8_t4_packed right; left = right; }                        /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'uint16_t', possible loss of data}} */
{ uint16_t left; uint8_t4_packed right; left = right; }                       /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'uint16_t', possible loss of data}} */
{ int16_t left; uint64_t right; left = right; }                               /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'int16_t', possible loss of data}} */
{ int16_t left; int8_t4_packed right; left = right; }                         /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'int16_t', possible loss of data}} */
{ int16_t left; uint8_t4_packed right; left = right; }                        /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'int16_t', possible loss of data}} */
{ float16_t left; uint64_t right; left = right; }                             /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'float16_t', possible loss of data}} */
{ float16_t left; int8_t4_packed right; left = right; }                       /* expected-warning {{conversion from larger type 'int8_t4_packed' to smaller type 'float16_t', possible loss of data}} */
{ float16_t left; uint8_t4_packed right; left = right; }                      /* expected-warning {{conversion from larger type 'uint8_t4_packed' to smaller type 'float16_t', possible loss of data}} */
{ uint64_t left; uint16_t right; left = right; }
{ uint64_t left; int16_t right; left = right; }
{ uint64_t left; float16_t right; left = right; }
{ uint64_t left; uint64_t right; left = right; }
{ uint64_t left; int8_t4_packed right; left = right; }
{ uint64_t left; uint8_t4_packed right; left = right; }
{ int8_t4_packed left; uint16_t right; left = right; }
{ int8_t4_packed left; int16_t right; left = right; }
{ int8_t4_packed left; float16_t right; left = right; }
{ int8_t4_packed left; uint64_t right; left = right; }                        /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'int8_t4_packed', possible loss of data}} */
{ int8_t4_packed left; int8_t4_packed right; left = right; }
{ int8_t4_packed left; uint8_t4_packed right; left = right; }
{ uint8_t4_packed left; uint16_t right; left = right; }
{ uint8_t4_packed left; int16_t right; left = right; }
{ uint8_t4_packed left; float16_t right; left = right; }
{ uint8_t4_packed left; uint64_t right; left = right; }                       /* expected-warning {{conversion from larger type 'uint64_t' to smaller type 'uint8_t4_packed', possible loss of data}} */
{ uint8_t4_packed left; int8_t4_packed right; left = right; }
{ uint8_t4_packed left; uint8_t4_packed right; left = right; }
// GENERATED_CODE:END

// Constant assignments

/*<py>
constant_ints = '0 -1 2U 3L 4ULL 3000000000 -3000000000 10000000000'.split()
constant_floats = '0.5 -0.5F'.split()
constants = constant_ints + constant_floats
# types2 assignments already tested in scalar-assignments.hlsl
constant_assignment_combos = [(left, right) for left in types1 for right in constants]
</py>*/

// Catch bugs with SemaHLSL GetUnsignedLimit/GetSignedLimit

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left = {right}; }}', constant_assignment_combos))</py>
// GENERATED_CODE:BEGIN
{ uint16_t left = 0; }
{ uint16_t left = -1; }
{ uint16_t left = 2U; }
{ uint16_t left = 3L; }
{ uint16_t left = 4ULL; }
{ uint16_t left = 3000000000; }                                               /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from 3000000000 to 24064}} */
{ uint16_t left = -3000000000; }                                              /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from -3000000000 to 41472}} */
{ uint16_t left = 10000000000; }                                              /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from 10000000000 to 58368}} */
{ uint16_t left = 0.5; }                                                      /* expected-warning {{implicit conversion from 'literal float' to 'uint16_t' changes value from 0.5 to 0}} */
{ uint16_t left = -0.5F; }                                                    /* expected-warning {{conversion from larger type 'float' to smaller type 'uint16_t', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'uint16_t' changes value from 0.5 to 0}} */
{ int16_t left = 0; }
{ int16_t left = -1; }
{ int16_t left = 2U; }
{ int16_t left = 3L; }
{ int16_t left = 4ULL; }
{ int16_t left = 3000000000; }                                                /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from 3000000000 to 24064}} */
{ int16_t left = -3000000000; }                                               /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from -3000000000 to -24064}} */
{ int16_t left = 10000000000; }                                               /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from 10000000000 to -7168}} */
{ int16_t left = 0.5; }                                                       /* expected-warning {{implicit conversion from 'literal float' to 'int16_t' changes value from 0.5 to 0}} */
{ int16_t left = -0.5F; }                                                     /* expected-warning {{conversion from larger type 'float' to smaller type 'int16_t', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'int16_t' changes value from 0.5 to 0}} */
{ float16_t left = 0; }
{ float16_t left = -1; }
{ float16_t left = 2U; }
{ float16_t left = 3L; }
{ float16_t left = 4ULL; }
{ float16_t left = 3000000000; }
{ float16_t left = -3000000000; }
{ float16_t left = 10000000000; }
{ float16_t left = 0.5; }
{ float16_t left = -0.5F; }                                                   /* expected-warning {{conversion from larger type 'float' to smaller type 'float16_t', possible loss of data}} */
// GENERATED_CODE:END

// Catch bugs with clang type ranges when adding custom types

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('{{ {left} left[2]; left[1] = {right}; }}', constant_assignment_combos))</py>
// GENERATED_CODE:BEGIN
{ uint16_t left[2]; left[1] = 0; }
{ uint16_t left[2]; left[1] = -1; }
{ uint16_t left[2]; left[1] = 2U; }
{ uint16_t left[2]; left[1] = 3L; }
{ uint16_t left[2]; left[1] = 4ULL; }
{ uint16_t left[2]; left[1] = 3000000000; }                                   /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from 3000000000 to 24064}} */
{ uint16_t left[2]; left[1] = -3000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from -3000000000 to 41472}} */
{ uint16_t left[2]; left[1] = 10000000000; }                                  /* expected-warning {{implicit conversion from 'literal int' to 'uint16_t' changes value from 10000000000 to 58368}} */
{ uint16_t left[2]; left[1] = 0.5; }                                          /* expected-warning {{implicit conversion from 'literal float' to 'uint16_t' changes value from 0.5 to 0}} */
{ uint16_t left[2]; left[1] = -0.5F; }                                        /* expected-warning {{conversion from larger type 'float' to smaller type 'uint16_t', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'uint16_t' changes value from 0.5 to 0}} */
{ int16_t left[2]; left[1] = 0; }
{ int16_t left[2]; left[1] = -1; }
{ int16_t left[2]; left[1] = 2U; }
{ int16_t left[2]; left[1] = 3L; }
{ int16_t left[2]; left[1] = 4ULL; }
{ int16_t left[2]; left[1] = 3000000000; }                                    /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from 3000000000 to 24064}} */
{ int16_t left[2]; left[1] = -3000000000; }                                   /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from -3000000000 to -24064}} */
{ int16_t left[2]; left[1] = 10000000000; }                                   /* expected-warning {{implicit conversion from 'literal int' to 'int16_t' changes value from 10000000000 to -7168}} */
{ int16_t left[2]; left[1] = 0.5; }                                           /* expected-warning {{implicit conversion from 'literal float' to 'int16_t' changes value from 0.5 to 0}} */
{ int16_t left[2]; left[1] = -0.5F; }                                         /* expected-warning {{conversion from larger type 'float' to smaller type 'int16_t', possible loss of data}} expected-warning {{implicit conversion from 'float' to 'int16_t' changes value from 0.5 to 0}} */
{ float16_t left[2]; left[1] = 0; }
{ float16_t left[2]; left[1] = -1; }
{ float16_t left[2]; left[1] = 2U; }
{ float16_t left[2]; left[1] = 3L; }
{ float16_t left[2]; left[1] = 4ULL; }
{ float16_t left[2]; left[1] = 3000000000; }
{ float16_t left[2]; left[1] = -3000000000; }
{ float16_t left[2]; left[1] = 10000000000; }
{ float16_t left[2]; left[1] = 0.5; }
{ float16_t left[2]; left[1] = -0.5F; }                                       /* expected-warning {{conversion from larger type 'float' to smaller type 'float16_t', possible loss of data}} */
// GENERATED_CODE:END

}