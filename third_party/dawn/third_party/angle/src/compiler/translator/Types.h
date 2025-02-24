//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TYPES_H_
#define COMPILER_TRANSLATOR_TYPES_H_

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/span.h"

#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/SymbolUniqueId.h"

namespace sh
{

struct TPublicType;
class TType;
class TInterfaceBlock;
class TStructure;
class TSymbol;
class TVariable;
class TIntermSymbol;
class TSymbolTable;

class TField : angle::NonCopyable
{
  public:
    POOL_ALLOCATOR_NEW_DELETE
    TField(TType *type, const ImmutableString &name, const TSourceLoc &line, SymbolType symbolType)
        : mType(type), mName(name), mLine(line), mSymbolType(symbolType)
    {
        ASSERT(mSymbolType != SymbolType::Empty);
    }

    // TODO(alokp): We should only return const type.
    // Fix it by tweaking grammar.
    TType *type() { return mType; }
    const TType *type() const { return mType; }
    const ImmutableString &name() const { return mName; }
    const TSourceLoc &line() const { return mLine; }
    SymbolType symbolType() const { return mSymbolType; }

  private:
    TType *mType;
    const ImmutableString mName;
    const TSourceLoc mLine;
    const SymbolType mSymbolType;
};

typedef TVector<TField *> TFieldList;

class TFieldListCollection : angle::NonCopyable
{
  public:
    const TFieldList &fields() const { return *mFields; }

    bool containsArrays() const;
    bool containsMatrices() const;
    bool containsType(TBasicType t) const;
    bool containsSamplers() const;

    size_t objectSize() const;
    // How many locations the field list consumes as a uniform.
    int getLocationCount() const;
    int deepestNesting() const;
    const TString &mangledFieldList() const;

  protected:
    TFieldListCollection(const TFieldList *fields);

    const TFieldList *mFields;

  private:
    size_t calculateObjectSize() const;
    int calculateDeepestNesting() const;
    TString buildMangledFieldList() const;

    mutable size_t mObjectSize;
    mutable int mDeepestNesting;
    mutable TString mMangledFieldList;
};

//
// Base class for things that have a type.
//
class TType
{
  public:
    POOL_ALLOCATOR_NEW_DELETE
    TType();
    explicit TType(TBasicType t, uint8_t ps = 1, uint8_t ss = 1);
    TType(TBasicType t, TPrecision p, TQualifier q = EvqTemporary, uint8_t ps = 1, uint8_t ss = 1);
    explicit TType(const TPublicType &p);
    TType(const TStructure *userDef, bool isStructSpecifier);
    TType(const TInterfaceBlock *interfaceBlockIn,
          TQualifier qualifierIn,
          TLayoutQualifier layoutQualifierIn);
    TType(const TType &t);
    TType &operator=(const TType &t);

    constexpr TType(TBasicType t,
                    TPrecision p,
                    TQualifier q,
                    uint8_t ps,
                    uint8_t ss,
                    const angle::Span<const unsigned int> arraySizes,
                    const char *mangledName)
        : type(t),
          precision(p),
          qualifier(q),
          invariant(false),
          precise(false),
          interpolant(false),
          memoryQualifier(TMemoryQualifier::Create()),
          layoutQualifier(TLayoutQualifier::Create()),
          primarySize(ps),
          secondarySize(ss),
          mArraySizes(arraySizes),
          mArraySizesStorage(nullptr),
          mInterfaceBlock(nullptr),
          mStructure(nullptr),
          mIsStructSpecifier(false),
          mInterfaceBlockFieldIndex(0),
          mMangledName(mangledName)
    {}

    constexpr TType(TType &&t)
        : type(t.type),
          precision(t.precision),
          qualifier(t.qualifier),
          invariant(t.invariant),
          precise(t.precise),
          interpolant(t.interpolant),
          memoryQualifier(t.memoryQualifier),
          layoutQualifier(t.layoutQualifier),
          primarySize(t.primarySize),
          secondarySize(t.secondarySize),
          mArraySizes(t.mArraySizes),
          mArraySizesStorage(t.mArraySizesStorage),
          mInterfaceBlock(t.mInterfaceBlock),
          mStructure(t.mStructure),
          mIsStructSpecifier(t.mIsStructSpecifier),
          mInterfaceBlockFieldIndex(0),
          mMangledName(t.mMangledName)
    {
        t.mArraySizesStorage = nullptr;
    }

    constexpr TBasicType getBasicType() const { return type; }
    void setBasicType(TBasicType t);

    TPrecision getPrecision() const { return precision; }
    void setPrecision(TPrecision p) { precision = p; }

