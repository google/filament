// RUN: %dxc -Tlib_6_3 -HV 2017   -verify %s
// RUN: %dxc -Tps_6_0 -HV 2017   -verify %s

enum MyEnum {
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    TEN = 10,
};

enum class MyEnumClass {
  ZEROC,
  ONEC,
  TWOC,
  THREEC,
  FOURC = 4,
};

enum MyEnumBool : bool {
  ZEROB = true,
};

enum MyEnumInt : int {
  ZEROI,
  ONEI,
  TWOI,
  THREEI,
  FOURI,
  NEGONEI = -1,
};

enum MyEnumUInt : uint {
  ZEROU,
  ONEU,
  TWOU,
  THREEU,
  FOURU,
};

enum MyEnumDWord : dword {
  ZERODWORD,
};

enum MyEnum64 : uint64_t {
  ZERO64,
  ONE64,
  TWO64,
  THREE64,
  FOUR64,
};

enum MyEnumMin16int : min16int {
  ZEROMIN16INT,
};

enum MyEnumMin16uint : min16uint {
  ZEROMIN16UINT,
};

enum MyEnumHalf : half {                                    /* expected-error {{non-integral type 'half' is an invalid underlying type}} */
  ZEROH,
};

enum MyEnumFloat : float {                                  /* expected-error {{non-integral type 'float' is an invalid underlying type}} */
  ZEROF,
};

enum MyEnumDouble : double {                                /* expected-error {{non-integral type 'double' is an invalid underlying type}} */
  ZEROD,
};

enum MyEnumMin16Float : min16float {                        /* expected-error {{non-integral type 'min16float' is an invalid underlying type}} */
  ZEROMIN16F,
};

enum MyEnumMin10Float : min10float {                        /* expected-error {{non-integral type 'min10float' is an invalid underlying type}} */
  ZEROMIN10F,
};

int getValueFromMyEnum(MyEnum v) {                          /* expected-note {{candidate function not viable: no known conversion from 'MyEnum64' to 'MyEnum' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'MyEnumClass' to 'MyEnum' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'MyEnumInt' to 'MyEnum' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'MyEnumUInt' to 'MyEnum' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'literal int' to 'MyEnum' for 1st argument}} */
  switch (v) {
    case MyEnum::ZERO:
      return 0;
    case MyEnum::ONE:
      return 1;
    case MyEnum::TWO:
      return 2;
    case MyEnum::THREE:
      return 3;
    default:
      return -1;
  }
}

int getValueFromMyEnumClass(MyEnumClass v) {
  switch (v) {
    case MyEnumClass::ZEROC:
      return 0;
    case MyEnumClass::ONEC:
      return 1;
    case MyEnumClass::TWOC:
      return 2;
    case MyEnumClass::THREEC:
      return 3;
    default:
      return -1;
  }
}

int getValueFromInt(int i) {                                /* expected-note {{candidate function not viable: no known conversion from 'MyEnumClass' to 'int' for 1st argument}} */
  switch (i) {
    case 0:
      return 0;
    case 1:
      return 1;
    case 2:
      return 2;
    default:
      return -1;
  }
}