    constexpr TQualifier getQualifier() const { return qualifier; }
    void setQualifier(TQualifier q) { qualifier = q; }

    bool isInvariant() const { return invariant; }
    void setInvariant(bool i) { invariant = i; }

    bool isPrecise() const { return precise; }
    void setPrecise(bool i) { precise = i; }

    bool isInterpolant() const { return interpolant; }
    void setInterpolant(bool i) { interpolant = i; }

    TMemoryQualifier getMemoryQualifier() const { return memoryQualifier; }
    void setMemoryQualifier(const TMemoryQualifier &mq) { memoryQualifier = mq; }

    TLayoutQualifier getLayoutQualifier() const { return layoutQualifier; }
    void setLayoutQualifier(TLayoutQualifier lq) { layoutQualifier = lq; }

    uint8_t getNominalSize() const { return primarySize; }
    uint8_t getSecondarySize() const { return secondarySize; }
    uint8_t getCols() const
    {
        ASSERT(isMatrix());
        return primarySize;
    }
    uint8_t getRows() const
    {
        ASSERT(isMatrix());
        return secondarySize;
    }
    void setPrimarySize(uint8_t ps);
    void setSecondarySize(uint8_t ss);

    // Full size of single instance of type
    size_t getObjectSize() const;

    // Get how many locations this type consumes as a uniform.
    int getLocationCount() const;

    bool isMatrix() const { return primarySize > 1 && secondarySize > 1; }
    bool isNonSquareMatrix() const { return isMatrix() && primarySize != secondarySize; }
    bool isArray() const { return !mArraySizes.empty(); }
    bool isArrayOfArrays() const { return mArraySizes.size() > 1u; }
    size_t getNumArraySizes() const { return mArraySizes.size(); }
    const angle::Span<const unsigned int> &getArraySizes() const { return mArraySizes; }
    unsigned int getArraySizeProduct() const;
    bool isUnsizedArray() const;
    unsigned int getOutermostArraySize() const
    {
        ASSERT(isArray());
        return mArraySizes.back();
    }
    void makeArray(unsigned int s);

    // sizes contain new outermost array sizes.
    void makeArrays(const angle::Span<const unsigned int> &sizes);
    // Here, the array dimension value 0 corresponds to the innermost array.
    void setArraySize(size_t arrayDimension, unsigned int s);

    // Will set unsized array sizes according to newArraySizes. In case there are more
    // unsized arrays than there are sizes in newArraySizes, defaults to setting any
    // remaining array sizes to 1.
    void sizeUnsizedArrays(const angle::Span<const unsigned int> &newArraySizes);

    // Will size the outermost array according to arraySize.
    void sizeOutermostUnsizedArray(unsigned int arraySize);

    // Note that the array element type might still be an array type in GLSL ES version >= 3.10.
    void toArrayElementType();
    // Removes all array sizes.
    void toArrayBaseType();
    // Turns a matrix into a column of it.
    void toMatrixColumnType();
    // Turns a matrix or vector into a component of it.
    void toComponentType();

    const TInterfaceBlock *getInterfaceBlock() const { return mInterfaceBlock; }
    void setInterfaceBlock(const TInterfaceBlock *interfaceBlockIn);
    bool isInterfaceBlock() const { return type == EbtInterfaceBlock; }

    void setInterfaceBlockField(const TInterfaceBlock *interfaceBlockIn, size_t fieldIndex);
    size_t getInterfaceBlockFieldIndex() const { return mInterfaceBlockFieldIndex; }

    bool isVector() const { return primarySize > 1 && secondarySize == 1; }
    bool isVectorArray() const { return primarySize > 1 && secondarySize == 1 && isArray(); }
    bool isRank0() const { return primarySize == 1 && secondarySize == 1; }
    bool isScalar() const
    {
        return primarySize == 1 && secondarySize == 1 && !mStructure && !isArray();
    }
    bool isScalarArray() const
    {
        return primarySize == 1 && secondarySize == 1 && !mStructure && isArray();
    }
    bool isScalarFloat() const { return isScalar() && type == EbtFloat; }
    bool isScalarInt() const { return isScalar() && (type == EbtInt || type == EbtUInt); }

    bool canBeConstructed() const;

    const TStructure *getStruct() const { return mStructure; }

    static constexpr char GetSizeMangledName(uint8_t primarySize, uint8_t secondarySize)
    {
        unsigned int sizeKey = (secondarySize - 1u) * 4u + primarySize - 1u;
        if (sizeKey < 10u)
        {
            return static_cast<char>('0' + sizeKey);
        }
        return static_cast<char>('A' + sizeKey - 10);
    }
    const char *getMangledName() const;

    bool sameNonArrayType(const TType &right) const;

    // Returns true if arrayType is an array made of this type.
    bool isElementTypeOf(const TType &arrayType) const;

    bool operator==(const TType &right) const
    {
        size_t numArraySizesL = getNumArraySizes();
        size_t numArraySizesR = right.getNumArraySizes();
        bool arraySizesEqual  = numArraySizesL == numArraySizesR &&
                               (numArraySizesL == 0 || mArraySizes == right.mArraySizes);
        return type == right.type && primarySize == right.primarySize &&
               secondarySize == right.secondarySize && arraySizesEqual &&
               mStructure == right.mStructure;
        // don't check the qualifier, it's not ever what's being sought after
    }
    bool operator!=(const TType &right) const { return !operator==(right); }
    bool operator<(const TType &right) const
    {
        if (type != right.type)
            return type < right.type;
        if (primarySize != right.primarySize)
            return primarySize < right.primarySize;
        if (secondarySize != right.secondarySize)
            return secondarySize < right.secondarySize;
        size_t numArraySizesL = getNumArraySizes();
        size_t numArraySizesR = right.getNumArraySizes();
        if (numArraySizesL != numArraySizesR)
            return numArraySizesL < numArraySizesR;
        for (size_t i = 0; i < numArraySizesL; ++i)
        {
            if (mArraySizes[i] != right.mArraySizes[i])
                return mArraySizes[i] < right.mArraySizes[i];
        }
        if (mStructure != right.mStructure)
            return mStructure < right.mStructure;

        return false;
    }

    const char *getBasicString() const { return sh::getBasicString(type); }

    const char *getPrecisionString() const { return sh::getPrecisionString(precision); }
    const char *getQualifierString() const { return sh::getQualifierString(qualifier); }

    const char *getBuiltInTypeNameString() const;

    // If this type is a struct, returns the deepest struct nesting of
    // any field in the struct. For example:
    //   struct nesting1 {
    //     vec4 position;
    //   };
    //   struct nesting2 {
    //     nesting1 field1;
    //     vec4 field2;
    //   };
    // For type "nesting2", this method would return 2 -- the number
    // of structures through which indirection must occur to reach the
    // deepest field (nesting2.field1.position).
    int getDeepestStructNesting() const;

    bool isNamelessStruct() const;

    bool isStructureContainingArrays() const;
    bool isStructureContainingMatrices() const;
    bool isStructureContainingType(TBasicType t) const;
    bool isStructureContainingSamplers() const;
    bool isInterfaceBlockContainingType(TBasicType t) const;

    bool isStructSpecifier() const { return mIsStructSpecifier; }

    // Return true if variables of this type should be replaced with an inline constant value if
    // such is available. False will be returned in cases where output doesn't support
    // TIntermConstantUnion nodes of the type, or if the type contains a lot of fields and creating
    // several copies of it in the output code is undesirable for performance.
    bool canReplaceWithConstantUnion() const;

    // The char arrays passed in must be pool allocated or static.
    void createSamplerSymbols(const ImmutableString &namePrefix,
                              const TString &apiNamePrefix,
                              TVector<const TVariable *> *outputSymbols,
                              TMap<const TVariable *, TString> *outputSymbolsToAPINames,
                              TSymbolTable *symbolTable) const;

    // Initializes all lazily-initialized members.
    void realize();

    bool isSampler() const { return IsSampler(type); }
    bool isSamplerCube() const { return type == EbtSamplerCube; }
    bool isAtomicCounter() const { return IsAtomicCounter(type); }
    bool isSamplerVideoWEBGL() const { return type == EbtSamplerVideoWEBGL; }
    bool isImage() const { return IsImage(type); }
    bool isPixelLocal() const { return IsPixelLocal(type); }

  private:
    constexpr void invalidateMangledName() { mMangledName = nullptr; }
    const char *buildMangledName() const;
    constexpr void onArrayDimensionsChange(const angle::Span<const unsigned int> &sizes)
    {
        mArraySizes = sizes;
        invalidateMangledName();
    }

    TBasicType type;
    TPrecision precision;
    TQualifier qualifier;
    bool invariant;
    bool precise;
    bool interpolant;

    TMemoryQualifier memoryQualifier;
    TLayoutQualifier layoutQualifier;
    uint8_t primarySize;    // size of vector or cols matrix
    uint8_t secondarySize;  // rows of a matrix