[shader("pixel")]
int4 main() : SV_Target {
    int v0 = getValueFromInt(ZERO);
    int v1 = getValueFromInt(MyEnumClass::ONEC); /* expected-error {{no matching function for call to 'getValueFromInt'}} */
    int v2 = getValueFromInt(TWOI);
    int v3 = getValueFromInt(THREEU);
    int v4 = getValueFromInt(FOUR64);

    int n0 = getValueFromMyEnum(ZERO);
    int n1 = getValueFromMyEnum(MyEnumClass::ONEC); /* expected-error {{no matching function for call to 'getValueFromMyEnum'}} */
    int n2 = getValueFromMyEnum(TWOI);              /* expected-error {{no matching function for call to 'getValueFromMyEnum'}} */
    int n3 = getValueFromMyEnum(THREEU);            /* expected-error {{no matching function for call to 'getValueFromMyEnum'}} */
    int n4 = getValueFromMyEnum(ZERO64);            /* expected-error {{no matching function for call to 'getValueFromMyEnum'}} */
    int n5 = getValueFromMyEnum(2);                 /* expected-error {{no matching function for call to 'getValueFromMyEnum'}} */

    int n6 = getValueFromMyEnumClass(MyEnumClass::ONEC);


    MyEnum cast0 = (MyEnum) MyEnum::FOUR;
    MyEnum cast1 = (MyEnum) MyEnumClass::THREEC;
    MyEnum cast2 = (MyEnum) MyEnumInt::TWOI;
    MyEnum cast3 = (MyEnum) MyEnum64::ONE64;
    MyEnum cast4 = (MyEnum) MyEnumUInt::ZEROU;

    MyEnum lst[4] = { ONE, TWO, TWO, THREE };

    MyEnum unary0 = MyEnum::ZERO;
    MyEnumClass unary1 = MyEnumClass::ZEROC;
    unary0++;                                       /* expected-error {{cannot increment expression of enum type 'MyEnum'}} */
    unary0--;                                       /* expected-error {{cannot decrement expression of enum type 'MyEnum'}} */
    ++unary0;                                       /* expected-error {{cannot increment expression of enum type 'MyEnum'}} */
    --unary0;                                       /* expected-error {{cannot decrement expression of enum type 'MyEnum'}} */
    unary1++;                                       /* expected-error {{numeric type expected}} */
    unary1--;                                       /* expected-error {{numeric type expected}} */
    ++unary1;                                       /* expected-error {{numeric type expected}} */
    --unary1;                                       /* expected-error {{numeric type expected}} */

    int unaryInt = !unary0;
    unaryInt = ~unary0;
    unaryInt = !unary1;                  /* expected-error {{numeric type expected}} */
    unaryInt = ~unary1;                  /* expected-error {{int or unsigned int type required}} */

    MyEnum castV = 1;                    /* expected-error {{cannot initialize a variable of type 'MyEnum' with an rvalue of type 'literal int'}} */
    MyEnumInt castI = 10;                /* expected-error {{cannot initialize a variable of type 'MyEnumInt' with an rvalue of type 'literal int'}} */
    MyEnumClass castC = 52;              /* expected-error {{cannot initialize a variable of type 'MyEnumClass' with an rvalue of type 'literal int'}} */
    MyEnumUInt castU = 34;               /* expected-error {{cannot initialize a variable of type 'MyEnumUInt' with an rvalue of type 'literal int'}} */
    MyEnum64 cast64 = 4037;              /* expected-error {{cannot initialize a variable of type 'MyEnum64' with an rvalue of type 'literal int'}} */

    MyEnum MyEnum = MyEnum::ZERO;
    MyEnumClass MyEnumClass = MyEnumClass::FOURC;
    int i0 = MyEnum;
    int i1 = MyEnum::ZERO;
    int i2 = MyEnumClass;                 /* expected-error {{cannot initialize a variable of type 'int' with an lvalue of type 'MyEnumClass'}} */
    int i3 = MyEnumClass::FOURC;          /* expected-error {{cannot initialize a variable of type 'int' with an rvalue of type 'MyEnumClass'}} */
    float f0 = MyEnum;
    float f1 = MyEnum::ZERO;
    float f2 = MyEnumClass;               /* expected-error {{cannot initialize a variable of type 'float' with an lvalue of type 'MyEnumClass'}} */
    float f3 = MyEnumClass::FOURC;        /* expected-error {{cannot initialize a variable of type 'float' with an rvalue of type 'MyEnumClass'}} */

    int unaryD = THREE++;                /* expected-error {{cannot increment expression of enum type 'MyEnum'}} */
    int unaryC = --MyEnumClass::FOURC;   /* expected-error {{expression is not assignable}} */
    int unaryI = ++TWOI;                 /* expected-error {{cannot increment expression of enum type 'MyEnumInt'}} */
    uint unaryU = ZEROU--;               /* expected-error {{cannot decrement expression of enum type 'MyEnumUInt'}} */
    int unary64 = ++THREE64;             /* expected-error {{cannot increment expression of enum type 'MyEnum64'}} */


    int Iadd = MyEnum::THREE - 48;
    int IaddI = MyEnumInt::ZEROI + 3;
    int IaddC = MyEnumClass::ONEC + 10; /* expected-error {{numeric type expected}} */
    int IaddU = MyEnumUInt::TWOU + 15;
    int Iadd64 = MyEnum64::THREE64 - 67;

    float Fadd = MyEnum::ONE + 1.5f;
    float FaddI = MyEnumInt::TWOI + 3.41f;
    float FaddC = MyEnumClass::THREEC - 256.0f; /* expected-error {{numeric type expected}} */
    float FaddU = MyEnumUInt::FOURU + 283.48f;
    float Fadd64 = MyEnum64::ZERO64  - 8471.0f;

    if (MyEnum == ONE)
      ;
    if (MyEnum != ONE)
      ;
    if (MyEnum > ONE)
      ;

    if (MyEnumClass == MyEnumClass::ONEC)
      ;
    if (MyEnumClass != MyEnumClass::ONEC)
      ;
    if (MyEnumClass < MyEnumClass::ONEC) /* expected-error {{numeric type expected}} */
      ;

    if (MyEnum == MyEnumClass::ONEC) ;  /* expected-error {{type mismatch}} */

    return 1;
}