    // Used to make an array type. Outermost array size is stored at the end of the vector. Having 0
    // in this vector means an unsized array.
    angle::Span<const unsigned int> mArraySizes;
    // Storage for mArraySizes, if any.  This is usually the case, except for constexpr TTypes which
    // only have a valid mArraySizes (with mArraySizesStorage being nullptr).  Therefore, all
    // modifications to array sizes happen on the storage (and if dimensions change, mArraySizes is
    // also updated) and all reads are from mArraySizes.
    TVector<unsigned int> *mArraySizesStorage;

    // This is set only in the following two cases:
    // 1) Represents an interface block.
    // 2) Represents the member variable of an unnamed interface block.
    // It's nullptr also for members of named interface blocks.
    const TInterfaceBlock *mInterfaceBlock;

    // nullptr unless this is a struct
    const TStructure *mStructure;
    bool mIsStructSpecifier;

    // If this is a field of a nameless interface block, this would indicate which member it's
    // refering to.
    size_t mInterfaceBlockFieldIndex;

    mutable const char *mMangledName;
};

// TTypeSpecifierNonArray stores all of the necessary fields for type_specifier_nonarray from the
// grammar
struct TTypeSpecifierNonArray
{
    TBasicType type;
    uint8_t primarySize;    // size of vector or cols of matrix
    uint8_t secondarySize;  // rows of matrix
    const TStructure *userDef;
    TSourceLoc line;

    // true if the type was defined by a struct specifier rather than a reference to a type name.
    bool isStructSpecifier;

    void initialize(TBasicType aType, const TSourceLoc &aLine)
    {
        ASSERT(aType != EbtStruct);
        type              = aType;
        primarySize       = 1;
        secondarySize     = 1;
        userDef           = nullptr;
        line              = aLine;
        isStructSpecifier = false;
    }

    void initializeStruct(const TStructure *aUserDef,
                          bool aIsStructSpecifier,
                          const TSourceLoc &aLine)
    {
        type              = EbtStruct;
        primarySize       = 1;
        secondarySize     = 1;
        userDef           = aUserDef;
        line              = aLine;
        isStructSpecifier = aIsStructSpecifier;
    }

    void setAggregate(uint8_t size) { primarySize = size; }

    void setMatrix(uint8_t columns, uint8_t rows)
    {
        ASSERT(columns > 1 && rows > 1 && columns <= 4 && rows <= 4);
        primarySize   = columns;
        secondarySize = rows;
    }

    bool isMatrix() const { return primarySize > 1 && secondarySize > 1; }

    bool isVector() const { return primarySize > 1 && secondarySize == 1; }
};

// Type represeting parsed type specifire on a struct or variable declaration or
// parameter declaration.
// Note: must be trivially constructible.
struct TPublicType
{
    // Must have a trivial default constructor since it is used in YYSTYPE.
    TPublicType() = default;

    void initialize(const TTypeSpecifierNonArray &typeSpecifier, TQualifier q);
    void initializeBasicType(TBasicType basicType);
    const char *getBasicString() const { return sh::getBasicString(getBasicType()); }

    TBasicType getBasicType() const { return typeSpecifierNonArray.type; }
    void setBasicType(TBasicType basicType) { typeSpecifierNonArray.type = basicType; }
    void setQualifier(TQualifier value) { qualifier = value; }
    void setPrecision(TPrecision value) { precision = value; }
    void setMemoryQualifier(const TMemoryQualifier &value) { memoryQualifier = value; }
    void setPrecise(bool value) { precise = value; }

    uint8_t getPrimarySize() const { return typeSpecifierNonArray.primarySize; }
    uint8_t getSecondarySize() const { return typeSpecifierNonArray.secondarySize; }

    const TStructure *getUserDef() const { return typeSpecifierNonArray.userDef; }
    const TSourceLoc &getLine() const { return typeSpecifierNonArray.line; }

    bool isStructSpecifier() const { return typeSpecifierNonArray.isStructSpecifier; }

    bool isStructureContainingArrays() const;
    bool isStructureContainingType(TBasicType t) const;
    void setArraySizes(TVector<unsigned int> *sizes);
    bool isArray() const;
    void clearArrayness();
    bool isAggregate() const;
    bool isUnsizedArray() const;
    void sizeUnsizedArrays();
    void makeArrays(TVector<unsigned int> *sizes);

    TTypeSpecifierNonArray typeSpecifierNonArray;
    TLayoutQualifier layoutQualifier;
    TMemoryQualifier memoryQualifier;
    TQualifier qualifier;
    bool invariant;
    bool precise;
    TPrecision precision;

    // Either nullptr or empty in case the type is not an array. The last element is the outermost
    // array size. Note that due to bison restrictions, copies of the public type created by the
    // copy constructor share the same arraySizes pointer.
    const TVector<unsigned int> *arraySizes;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TYPES_H_
